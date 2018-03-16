/*
 * sequencer_v0.1.c
 *
 * Created: 10.3.2018 20:11:55
 * Author : fkla
 */

#define  F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "main.h"
#include "uart.h"
#include "lcd.h"
#include "events.h"

uint8_t way = 0;
uint8_t fault_count = 9;
uint8_t fault_flag = 2;           // 0 bez poruchy, 1 porucha, 2 prvni zapnuti-test
char *pom;
uint8_t once = 1;
sequencer_t old_state;
sequencer_t actual_state = FAULT;

void setup(void)
{
    DDRB |= 0b01111111;    //display (H output)
    DDRD &= ~(1<<2);       // INT0 as input (L)
    PORTD |= 1<<2;         // INT0 H pull-up, L Hi-impedance

    EICRA |= 1<<ISC00;     // any logical change INT0 generate interrupt
    TIMSK1 |= 1<<TOIE1;    // enable interrupt when overflow Timer

    lcd_init(LCD_DISP_ON); // initialization display
    lcd_clrscr();          // clear display

    uart_init();
    sei();                 //enable all interrupts
}

int main(void)
{
    setup();
    pom = "Pok";
    uart_puts("Start , vse vypne skokem do FAULT a provede prvni test v AFTER_FAULT\n");
    TCNT1 = 65530;
    fault_flag = 2;
    timer1_set_state(ENABLE);
    lcd_puts(pom);
    lcd_gotoxy(0,0);
    while (1);
    return 0;
}

ISR(INT0_vect)
{
    Event_PTT_button_status_changed();
}

ISR(TIMER1_OVF_vect)
{
    switch(actual_state)
    {
        case EVENT0:
            E0_oNoFF_relay2();
            break;
        case EVENT1:
            E1_oNoFF_bias();
            break;
        case EVENT2:
            E2_oN_Ucc_oFF_relay1();
            break;
        case FAULT:
            Fault_oFF_all();
            break;
        case AFTER_FAULT:
            After_Fault_check_status();
            break;
        case TEST_PTT:
            Test_state_of_PTT_button();
            break;
        default:
            actual_state = FAULT;
    }
}
