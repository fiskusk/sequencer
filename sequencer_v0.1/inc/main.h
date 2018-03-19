#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include "types.h"

#define TSEQ   500  // Time delay between of two sequence
#define TREL   5000 // Delay between servo1 and servo2. They must switch simultaneously.
#define TFAULT 0    // Preset register Timer1 aprox. fault * F_COUNT
#define FCOUNT 4    // Indicate how many repeats will be in fault


#define button_ptt_is_pressed() bit_is_clear(PIND, 2)

extern uint8_t way;
extern uint8_t fault_count;
extern uint8_t fault_flag;
extern char *pom;
extern sequencer_t old_state;
extern sequencer_t machine_state;
extern uint8_t once_fault_event;
extern uint8_t once_ptt_event;

#endif // MAIN_H_INCLUDED
