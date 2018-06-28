#include "ui.h"
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include "lcd.h"
#include "op_button.h"

volatile ui_state_t ui_state;               
volatile state_t print_func;

uint16_t pwr, ref;                          
uint8_t delay_ovf_count = 0;
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
/*  Toggle port                                                         */
/************************************************************************/

void toggle_status_led_port(void)
{
    static state_t state_led = ENABLE;  // predefine status led to activate
    switching_status_led(state_led);// EN/DIS status LED
    // negate state of LED
    if (state_led == ENABLE)
        state_led = DISABLE;
    else
        state_led = ENABLE;
}

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
            if (delay_ovf_count == 0)
            {
                ptt_set_irq(DISABLE);           // disable PTT interruption
                operate_button_set_irq(DISABLE);// disable OP butt. intrr.
                lcd_gotoxy(0,1);
                lcd_puts("PA 1 kW 144 MHz");
                lcd_gotoxy(4,2);
                lcd_puts("rev. 1.1");
            }
            if (delay_ovf_count <= 61)
            {
                delay_ovf_count++;
                switching_fan(ENABLE);          // turn on FAN 
                break;
            }
            switching_fan(DISABLE);         // turn off FAN
            if (delay_ovf_count <= 150)
            {
                delay_ovf_count++;
                break;
            }
            delay_ovf_count = 0;
            // print to LCD static text
            lcd_gotoxy(0,0);
            lcd_puts("SWR  .     ");
            lcd_gotoxy(0,3);
            lcd_puts("U=  . V  I=  . A");
            ui_state = UI_STANDBY;              // next state is standby mode
            mode = "SB";
            switching_operate_stby_led(DISABLE);    // turn on LED, which indicates stby mode
            operate_button_set_irq(ENABLE);
            break;
        
        case UI_STANDBY:
            lcd_gotoxy(0,1);
            lcd_puts("  STANDBY MODE  ");
            lcd_gotoxy(0,2);
            delay_ovf_count++;
            if (delay_ovf_count <= 92)
            {
                lcd_puts("TO UNBLOCK PRESS");
                break;
            }
            if (delay_ovf_count <= 184)
            {
                lcd_puts("OPERATION BUTTON");
                break;
            }
            delay_ovf_count = 0;
            break;
            
        case UI_OPERATE:                        // normal operating state
            pwr = adc_get_pwr();
            sprintf_P(buffer, PSTR("OUT POWER %4d W"), pwr); 
            lcd_gotoxy(0,1);
            lcd_puts(buffer);               // print power
            ref = adc_get_reflected();
            if (ref == 2)
                ref = 0;
            sprintf_P(buffer, PSTR("REF.POWER %4d W"), ref);
            lcd_gotoxy(0,2);
            lcd_puts(buffer);               // print reflected power
            lcd_gotoxy(3,0);
            lcd_puts(adc_get_swr(pwr,ref)); // print swr 
            break;
        
        case UI_HI_REF:                     // UI state, when HI SWR
            lcd_gotoxy(0,1);
            lcd_puts("TOO HI REF POWER");
            lcd_gotoxy(0,2);
            delay_ovf_count++;
            if (delay_ovf_count <= 92)
            {
                lcd_puts("TO UNBLOCK PRESS");
                break;
            }
            if (delay_ovf_count <= 184)
            {
                lcd_puts("OPERATION BUTTON");
                break; 
            }
            delay_ovf_count = 0;
            break;
            
        case UI_HI_PWR:                     // UI state, when HI SWR
            lcd_gotoxy(0,1);
            lcd_puts("TOO HI OUT POWER");
            lcd_gotoxy(0,2);
            delay_ovf_count++;
            if (delay_ovf_count <= 92)
            {
                lcd_puts("TO UNBLOCK PRESS");
                break;
            }
            if (delay_ovf_count <= 184)
            {
                lcd_puts("OPERATION BUTTON");
                break;
            }
            delay_ovf_count = 0;
            break;
        
        case UI_HI_TEMP:                    // UI state, when HI heatsink temp
            lcd_gotoxy(0,1);
            lcd_puts("HEATSINK TEMP HI");
            lcd_gotoxy(0,2);
            delay_ovf_count++;
            if (delay_ovf_count <= 92)
            {
                lcd_puts("TO UNBLOCK PRESS");
                break;
            }
            if (delay_ovf_count <= 184)
            {
                lcd_puts("OPERATION BUTTON");
                break;
            }
            delay_ovf_count = 0;
            break;
        
        case UI_VOLTAGE_BEYOND_LIM:         // UI state, when voltage beyonfd limits
            lcd_gotoxy(0,1);
            lcd_puts("VOLT. BEYOND LIM");
            lcd_gotoxy(0,2);
            delay_ovf_count++;
            if (delay_ovf_count <= 92)
            {
                lcd_puts("TO UNBLOCK PRESS");
                break;
            }
            if (delay_ovf_count <= 184)
            {
                lcd_puts("OPERATION BUTTON");
                break;
            }
            delay_ovf_count = 0;
            break;
        
        case UI_CURRENT_OVERLOAD:           // UI state, when current overload
            lcd_gotoxy(0,1);
            lcd_puts("CURR. EXCEED 40A");
            lcd_gotoxy(0,2);
            delay_ovf_count++;
            if (delay_ovf_count <= 92)
            {
                lcd_puts("TO UNBLOCK PRESS");
                break;
            }
            if (delay_ovf_count <= 184)
            {
                lcd_puts("OPERATION BUTTON");
                break;
            }
            delay_ovf_count = 0;
            break;
        
        default:
            ui_state = UI_OPERATE;
            break;
    }
    if (ui_state != UI_INIT)
    {
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
        lcd_puts(mode);              // print status
    }
    
    
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
    static uint16_t led_ovf_count = 0;  // predefine overflow counter to zero
        
    // after 50 repeats interrupt do this (820 ms)
    if (++led_ovf_count > 50)       
    {
        led_ovf_count = 0;              // reset overflow counter
        toggle_status_led_port();
    }
}
