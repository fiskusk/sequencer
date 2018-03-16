#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#define  F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "uart.h"
#include "lcd.h"
#include "events.h"



#define TSEQ 500       // Time of sequence
#define TREL 5000      // Delay time after servo1 switch on
#define TFAULT 0       // Time after fault * fault_count
#define FCOUNT 10       // Indicate how many repeat will be in fault


#define button_ptt_is_pressed() bit_is_clear(PIND,2)



void way_up(void);
void way_down(void);
void error(void);


typedef enum{
    EVENT0,	        // on.off rele2 tak, aby se seplo ve stejny cas jako rele 1
    EVENT1,         // on.off bias
    EVENT2,         // on Ucc nebo off rele1
    FAULT,          // chybovy stav, vse vypne... po startu vykona rychle, v pripade chyby vykonava opakovane, tak aby to zabralo 
    AFTER_FAULT,    // zapne ADC, vycte hodnoty, zkontroluje stav, pokud OK nastavi vychozi stav na EVENT0, pøi zmaèknuti PTT se bude dat spustit.. 
    TEST_PTT        // rozhoduje co je s tlacitkem
} sequencer_t;

typedef enum {
    ENABLE = 1,
    DISABLE = !ENABLE,
} state_t;

void timer1_set_state(state_t state);
void button_ptt_set_irq(state_t state);


extern uint8_t way;
extern uint8_t fault_count;
extern uint8_t fault_flag;           // 0 bez poruchy, 1 porucha, 2 prvni zapnuti-test
extern char *pom;
extern uint8_t once;
extern sequencer_t old_state;
extern sequencer_t actual_state;





#endif // MAIN_H_INCLUDED
