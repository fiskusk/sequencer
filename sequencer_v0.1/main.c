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

ISR(INT0_vect)
{
    timer1_set_state(DISABLE);
    TCNT1 = 64910;
    if (once == 1)
    {
        old_state = actual_state;
        once = !once;
    }
    actual_state = TEST_PTT;
    uart_puts("Preruseni ISR INT0, skace do test_PTT\n");
    timer1_set_state(ENABLE);
}

ISR(TIMER1_OVF_vect)
{
    switch(actual_state)
    {
        case EVENT0:
            timer1_set_state(DISABLE);
            E0_on_relay2();
            break;
        case EVENT1:
            timer1_set_state(DISABLE);
            if (way == 1)
            {
                pom = "Switch ON bias   ";
                uart_puts("EVENT1 zapinam bias\n");
                TCNT1 = TSEQ;
                timer1_set_state(ENABLE);
                actual_state = EVENT2;
            }
            else
            {
                pom = "Switch OFF bias    ";
                uart_puts("EVENT 1  vypinam bias\n");
                TCNT1 = TSEQ;
                timer1_set_state(ENABLE);
                actual_state = EVENT0;
            }
            break;
        case EVENT2:
           timer1_set_state(DISABLE);
           if (way == 1)
           {
               uart_puts("EVENT2 zapinam Ucc\n");
               pom = "Switch ON Ucc     ";
           }
           else
           {
               uart_puts("EVENT2 vypinam rele 1\n");
               pom    = "Switch OFF rel 1";           }
           break;
        case FAULT:
           timer1_set_state(DISABLE);
           button_ptt_set_irq(DISABLE);
           if (once == 1)
           {
               uart_puts("nastal FAULT - FAULT postupne vse vypnu a drz�m delsi dobu, tedy tohle vse vcetne vypinani vseho delam znovu...\n");
               pom = "Switch OFF Ucc     ";
               _delay_ms(TSEQ);
               pom = "Switch OFF bias    ";
               _delay_ms(TSEQ);
               pom = "switch OFF rel 2   ";
               _delay_ms(TREL);
               pom = "switch OFF rel 1   ";
               pom = "rozmrdals to       ";
               TCNT1 = TFAULT;
               fault_count++;
               actual_state = FAULT;
               once = !once;
               timer1_set_state(ENABLE);
           }
           else if (fault_count < FCOUNT)
           {
               TCNT1 = TFAULT;
               fault_count++;
               actual_state = FAULT;
               uart_puts("FAULT_count:");
               uart_putc(fault_count);
               uart_puts("\n");
               timer1_set_state(ENABLE);
           }
           else
           {
               uart_puts("jak probehne x opakovani FAULTu (v prvnim startu prednastaveny jenom na jedno projet�), pak zkontroluju jestli je uz vse OK SKOKEM na after FAULT\n");
               pom = "Checking process   ";
               actual_state = AFTER_FAULT;
               TCNT1 = 65520;
               fault_count = 0;
               once = 1;
               timer1_set_state(ENABLE);
           }
           break;
        case AFTER_FAULT:
            timer1_set_state(DISABLE);
            if (fault_flag >= 1)
            {
                uart_puts("Pusteni ADC a cekani na komparaci, nyni simuluji ze vse OK pevnym nastavim fault_flag = 0\n");
                pom = "SW&RD ADC a COMP    ";
                fault_flag = 0;
                actual_state = AFTER_FAULT;
                TCNT1 = 250;
                timer1_set_state(ENABLE);
            }
            else
            {
                uart_puts("pokud vse OK, pak skace sem a nastavi curent state EVENT0, tedy umozneno startovat pres tlacitko do UP, DW pokud komparator nespusti FAULT \n");
                pom = "Vse OK           ";
                button_ptt_set_irq(ENABLE);
                actual_state = EVENT0;
            }
            break;
         case TEST_PTT:
            timer1_set_state(DISABLE);
            uart_puts("jsem v case PTT\n");
            uart_putc(PIND & (1<<2));
            actual_state = old_state;
            if (  button_ptt_is_pressed() && actual_state != FAULT && actual_state != AFTER_FAULT )
            {
                uart_puts("zapinam\n");
                way = 1;
                way_up();
            }
            else if ( !button_ptt_is_pressed() && actual_state != FAULT && actual_state != AFTER_FAULT )
            {
                uart_puts("vypinam\n");
                way = 0;
                way_down();
            }
            else
            {
                uart_puts("Tlacis ale nezapnu se, mam poruchu\n");
                error();
            }
            break;
        default:
            actual_state = FAULT;
    }
}
