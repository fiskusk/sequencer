#include "ptt.h"

#define PTT_DDR     DDRD
#define PTT_PORT    PORTD
#define PTT_PIN     PIND
#define PTT_PIN_NUM 2

#define button_ptt_is_pressed() bit_is_clear(PTT_PIN, PTT_PIN_NUM)

void ptt_init(void)
{
    // INT0 init
    PTT_DDR &= ~(1<<PTT_PIN_NUM);
    PTT_PORT |= 1<<PTT_PIN_NUM;
    EICRA |= 1<<ISC00;                   // any logical change INT0 generate interrupt
    
    // timer 0 init
    TIMSK0 = TOIE0;
}

void ptt_timer_set_state(state_t state)    // switch, which turn on (1) timer0, or turn off (0)
{
    (state == ENABLE) ? (TCCR0B = (1<<CS02) | (1<<CS00)) : (TCCR0B &= ~((1<<CS02) | (1<<CS00)));
}

ISR(INT0_vect)
{
    TCNT0 = 156;
    ptt_timer_set_state(ENABLE);
}

ISR(TIMER0_OVF_vect)
{
    ptt_timer_set_state(DISABLE);
    if (button_ptt_is_pressed() && machine_state != FAULT && machine_state != AFTER_FAULT)
    {
        switching_relay1(SWITCHING_ON);
        machine_state = EVENT0;
        TCNT1         = TREL;
        switching_timer_set_state(ENABLE);
    }
    else if (!button_ptt_is_pressed() && machine_state != FAULT && machine_state != AFTER_FAULT)
    {
        switching_ucc(SWITCHING_OFF);
        machine_state = EVENT1;
        TCNT1        = TSEQ;
        switching_timer_set_state(ENABLE);
    }
    else
    {
        error();
    }
}

void error(void)
{    // test prints --- SEM SE TO PODLE MÌ NEMÁ NIKDY ABSOLUTNÌ NIKDY JAK DOSTAT
    uart_puts("Tlacis ale nezapnu se, mam poruchu\n");
    pom = "nastala chyba       ";

    ptt_set_irq(DISABLE); // deny next PTT interrupt
    machine_state = FAULT;        // next EVENT will be FAULT
    TIFR1 |= 1 << TOV1;       // jump quickly to ISR Timer1 into AFTER_FAULT
    switching_timer_set_state(ENABLE);
}

void ptt_set_irq(state_t state)
{
    (state == ENABLE) ? (EIMSK |= 1<<INT0) : (EIMSK &= ~(1<<INT0));
}
