#ifndef SWITCHING_H_INCLUDED
#define SWITCHING_H_INCLUDED

#include <avr/interrupt.h>
#include <avr/io.h>

#include "types.h"
#include "main.h"

typedef enum {
    SWITCHING_OFF,
    SWITCHING_ON
} switching_t;

typedef enum{
    EVENT0,
    EVENT1,
    EVENT2,
} sequencer_t;

sequencer_t machine_state;                       // default after start up device, go to fault event
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
