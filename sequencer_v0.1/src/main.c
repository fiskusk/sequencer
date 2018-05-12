#include "types.h"
#include "settings.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

#include "lcd.h"
#include "uart.h"
#include "ui.h"
#include "adc.h"
#include "ptt.h"
#include "switching.h"

// global variables
char *pom; // auxiliary variable for sending message to UART or LCD

void setup(void)
{
    // setup ports
    DDRC  = (1 << 5) | (1 << 6); // set PORTC5 as output
    DDRB |= 0b01111111;          // set display pins as output (H output)
    
    ui_init();
    ptt_init();
    adc_init();
    switching_init();

    // setup LCD and UART
    lcd_init(LCD_DISP_ON); // initialization display
    lcd_clrscr();          // clear display
    uart_init();           // initialization UART
   
    sei(); // enable all interrupts
    
    switching_bias(DISABLE);
}

int main(void)
{
    wdt_enable(WDTO_2S);

    setup();
   
    // in infinite loop print info to LCD
    while (1)
    {   
        wdt_reset();
        if (print_func == ENABLE)
            ui_handle();       
    }
    return 0;
} /* main */

//volatile uint16_t sec_tmr;
//volatile uint8_t sec_flag;
//
////1ms interrupt
//ISR(Nejakej_citac)
//{
//if (sec_tmr)
//{
    //if (--sec_tmr == 0)
    //{
        //sec_tmr = 999;
        //sec_flag = 1;
    //}
//}
//else
//sec_tmr = 999;
//}

//in the while(){} do this:
//if (blink_led)
//{
//  blink_led = 0;
//  led_change_state();
//}
////if (sec_flag)
//{
//  sec_flag = 0;
//  stopky_handle();
//}
