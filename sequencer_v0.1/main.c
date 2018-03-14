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

#include "uart.h"
#include "lcd.h"
#include "main.h"

int main(void)
{
    setup();
    pom = "Pok";
    uart_puts("Start , vse vypne skokem do FAULT a provede prvni test v AFTER_FAULT\n");
    TCNT1 = 65530;
    fault_flag = 2;
    timer1_set_state(ENABLE);
    while (1)
    {

        lcd_puts(pom);
        lcd_gotoxy(0,0);
    }
    return 0;
}

ISR(INT0_vect)
{
    timer1_set_state(DISABLE);
    TCNT1 = 64910;
    if (once == 1)
    {
        zalozni_state = current_state;
        once = !once;
    }
    current_state = TEST_PTT;
    uart_puts("Preruseni ISR INT0, skace do test_PTT\n");
    timer1_set_state(ENABLE);
}

ISR(TIMER1_OVF_vect)
{
    switch(current_state){
        case EVENT0:
            timer1_set_state(DISABLE);
            if (way == 1)
            {
                pom = "Switch ON rel 2    ";
                uart_puts("EVENT0 zapinam rele 2\n");
                TCNT1 = TSEQ;
                timer1_set_state(ENABLE);
                current_state = EVENT1;
            }
            else
            {
                pom = "Switch OFF rel 2    ";
                uart_puts("EVENT0 vypinam rele 2\n");
                TCNT1 = TREL;
                current_state = EVENT2;
                timer1_set_state(ENABLE);
            }
            break;
        case EVENT1:
            timer1_set_state(DISABLE);
            if (way == 1)
            {
                pom = "Switch ON bias   ";
                uart_puts("EVENT1 zapinam bias\n");
                TCNT1 = TSEQ;
                timer1_set_state(ENABLE);
                current_state = EVENT2;
            }
            else
            {
                pom = "Switch OFF bias    ";
                uart_puts("EVENT 1  vypinam bias\n");
                TCNT1 = TSEQ;
                timer1_set_state(ENABLE);
                current_state = EVENT0;
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
               uart_puts("EVENT2 vypinam rele 2\n");
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
               current_state = FAULT;
               once = !once;
               timer1_set_state(ENABLE);
           }
           else if (fault_count < FCOUNT)
           {
               TCNT1 = TFAULT;
               fault_count++;
               current_state = FAULT;
               uart_puts("FAULT_count:");
               uart_putc(fault_count);
               uart_puts("\n");
               timer1_set_state(ENABLE);
           }
           else
           {
               uart_puts("jak probehne x opakovani FAULTu (v prvnim startu prednastaveny jenom na jedno projet�), pak zkontroluju jestli je uz vse OK SKOKEM na after FAULT\n");
               pom = "Checking process   ";
               current_state = AFTER_FAULT;
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
                current_state = AFTER_FAULT;
                TCNT1 = 250;
                timer1_set_state(ENABLE);
            }
            else
            {
                uart_puts("pokud vse OK, pak skace sem a nastavi curent state EVENT0, tedy umozneno startovat pres tlacitko do UP, DW pokud komparator nespusti FAULT \n");
                pom = "Vse OK           ";
                button_ptt_set_irq(ENABLE);
                current_state = EVENT0;
            }
            break;
         case TEST_PTT:
            timer1_set_state(DISABLE);
            uart_puts("jsem v case PTT\n");
            uart_putc(PIND & (1<<2));
            current_state = zalozni_state;
            if (  bit_is_clear(PIND,2) && !((current_state == FAULT) || (current_state == AFTER_FAULT)) )
            {
                uart_puts("zapinam\n");
                way = 1;
                way_up();
            }
            else if ( bit_is_set(PIND,2) && !((current_state == FAULT) || (current_state == AFTER_FAULT)) )
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
            current_state = FAULT;
    }
}
