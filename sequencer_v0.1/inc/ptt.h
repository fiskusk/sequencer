#ifndef PTT_H_INCLUDED
#define PTT_H_INCLUDED

#include "settings.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#include "types.h"
#include "main.h"
#include "uart.h"
#include "permitters.h"
#include "switching.h"


void ptt_init(void);
void ptt_timer_set_state(state_t state);
void error(void);
void ptt_set_irq(state_t state);


#endif // PTT_H_INCLUDED
