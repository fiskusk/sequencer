#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#define  F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "types.h"

#define TSEQ 500       // Time delay between of two sequence
#define TREL 5000      // Delay between servo1 and servo2. They must switch simultaneously.
#define TFAULT 0       // Preset register Timer1 aprox. fault * F_COUNT
#define FCOUNT 10      // Indicate how many repeats will be in fault
#define UMIN 50
#define UMAX 1024

#define button_ptt_is_pressed() bit_is_clear(PIND,2)

extern uint8_t way;                 
extern uint8_t fault_count;
extern uint8_t fault_flag;           
extern char *pom;
extern sequencer_t old_state;
extern sequencer_t actual_state;

#endif // MAIN_H_INCLUDED
