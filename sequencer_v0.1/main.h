#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED


#define TSEQ 500       // Time of sequence
#define TREL 5000      // Delay time after servo1 switch on
#define TFAULT 0       // Time after fault * fault_count
#define FCOUNT 10       // Indicate how many repeat will be in fault

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
char* pom;
int zalozni_state;
uint8_t once = 1;


sequencer_t current_state = FAULT;

/*********************************************************************************
*
* Function Name : setup
* Description    : main setup initialization function, which set control
*                  registers for PORTs, interrupts, display initialization
*
*********************************************************************************/
void setup(void)
{
    // Setup for used ports
    DDRB |= 0b01111111;          //display (H output)

    DDRD &= ~(1<<2);             // INT0 as input (L)
    PORTD |= 1<<2;               // INT0 H pull-up, L Hi-impedance

    EICRA |= 1<<ISC00;            // any logical change INT0 generate interrupt
    //| (1<<CS10);   // prescaler 64 CS11 CS10 //256 CS12
    TIMSK1 |= 1<<TOIE1;                // enable interrupt when overflow Timer

    lcd_init(LCD_DISP_ON);                 // initialization display
    lcd_clrscr();               // clear display

    sei();                      //enable all interrupts

    uart_init();
}

void timer1_set_state(state_t state)    // switch, which turn on (1) timer1, or turn off (0)
{
    if (state == ENABLE)
       TCCR1B |= (1<<CS12);
    else
       TCCR1B &= ~(1<<CS12);
}

void button_ptt_set_irq(state_t state)
{
    if (state == ENABLE)
       EIMSK |= 1<<INT0;           // enable INT0
    else
       EIMSK &= ~(1<<INT0);           // enable INT0
}

void way_up(void)

{
    tmr1(0);
    pom = "sw rel1+turn fan   ";
    current_state = EVENT0;
    TCNT1 = TREL;
    uart_puts("bylo zmacknuto tlacitko, zapinam tedy rele 1 a vetron\n");
    tmr1(1);
}

void way_down(void)
{
    tmr1(0);
    pom = "Switch OFF Ucc    ";
    current_state = EVENT1;
    TCNT1 = TSEQ;
    uart_puts("bylo pusteno tlacitko, vypinam Ucc\n");
    tmr1(1);
}

void error(void)
{
    tmr1(0);
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
