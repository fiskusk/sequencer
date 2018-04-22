#include "ui.h"
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include "lcd.h"

volatile ui_state_t ui_state;
volatile state_t print_func;

uint16_t cela_cast  = 0;
uint16_t desetinna = 0;
float des_tvar     = 0;
char buffer[20];

void ui_init(void)
{
    // timer 0 init
    TIMSK0 |= 1<<TOIE0;
    TCCR0B |= (1 << CS02) | (1 << CS00);    // div 1024
    ui_state = UI_INIT;
}

ISR(TIMER0_OVF_vect)
{
    print_func = ENABLE;
    static uint16_t adc_ovf_count = 0;
    static state_t state_led = ENABLE;
    if (++adc_ovf_count > 50)
    {
        adc_ovf_count = 0;
        switching_status_led(state_led);
        if (state_led == ENABLE)
            state_led = DISABLE;
        else
            state_led = ENABLE;
    }
}

void ui_handle(void)
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
            wdt_reset();
            switching_fan(DISABLE);
            _delay_ms(1500);
            wdt_reset();
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
    des_tvar  = ( (adc_ucc * ADC_REF) / 1024.0) * 28.08988764; // * 28.08988764 or  27.92008197
    cela_cast = des_tvar;
    desetinna = (des_tvar - (float) cela_cast) * 10;
    sprintf_P(buffer, PSTR("%2d"), cela_cast);
    lcd_gotoxy(2,3);
    lcd_puts(buffer);
    sprintf_P(buffer, PSTR("%d"),desetinna);
    lcd_gotoxy(5,3);
    lcd_puts(buffer);
    
    des_tvar  = ((adc_icc * ADC_REF) / 1024.0) / (0.0025 * 20);
    cela_cast = des_tvar;
    desetinna = (des_tvar - (float) cela_cast) * 10;
    sprintf_P(buffer, PSTR("%2d"),cela_cast);
    lcd_gotoxy(11,3);
    lcd_puts(buffer);
    sprintf_P(buffer, PSTR("%d"),desetinna);
    lcd_gotoxy(14,3);
    lcd_puts(buffer);
    
    //cela_cast = (-68.504) * ((adc_temp_heatsink * ADC_REF) / 1024.0) + 139.33;
    //des_tvar = ((adc_temp_heatsink * 2.502) / 1024.0);
    //lcd_gotoxy(9,0);
    sprintf_P(buffer, PSTR("%2d%cC "), adc_get_temp(),0xDF);
    lcd_gotoxy(9,0);
    lcd_puts(buffer);
    
    lcd_gotoxy(14,0);
    lcd_puts(pom);
    
    print_func = DISABLE;
}