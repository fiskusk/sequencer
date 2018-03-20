/*
 * permitters.c
 *
 * Created: 16.3.2018 17:03:26
 *  Author: fkla
 */ 

#include "permitters.h"

void timer1_set_state(state_t state)    // switch, which turn on (1) timer1, or turn off (0)
{
	(state == ENABLE) ? (TCCR1B |= (1<<CS12)) : (TCCR1B &= ~(1<<CS12));
}

uint8_t loop_repeat(state_t state)
{
    uint8_t loop_enable = (state == ENABLE) ? 1 : 0;
    return loop_enable;
}

void adc_set_state(state_t state)
{
    (state == ENABLE) ? (ADCSRA |= (1<<ADIE)) : (ADCSRA &= ~(1<<ADIE));
}