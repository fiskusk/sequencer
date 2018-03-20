/*
 * permitters.c
 *
 * Created: 16.3.2018 17:03:26
 *  Author: fkla
 */ 

#include "permitters.h"



uint8_t loop_repeat(state_t state)
{
    uint8_t loop_enable = (state == ENABLE) ? 1 : 0;
    return loop_enable;
}

void adc_set_state(state_t state)
{
    (state == ENABLE) ? (ADCSRA |= (1<<ADIE)) : (ADCSRA &= ~(1<<ADIE));
}