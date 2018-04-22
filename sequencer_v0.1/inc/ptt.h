#ifndef PTT_H_INCLUDED
#define PTT_H_INCLUDED

#define PTT_DDR     DDRD
#define PTT_PORT    PORTD
#define PTT_PIN     PIND
#define PTT_PIN_NUM 2

#define button_ptt_is_pressed() bit_is_set(PTT_PIN, PTT_PIN_NUM)

#include "settings.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#include "types.h"
#include "uart.h"
#include "switching.h"


void ptt_init(void);
void ptt_set_irq(state_t state);


#endif // PTT_H_INCLUDED
