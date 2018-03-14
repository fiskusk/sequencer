#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED


#define TSEQ 500       // Time of sequence
#define TREL 5000      // Delay time after servo1 switch on
#define TFAULT 0       // Time after fault * fault_count
#define FCOUNT 10       // Indicate how many repeat will be in fault


#define button_ptt_is_pressed() bit_is_clear(PIND,2)


typedef enum{
    EVENT0,
    EVENT1,
    EVENT2,
    FAULT,
    AFTER_FAULT,
    TEST_PTT
} sequencer_t;

typedef enum {
    ENABLE = 1,
    DISABLE = !ENABLE,
} state_t;


uint8_t way = 0;
uint8_t fault_count = 9;
uint8_t fault_flag = 2;           // 0 bez poruchy, 1 porucha, 2 prvni zapnuti-test
char *pom;
uint8_t once = 1;
sequencer_t old_state;
sequencer_t actual_state = FAULT;


void timer1_set_state(state_t state)    // switch, which turn on (1) timer1, or turn off (0)
{
    (state == ENABLE) ? (TCCR1B |= (1<<CS12)) : (TCCR1B &= ~(1<<CS12));
}

void button_ptt_set_irq(state_t state)
{
    (state == ENABLE) ? (EIMSK |= 1<<INT0) : (EIMSK &= ~(1<<INT0));
}

void way_up(void)
{
    timer1_set_state(DISABLE);
    pom = "sw rel1+turn fan   ";
    actual_state = EVENT0;
    TCNT1 = TREL;
    uart_puts("bylo zmacknuto tlacitko, zapinam tedy rele 1 a vetron\n");
    timer1_set_state(ENABLE);
}

void way_down(void)
{
    timer1_set_state(DISABLE);
    pom = "Switch OFF Ucc    ";
    actual_state = EVENT1;
    TCNT1 = TSEQ;
    uart_puts("bylo pusteno tlacitko, vypinam Ucc\n");
    timer1_set_state(ENABLE);
}

void error(void)
{
    timer1_set_state(DISABLE);
    if (fault_flag == 1)
    {
        pom = "nastala chyba       ";
        uart_puts("Po stlaceni byla nastava chyba, nemohu se spustit\n");
    }
    else
    {
        pom = "Divny, Vse OK          ";
        uart_puts("To je divny, vse je uz ok\n");
    }
}

#endif // MAIN_H_INCLUDED
