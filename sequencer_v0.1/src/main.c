// include used libraries
#include "settings.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

// include my libraries
#include "main.h"           // main library
#include "lcd.h"            // library for LCD display
#include "uart.h"           // library for UART to debuging
//#include "events.h"         // library functions for events
#include "adc.h"
#include "ptt.h"
#include "switching.h"

// global variables
uint8_t way = 0;                    // default way is to turning off (normally when PTT push off)
uint8_t fault_count = 2;            // presets the fault counter to the default value (only one loo�)
uint8_t fault_flag;             // 0 bez poruchy, 1 porucha, 2 prvni zapnuti-test
char *pom;                          // auxiliary variable for sending message to UART or LCD

volatile uint8_t cela_cast = 0;
volatile uint16_t desetinna = 0;
volatile float des_tvar = 0;
char buffer[9], buffer2[9], buffer3[9];
uint8_t once_fault_event;

void setup(void)
{
    // setup ports
    DDRB |= 0b01111111;    // set display pins as output (H output)
    DDRC = (1<<5) | (1<<6);        // set PORTC5 as output

    ptt_init();
    adc_init();
    switching_init();

    // setup LCD and UART
    lcd_init(LCD_DISP_ON); // initialization display
    lcd_clrscr();          // clear display
    uart_init();           // initialization UART

    sei();                 // enable all interrupts
}

void device_initialize(void);

int main(void)
{
    setup();

    //test prints
    pom = "Pok";
    uart_puts("Start , vse vypne skokem do FAULT a provede prvni test v AFTER_FAULT\n");
    
    device_initialize();
	// after startup, actual_state was set in setup() to fault
    // now jump immediately to ISR_timer1 to execute routine of fault
    TIFR1 |= 1<<TOV1;
    fault_flag = 2;             // flag set to identifing first startup device
    once_fault_event = loop_repeat(ENABLE);
    switching_timer_set_state(ENABLE);   // Timer1 GO!

    // in infinite loop print info to LCD
    while (1)
    {
        lcd_gotoxy(0,0);
        lcd_puts(pom);

        lcd_gotoxy(0,1);
        lcd_puts("    ");
        lcd_gotoxy(0,1);
        //for (uint8_t i = 0,i<=)
        itoa(adc_power,buffer3,10);
        lcd_puts(buffer3);

        des_tvar = (adc_power*1.133)/1024.0;
        cela_cast = des_tvar;
        desetinna = (des_tvar - (float)cela_cast)*1000;
        itoa(desetinna,buffer2,10);
        itoa(cela_cast,buffer,10);

        lcd_gotoxy(9,1);
        lcd_puts(buffer);
        lcd_putc(',');
        lcd_puts(buffer2);
        lcd_puts(" V  ");

        _delay_ms(200);
    };
    return 0;
}


