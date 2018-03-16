#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#define  F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define TSEQ 500       // Time of sequence
#define TREL 5000      // Delay time after servo1 switch on
#define TFAULT 0       // Time after fault * fault_count
#define FCOUNT 10       // Indicate how many repeat will be in fault

#define button_ptt_is_pressed() bit_is_clear(PIND,2)

extern uint8_t way;                 // variable distinguish, if PTT button is press or switch off
extern uint8_t fault_count;
extern uint8_t fault_flag;           // 0 bez poruchy, 1 porucha, 2 prvni zapnuti-test
extern char *pom;
extern uint8_t once;
extern sequencer_t old_state;
extern sequencer_t actual_state;

#endif // MAIN_H_INCLUDED
