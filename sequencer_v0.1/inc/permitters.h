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

// ========================================================
void timer1_set_state(state_t state);    
void adc_set_state(state_t state);
uint8_t loop_repeat(state_t state);
// ========================================================


#endif // PERMITTERS_H
