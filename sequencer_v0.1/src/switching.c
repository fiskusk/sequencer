#include "switching.h"

#define SWITCHING_RELAY1_DDR DDRD
#define SWITCHING_RELAY2_DDR DDRD
#define SWITCHING_BIAS_DDR DDRD
#define SWITCHING_UCC_DDR DDRD
#define SWITCHING_FAN_DDR DDRC

#define SWITCHING_RELAY1_PORT PORTD
#define SWITCHING_RELAY2_PORT PORTD
#define SWITCHING_BIAS_PORT PORTD
#define SWITCHING_UCC_PORT PORTD
#define SWITCHING_FAN_PORT PORTC

#define SWITCHING_RELAY1_PIN_NUM 3
#define SWITCHING_RELAY2_PIN_NUM 5
#define SWITCHING_BIAS_PIN_NUM 6
#define SWITCHING_UCC_PIN_NUM 7
#define SWITCHING_FAN_PIN_NUM 5


#define SWITCHING_RELAY1_ON (SWITCHING_RELAY1_PORT |= 1<<SWITCHING_RELAY1_PIN_NUM)
#define SWITCHING_RELAY1_OFF (SWITCHING_RELAY1_PORT &= ~(1<<SWITCHING_RELAY1_PIN_NUM))

#define SWITCHING_RELAY2_ON (SWITCHING_RELAY2_PORT |= 1<<SWITCHING_RELAY2_PIN_NUM)
#define SWITCHING_RELAY2_OFF (SWITCHING_RELAY2_PORT &= ~(1<<SWITCHING_RELAY2_PIN_NUM))

#define SWITCHING_BIAS_ON (SWITCHING_BIAS_PORT |= 1<<SWITCHING_BIAS_PIN_NUM)
#define SWITCHING_BIAS_OFF (SWITCHING_BIAS_PORT &= ~(1<<SWITCHING_BIAS_PIN_NUM))

#define SWITCHING_UCC_ON (SWITCHING_UCC_DDR |= 1<<SWITCHING_UCC_PIN_NUM)
#define SWITCHING_UCC_OFF (SWITCHING_UCC_PORT &= ~(1<<SWITCHING_UCC_PIN_NUM))

#define SWITCHING_FAN_ON (SWITCHING_FAN_PORT |= 1<<SWITCHING_FAN_PIN_NUM)
#define SWITCHING_FAN_OFF (SWITCHING_FAN_PORT &= ~(1<<SWITCHING_FAN_PIN_NUM))

sequencer_t machine_state = FAULT;
switching_t switching_state;

void switching_init(void)
{
    SWITCHING_RELAY1_DDR |= 1<<SWITCHING_RELAY1_PIN_NUM;
    SWITCHING_RELAY2_DDR |= 1<<SWITCHING_RELAY2_PIN_NUM;
    SWITCHING_BIAS_DDR |= 1<<SWITCHING_BIAS_PIN_NUM;
    SWITCHING_UCC_DDR |= 1<<SWITCHING_UCC_PIN_NUM;
    SWITCHING_FAN_DDR |= 1<<SWITCHING_FAN_PIN_NUM;

    SWITCHING_RELAY1_PORT &= ~(1<<SWITCHING_RELAY1_PIN_NUM);
    SWITCHING_RELAY2_PORT &= ~(1<<SWITCHING_RELAY2_PIN_NUM);
    SWITCHING_BIAS_PORT &= ~(1<<SWITCHING_BIAS_PIN_NUM);
    SWITCHING_UCC_PORT &= ~(1<<SWITCHING_UCC_PIN_NUM);
    SWITCHING_FAN_PORT &= ~(1<<SWITCHING_FAN_PIN_NUM);

    TIMSK1 |= 1<<TOIE1;
}

void switching_timer(state_t state)    // switch, which turn on (1) timer1, or turn off (0)
{
	(state == ENABLE) ? (TCCR1B |= (1<<CS12)) : (TCCR1B &= ~(1<<CS12));
}

void switching_relay1(state_t state)
{
    uart_puts("ON/OFF rele 1\n");
    pom = "sw rel1 ON/OFF   ";
    (state == ENABLE) ? SWITCHING_RELAY1_ON : SWITCHING_RELAY1_OFF;
}

void switching_relay2(state_t state)
{
    uart_puts("ON/OFF rele 2\n");
    pom = "sw rel2 ON/OFF   ";
    (state == ENABLE) ? (SWITCHING_RELAY2_ON) : (SWITCHING_RELAY2_OFF);
}

void switching_bias(state_t state)
{
    uart_puts("ON/OFF bias\n");
    pom = "sw bias ON/OFF   ";
    (state == ENABLE) ? (SWITCHING_BIAS_ON) : (SWITCHING_BIAS_OFF);
}

void switching_ucc(state_t state)
{
    pom = "ON/OFF Ucc    ";
    uart_puts("sw Ucc ON/OFF\n");
    (state == ENABLE) ? (SWITCHING_UCC_ON) : (SWITCHING_UCC_OFF);
}

void switching_fan(state_t state)
{
    uart_puts("ON/OFF fan\n");
    pom = "sw fan ON/OFF   ";
    (state == ENABLE) ? (SWITCHING_FAN_ON) : (SWITCHING_FAN_OFF);
}

void switching_on_sequence(void)
{
    switch(machine_state) // switch jenom na zapnut�,... nakonec celej ten switch jedna funkce switching_on_sequenc
    {
    case EVENT0:
        switching_relay2(switching_state);
        TCNT1 = TSEQ;
        machine_state = EVENT1;
        switching_timer(ENABLE);
        break;
    case EVENT1:
        switching_bias(switching_state);
        TCNT1 = TSEQ;
        machine_state = EVENT2;
        switching_timer(ENABLE);
        break;
    case EVENT2:
        switching_ucc(switching_state);
        break;  //koncit to bude vypnutim timeru, chybovy stavy vyhod dopi�e
    }
}

void switching_off_sequence(void)
{
    switch(machine_state) // switch jenom na zapnut�,... nakonec celej ten switch jedna funkce switching_on_sequenc
    {
    case EVENT0:
        switching_bias(switching_state);
        TCNT1           = TSEQ;
        machine_state   = EVENT1;
        switching_timer(ENABLE);
        break;
    case EVENT1:
        switching_relay2(switching_state);
        TCNT1           = TREL;
        machine_state   = EVENT2;
        switching_timer(ENABLE);
        break;
    case EVENT2:
        switching_relay1(switching_state);
        break;
    }
}

ISR(TIMER1_OVF_vect)
{
    switching_timer(DISABLE);
    if (state == ENABLE)
    {
        switching_on_sequence();
    }
    else
    {
        switching_off_sequence();
    }

}
