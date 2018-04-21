#include "ptt.h"

void ptt_init(void)
{
    // INT0 init
    PTT_DDR  &= ~(1 << PTT_PIN_NUM);
    //PTT_PORT |= 1 << PTT_PIN_NUM;     // PTT button on port dont have pull-up
    EICRA    |= 1 << ISC00;             // any logical change INT0 generate interrupt

    // timer 0 init
    //TIMSK0 |= 1<<TOIE0;
    
}

//void ptt_timer(state_t state)
//{
    //if (state == ENABLE)
        //TCCR0B = (1 << CS02) | (1 << CS00);
    //else
        //TCCR0B &= ~((1 << CS02) | (1 << CS00));
//}

void ptt_set_irq(state_t state)
{
    (state == ENABLE) ? (EIMSK |= 1 << INT0) : (EIMSK &= ~(1 << INT0));
}

ISR(INT0_vect)
{
    TCNT1 = 65000;
    switching_timer(ENABLE);
}

//ISR(TIMER0_OVF_vect)
//{
    //ptt_timer(DISABLE);
    //if (button_ptt_is_pressed())
    //{
        //switching_state = SWITCHING_ON;
        //pom = "TX";
        //switching_on_sequence();
    //}
    //else
    //{
        //switching_state = SWITCHING_OFF;
        //pom = "RX";
        //switching_off_sequence();
    //}
//}
