/*
 * permitters.h
 *
 * Created: 16.3.2018 16:48:31
 *  Author: fkla
 */ 


#ifndef PERMITTERS_H_INCLUDED
#define PERMITTERS_H_INCLUDED

#include "state.h"
#include <avr/io.h>

// ========================================================
extern	void timer1_set_state(state_t state);    // switch, which turn on (1) timer1, or turn off (0)
extern	void button_ptt_set_irq(state_t state);
// ========================================================


#endif // PERMITTERS_H 