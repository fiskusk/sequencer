#include "ptt.h"

void ptt_init(void)
{
    // INT0 init
    PTT_DDR  &= ~(1 << PTT_PIN_NUM);
    //PTT_PORT |= 1 << PTT_PIN_NUM;     // PTT button on port dont have pull-up
    EICRA    |= 1 << ISC00;             // any logical change INT0 generate interrupt
   
}

void ptt_set_irq(state_t state)
{
    (state == ENABLE) ? (EIMSK |= 1 << INT0) : (EIMSK &= ~(1 << INT0));
}

ISR(INT0_vect)
{
    TCNT1 = 64911;
    switching_timer(ENABLE);
}

