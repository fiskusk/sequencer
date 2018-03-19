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
#include "events.h"         // library functions for events
#include "adc.h"
#include "ptt.h"

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
uint8_t once_ptt_event;


sequencer_t old_state;                                  // backup enum types
sequencer_t machine_state = FAULT;                       // default after start up device, go to fault event


void setup(void)
{
    // setup ports
    DDRB |= 0b01111111;    // set display pins as output (H output)
    DDRC = (1<<5) | (1<<6);        // set PORTC5 as output

    // setup TIMER1
    EICRA |= 1<<ISC00;     // any logical change INT0 generate interrupt
    TIMSK1 |= 1<<TOIE1;    // enable interrupt when overflow Timer1

    ptt_init();
    adc_init();

    // setup LCD and UART
    lcd_init(LCD_DISP_ON); // initialization display
    lcd_clrscr();          // clear display
    uart_init();           // initialization UART

    sei();                 // enable all interrupts
}

int main(void)
{
    setup();

    //test prints
    pom = "Pok";
    uart_puts("Start , vse vypne skokem do FAULT a provede prvni test v AFTER_FAULT\n");

	// after startup, actual_state was set in setup() to fault
    // now jump immediately to ISR_timer1 to execute routine of fault
    TIFR1 |= 1<<TOV1;
    PORTC &= ~(1<<5);
    fault_flag = 2;             // flag set to identifing first startup device
    once_fault_event = loop_repeat(ENABLE);
    timer1_set_state(ENABLE);   // Timer1 GO!

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



ISR(INT0_vect)
{
    event_PTT_button_status_changed();
}

ISR(TIMER1_OVF_vect)
{
    switch(machine_state)
    {
        case EVENT0:
            E0_on_off_relay2();
            break;
        case EVENT1:
            E1_on_off_bias();
            break;
        case EVENT2:
            E2_on_Ucc_off_relay1();
            break;
        case FAULT:
            fault_off_all();
            break;
        case AFTER_FAULT:
            after_fault_check_status();
            break;
        case TEST_PTT:
            test_state_of_PTT_button();
            break;
        default:
            machine_state = FAULT;
            break;
    }
}
