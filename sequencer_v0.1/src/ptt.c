#include "ptt.h"
#include "switching.h"

/************************************************************************/
/*  Initialization function for PTT button                              */
/************************************************************************/
void ptt_init(void)
{
    // INT0 init
    PTT_DDR  &= ~(1 << PTT_PIN_NUM);    // PTT port as input
    //PTT_PORT |= 1 << PTT_PIN_NUM;     // PTT button on port have pull-up
    EICRA    |= 1 << ISC00;             // any logical change INT0 generate interrupt
   
}/* ptt_init */

/************************************************************************/
/*  Function for enable PTT interruption request                        */
/************************************************************************/
void ptt_set_irq(state_t state)
{
    (state == ENABLE) ? (EIMSK |= 1 << INT0) : (EIMSK &= ~(1 << INT0));
}/* ptt_set_irq */

/************************************************************************/
/*  PTT interruption routine                                            */
/************************************************************************/
ISR(INT0_vect)
{
    switching();
    //TCNT1 = 65535;              // the interruption from timer 1 occurs in 10 ms (64911)
    //switching_timer(ENABLE);    // enable timer 1
}

