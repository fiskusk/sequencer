#include "ui.h"

void ui_init(void)
{
    DDRB |= 0b01111111;          // set display pins as output (H output)
    
    // timer 0 init
    TIMSK0 |= 1<<TOIE0;
    TCCR0B = (1 << CS02) | (1 << CS00);
}

ISR(TIMER0_OVF_vect)
{
    static uint16_t adc_ovf_count = 0;
    static state_t state_led = ENABLE;
    
    if (++adc_ovf_count > 1000)
    {
        adc_ovf_count = 0;
        switching_status_led(state_led);
        if (state_led == ENABLE)
            state_led = DISABLE;
        else
            state_led = ENABLE;
    }
}