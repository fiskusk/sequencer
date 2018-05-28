#include "types.h"
#include "settings.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>            // include watchdog library

#include "lcd.h"
#include "ui.h"
#include "adc.h"
#include "ptt.h"
#include "switching.h"

// global variables
char *pom;      // auxiliary variable for sending message to UART or LCD

/************************************************************************/
/*  Initial setup function                                              */
/************************************************************************/
void setup(void)
{
    // setup ports
    DDRC  = (1 << 5) | (1 << 6);    // set PORTC5,6 as output
    DDRB |= 0b01111111;             // setup display pins as output (H output)
    
    ui_init();                      // setup TC0 and set default startup message
    ptt_init();                     // set port for PTT button
    adc_init();                     // setup ADC, TC2 and ADC ports (PortC)
    switching_init();               // setup ports as output and predefine states, EN TC0

    // setup LCD and UART
    lcd_init(LCD_DISP_ON);          // initialization display
    lcd_clrscr();                   // clear display
   
    sei();                          // enable all interrupts
    
    switching_bias(DISABLE);        // block bias
} /* setup */

/************************************************************************/
/*  Main function                                                       */
/************************************************************************/
int main(void)
{
    wdt_enable(WDTO_2S);            // watchdog enable for 2 second

    setup();                        // run initial initialization
   
    // in infinite loop print info to LCD
    while (1)
    {   
        wdt_reset();                // reset wdt state
        // each interrupt set print_func to enable and do this (each approx 16ms)
        if (print_func == ENABLE)   // when UI TC0 set enable print to LCD
            ui_handle();            // printing function
    }
    return 0;
} /* main */
