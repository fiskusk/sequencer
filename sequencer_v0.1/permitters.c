/*
 * permitters.c
 *
 * Created: 16.3.2018 17:03:26
 *  Author: fkla
 */ 

#include "permitters.h"
uint8_t loop_enable = 0;

void timer1_set_state(state_t state)    // switch, which turn on (1) timer1, or turn off (0)
{
	(state == ENABLE) ? (TCCR1B |= (1<<CS12)) : (TCCR1B &= ~(1<<CS12));
}

void button_ptt_set_irq(state_t state)
{
	(state == ENABLE) ? (EIMSK |= 1<<INT0) : (EIMSK &= ~(1<<INT0));
}

void loop_repeat(state_t state)
{
    loop_enable = (state == ENABLE) ? 1 : 0;
}

void adc_set_state(state_t state)
{
    (state == ENABLE) ? (ADCSRA = (1<<ADIE)) : (ADCSRA &= ~(1<<ADIE));
}