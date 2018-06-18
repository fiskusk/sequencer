#include "ui.h"
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include "lcd.h"

volatile ui_state_t ui_state;               
volatile state_t print_func;

uint16_t pwr, ref;                          
//uint16_t desetinna = 0;
//float des_tvar     = 0;
char buffer[20];                            

/************************************************************************/
/*  Initialization function for user interface                          */
/************************************************************************/
void ui_init(void)
{
    // timer 0 init
    TIMSK0 |= 1<<TOIE0;                     // EN interrupt from TC0 when overflow
    TCCR0B |= (1 << CS02) | (1 << CS00);    // prescaller fclk/1024
    ui_state = UI_INIT;                     // default UI_state, startup message
    
} /* ui_init */

/************************************************************************/
/*  ROUNDING FUNCTION                                                   */
/*  input value is a thousand times bigger - mili                       */
/*  output is field of characters                                       */
/************************************************************************/ 
char* ui_one_decimal(uint16_t milinumber)
{
    uint8_t int_part;                       // integer part
    uint16_t dec_part;                      // decimal part
    static char buffer[20];
    
    //if ( milinumber <= 99 && milinumber >= 50 )
        //milinumber += 10;
    if ( (milinumber - (milinumber/100) * 100) >= 50)
        milinumber += 10;
    int_part = milinumber/1000;
    dec_part = (milinumber%1000)/100;
    
    // transfer the result to the correct format to print on LCD
    sprintf_P(buffer, PSTR("%2d.%1d"), int_part, dec_part); 
    return buffer;
    
} /* ui_one_decimal */

/************************************************************************/
/*  UI HANDLE with info to print on LCD                                 */
/*  sctructure with switch - case                                       */
/*  cases setup externaly from adc.c, switching.c and so on             */
/************************************************************************/
void ui_handle(void)
{
    switch (ui_state)
    {
        case UI_INIT:                       // initial first message 
            ptt_set_irq(DISABLE);           // disable PTT interruption
            lcd_gotoxy(0,1);
            lcd_puts("PA 1 kW 144 MHz");
            lcd_gotoxy(4,2);
            lcd_puts("rev. 1.0");
            switching_fan(ENABLE);          // turn on FAN 
            _delay_ms(1000);        
            wdt_reset();                    // reset watchdog
            switching_fan(DISABLE);         // turn off FAN
            _delay_ms(1500);
            wdt_reset();                    // reset watchdog again
            ptt_set_irq(ENABLE);            // enable PTT interruption
            
            // print to LCD static text
            lcd_gotoxy(0,0);
            lcd_puts("SWR  .     ");
            lcd_gotoxy(0,1);
            lcd_puts("OUT POWER      W");
            lcd_gotoxy(0,2);
            lcd_puts("REF.POWER      W");
            lcd_gotoxy(0,3);
            lcd_puts("U=  . V  I=  . A");
            pom = "OK";                     // top right corner prints status OK
            ui_state = UI_RUN;              // next state is normal operating 
            break;
        
        case UI_RUN:                        // normal operating state
            pwr = adc_get_pwr();
            sprintf_P(buffer, PSTR("OUT POWER %4d W"), pwr); 
            lcd_gotoxy(0,1);
            lcd_puts(buffer);               // print power
            ref = adc_get_reflected();
            sprintf_P(buffer, PSTR("REF.POWER %4d W"), ref);
            lcd_gotoxy(0,2);
            lcd_puts(buffer);               // print reflected power
            lcd_gotoxy(3,0);
            lcd_puts(adc_get_swr(pwr,ref)); // print swr 
            break;
        
        case UI_HI_REF:                     // UI state, when HI SWR
            sprintf_P(buffer, PSTR("BLOCK TX FOR %2ds"), 20 - (timer_ovf_count / 61) );
            lcd_gotoxy(0,1);
            lcd_puts(buffer);
            sprintf_P(buffer, PSTR("TOO HIGH REF%4d"),adc_ref_cache );
            lcd_gotoxy(0,2);
            lcd_puts(buffer);
            break;
        
        case UI_HI_TEMP:                    // UI state, when HI heatsink temp
            sprintf_P(buffer, PSTR("BLOCK TX FOR %2ds"), 20 - (timer_ovf_count / 61) );
            lcd_gotoxy(0,1);
            lcd_puts(buffer);
            lcd_gotoxy(0,2);
            lcd_puts("HEATSINK TEMP HI");
            break;
        
        case UI_VOLTAGE_BEYOND_LIM:         // UI state, when voltage beyonfd limits
            lcd_gotoxy(0,1);
            lcd_puts("  POWER SUPPLY  ");
            lcd_gotoxy(0,2);
            lcd_puts("VOLT. BEYOND LIM");
            break;
        
        case UI_CURRENT_OVERLOAD:           // UI state, when current overload
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
    // these statements are always redrawn 
    lcd_gotoxy(2,3);
    lcd_puts(ui_one_decimal(adc_get_ucc() ) );                  // print voltage
    
    lcd_gotoxy(11,3);
    lcd_puts(ui_one_decimal(adc_get_icc() ) );                  // print current
    //lcd_puts(ui_decimal(12345));
    
    sprintf_P(buffer, PSTR("%2d%cC "), adc_get_temp(),0xDF);  // 0xDF is ° {degrees}   
    lcd_gotoxy(9,0);
    lcd_puts(buffer);           // print temperature
    
    lcd_gotoxy(14,0);
    lcd_puts(pom);              // print status
    
    // stop draw to display (next print after next interuption (aprrox. 16ms)
    print_func = DISABLE;       
} /* ui_handle */

/************************************************************************/
/*  INTERRUPT routine for UI handle                                     */
/*  added status LED flashing,                                          */
/*  LED control good function MCU                                       */
/*  this routine execute each aprox 16,4 ms                             */
/************************************************************************/
ISR(TIMER0_OVF_vect)
{
    print_func = ENABLE;                // enable print to LCD in main.c while func
    static uint16_t adc_ovf_count = 0;  // predefine overflow counter to zero
    static state_t state_led = ENABLE;  // predefine status led to activate
    
    // after 50 repeats interrupt do this
    if (++adc_ovf_count > 50)       
    {
        adc_ovf_count = 0;              // reset overflow counter
        switching_status_led(state_led);// EN/DIS status LED
        // negate state of LED
        if (state_led == ENABLE)
            state_led = DISABLE;
        else
            state_led = ENABLE;
    }
}
