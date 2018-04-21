#ifndef SWITCHING_H_INCLUDED
#define SWITCHING_H_INCLUDED

#include <avr/interrupt.h>
#include <avr/io.h>

#include "types.h"
#include "uart.h"
#include "adc.h"


#define SWITCHING_RELAY1_DDR     DDRC
#define SWITCHING_RELAY2_DDR     DDRC
#define SWITCHING_BIAS_DDR       DDRD
#define SWITCHING_UCC_DDR        DDRD
#define SWITCHING_FAN_DDR        DDRD

#define SWITCHING_RELAY1_PORT    PORTC
#define SWITCHING_RELAY2_PORT    PORTC
#define SWITCHING_BIAS_PORT      PORTD
#define SWITCHING_UCC_PORT       PORTD
#define SWITCHING_FAN_PORT       PORTD

#define SWITCHING_RELAY1_PIN_NUM 4
#define SWITCHING_RELAY2_PIN_NUM 5
#define SWITCHING_BIAS_PIN_NUM   7
#define SWITCHING_UCC_PIN_NUM    5
#define SWITCHING_FAN_PIN_NUM    3


#define SWITCHING_RELAY1_ON  (SWITCHING_RELAY1_PORT |= 1 << SWITCHING_RELAY1_PIN_NUM)
#define SWITCHING_RELAY1_OFF (SWITCHING_RELAY1_PORT &= ~(1 << SWITCHING_RELAY1_PIN_NUM))

#define SWITCHING_RELAY2_ON  (SWITCHING_RELAY2_PORT |= 1 << SWITCHING_RELAY2_PIN_NUM)
#define SWITCHING_RELAY2_OFF (SWITCHING_RELAY2_PORT &= ~(1 << SWITCHING_RELAY2_PIN_NUM))

#define SWITCHING_BIAS_ON    (SWITCHING_BIAS_PORT |= 1 << SWITCHING_BIAS_PIN_NUM)
#define SWITCHING_BIAS_OFF   (SWITCHING_BIAS_PORT &= ~(1 << SWITCHING_BIAS_PIN_NUM))

#define SWITCHING_UCC_ON     (SWITCHING_UCC_DDR |= 1 << SWITCHING_UCC_PIN_NUM)
#define SWITCHING_UCC_OFF    (SWITCHING_UCC_PORT &= ~(1 << SWITCHING_UCC_PIN_NUM))

#define SWITCHING_FAN_ON     (SWITCHING_FAN_PORT |= 1 << SWITCHING_FAN_PIN_NUM)
#define SWITCHING_FAN_OFF    (SWITCHING_FAN_PORT &= ~(1 << SWITCHING_FAN_PIN_NUM))


typedef enum {
    SWITCHING_OFF,
    SWITCHING_ON
} switching_t;

typedef enum {
    RELAY1_AND_FAN,
    RELAY2,
    BIAS,
    UCC
} sequencer_t;

extern sequencer_t machine_state; // default after start up device, go to fault event
extern switching_t switching_state;

void switching_timer(state_t state);
void switching_init(void);
void switching_relay1(state_t state);
void switching_relay2(state_t state);
void switching_bias(state_t state);
void switching_ucc(state_t state);
void switching_fan(state_t state);
void switching_on_sequence(void);
void switching_off_sequence(void);


#endif // SWITCHING_H_INCLUDED
