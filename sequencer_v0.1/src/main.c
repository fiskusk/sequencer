#include "types.h"
#include "settings.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "lcd.h"
#include "uart.h"

#include "adc.h"
#include "ptt.h"
#include "switching.h"

// global variables
char *pom; // auxiliary variable for sending message to UART or LCD
volatile ui_state_t ui_state = UI_INIT;

void setup(void)
{
    // setup ports
    DDRC  = (1 << 5) | (1 << 6); // set PORTC5 as output
    
      
    ptt_init();
    adc_init();
    switching_init();

    // setup LCD and UART
    lcd_init(LCD_DISP_ON); // initialization display
    lcd_clrscr();          // clear display
    uart_init();           // initialization UART
    
    ui_state = UI_INIT;
    
    sei(); // enable all interrupts
     
}

int main(void)
{
    uint16_t cela_cast  = 0;
    uint16_t desetinna = 0;
    float des_tvar     = 0;
    char buffer[20];

    
    setup();
    
    
    


    // in infinite loop print info to LCD
    while (1)
    {   
        switch (ui_state)
        {
            case UI_INIT:
                // TODO: Show init screen
                ptt_set_irq(DISABLE);
                uart_puts("Start\n");
                lcd_gotoxy(0,1);
                lcd_puts("PA 1 kW 144 MHz");
                lcd_gotoxy(4,2);
                lcd_puts("rev. 1.0");
                switching_fan(ENABLE);
                _delay_ms(1000);
                switching_fan(DISABLE);
                _delay_ms(1500);
                ptt_set_irq(ENABLE);
            
                lcd_gotoxy(0,0);
                lcd_puts("SWR  .     ");
                lcd_gotoxy(0,1);
                lcd_puts("OUT POWER      W");
                lcd_gotoxy(0,2);
                lcd_puts("REF.POWER      W");
                lcd_gotoxy(0,3);
                lcd_puts("U=  . V  I=  . A");
            
                pom = "OK";
                ui_state = UI_RUN;
                break;
            
            case UI_RUN:
                cela_cast  = (((adc_power * ADC_REF) / 1024.0) - 0.3607) / 0.0272;
                sprintf_P(buffer, PSTR("OUT POWER %4d W"),cela_cast);
                lcd_gotoxy(0,1);
                lcd_puts(buffer);
                
                cela_cast  = (((adc_swr * ADC_REF) / 1024.0) - 0.3607) / 0.0272;
                sprintf_P(buffer, PSTR("REF.POWER %4d W"),cela_cast);
                lcd_gotoxy(0,2);
                lcd_puts(buffer);
                break;
            
            case UI_HI_SWR:
                sprintf_P(buffer, PSTR("BLOCK TX FOR %2ds"), 20 - (timer_ovf_count / 61) );
                lcd_gotoxy(0,1);
                lcd_puts(buffer);
                
                sprintf_P(buffer, PSTR("HIGH SWR %3d    "),adc_swr_cache );
                lcd_gotoxy(0,2);
                lcd_puts(buffer);
                break;
                
            case UI_HI_TEMP:
                sprintf_P(buffer, PSTR("BLOCK TX FOR %2ds"), 20 - (timer_ovf_count / 61) );
                lcd_gotoxy(0,1);
                lcd_puts(buffer);
                
                lcd_gotoxy(0,2);
                lcd_puts("HEATSINK TEMP HI");
                break;
            case UI_VOLTAGE_BEYOND_LIM:
                lcd_gotoxy(0,1);
                lcd_puts("  POWER SUPPLY  ");
                lcd_gotoxy(0,2);
                lcd_puts("VOLT. BEYOND LIM");
                break;
             case UI_CURRENT_OVERLOAD:
                lcd_gotoxy(0,1);
                lcd_puts(" SUPPLY CURRENT ");
                lcd_gotoxy(0,2);
                sprintf_P(buffer, PSTR("EXCEEDED 40A %2ds"), 20 - (timer_ovf_count / 61) );
                lcd_puts(buffer);
                break;
            
            default:
                ui_state = UI_RUN;
                break;
        }
        // print Ucc
        des_tvar  = ( (adc_ucc * ADC_REF) / 1024.0) * 28.08988764; // * 28.08988764 or  27.92008197
        cela_cast = des_tvar;
        desetinna = (des_tvar - (float) cela_cast) * 10;
        
        sprintf_P(buffer, PSTR("%2d"), cela_cast);
        lcd_gotoxy(2,3);
        lcd_puts(buffer);
        
        sprintf_P(buffer, PSTR("%d"),desetinna);
        lcd_gotoxy(5,3);
        lcd_puts(buffer);
        
        // print Icc
        des_tvar  = ((adc_icc * ADC_REF) / 1024.0) / (0.0025 * 20);
        cela_cast = des_tvar;
        desetinna = (des_tvar - (float) cela_cast) * 10;
        
        sprintf_P(buffer, PSTR("%2d"),cela_cast);
        lcd_gotoxy(11,3);
        lcd_puts(buffer);
        
        sprintf_P(buffer, PSTR("%d"),desetinna);
        lcd_gotoxy(14,3);
        lcd_puts(buffer);

        
        //print Temp on heatsink
        cela_cast = (-68.504) * ((adc_temp_heatsink * ADC_REF) / 1024.0) + 139.33;
//        des_tvar = ((adc_temp_heatsink * 2.502) / 1024.0);
        lcd_gotoxy(9,0);
        sprintf_P(buffer, PSTR("%2d%cC "), cela_cast,0xDF);
        lcd_gotoxy(9,0);
        lcd_puts(buffer);
        
        
        
        // print status
        lcd_gotoxy(14,0);
        lcd_puts(pom);
        

    }
    return 0;
} /* main */
