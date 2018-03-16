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

void button_ptt_set_irq(state_t state)
{
	(state == ENABLE) ? (EIMSK |= 1<<INT0) : (EIMSK &= ~(1<<INT0));
}