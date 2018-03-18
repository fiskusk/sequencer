/*
 * permitters.h
 *
 * Created: 16.3.2018 16:48:31
 *  Author: fkla
 */


#ifndef PERMITTERS_H_INCLUDED
#define PERMITTERS_H_INCLUDED

#include "types.h"
#include <avr/io.h>

uint8_t loop_enable;

// ========================================================
void timer1_set_state(state_t state);    
void button_ptt_set_irq(state_t state);
void adc_set_state(state_t state);
void loop_repeat(state_t state);
// ========================================================


#endif // PERMITTERS_H
