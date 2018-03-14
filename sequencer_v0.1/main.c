/*
 * sequencer_v0.1.c
 *
 * Created: 10.3.2018 20:11:55
 * Author : fkla
 */

#define     F_CPU 16000000UL
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
    uart_puts("Start , vse vypne skokem do fault a provede prvni test v after_fault\n");
    TCNT1 = 65530;
    fault_flag = 2;
    tmr1(1);
    while (1)
    {

        lcd_puts(pom);
        lcd_gotoxy(0,0);
    }
    return 0;
}

ISR(INT0_vect)
{
    tmr1(0);
    TCNT1 = 64910;
    if (once == 1)
    {
        zalozni_state = current_state;
        once = !once;
    }
    current_state = test_PTT;
    uart_puts("Preruseni ISR INT0, skace do test_PTT\n");
    tmr1(1);
}

ISR(TIMER1_OVF_vect)
{
    switch(current_state){
        case event0:
            tmr1(0);
            if (way == 1)
            {
                pom = "Switch ON rel 2    ";
                uart_puts("event0 zapinam rele 2\n");
                TCNT1 = TSeq;
                tmr1(1);
                current_state = event1;
            }
            else
            {
                pom = "Switch OFF rel 2    ";
                uart_puts("event0 vypinam rele 2\n");
                TCNT1 = TRel;
                current_state = event2;
                tmr1(1);
            }
            break;
        case event1:
            tmr1(0);
            if (way == 1)
            {
                pom = "Switch ON bias   ";
                uart_puts("event1 zapinam bias\n");
                TCNT1 = TSeq;
                tmr1(1);
                current_state = event2;
            }
            else
            {
                pom = "Switch OFF bias    ";
                uart_puts("event 1  vypinam bias\n");
                TCNT1 = TSeq;
                tmr1(1);
                current_state = event0;
            }
            break;
        case event2:
           tmr1(0);
           if (way == 1)
           {
               uart_puts("event2 zapinam Ucc\n");
               pom = "Switch ON Ucc     ";
           }
           else
           {
               uart_puts("event2 vypinam rele 2\n");
               pom    = "Switch OFF rel 1";           }
           break;
        case fault:
           tmr1(0);
           ptt(0);
           if (once == 1)
           {
               uart_puts("nastal fault - fault postupne vse vypnu a drz�m delsi dobu, tedy tohle vse vcetne vypinani vseho delam znovu...\n");
               pom = "Switch OFF Ucc     ";
               _delay_ms(TSeq);
               pom = "Switch OFF bias    ";
               _delay_ms(TSeq);
               pom = "switch OFF rel 2   ";
               _delay_ms(TRel);
               pom = "switch OFF rel 1   ";
               pom = "rozmrdals to       ";
               TCNT1 = Tfault;
               fault_count++;
               current_state = fault;
               once = !once;
               tmr1(1);
           }
           else if (fault_count < FCout)
           {
               TCNT1 = Tfault;
               fault_count++;
               current_state = fault;
               uart_puts("fault_count:");
               uart_putc(fault_count);
               uart_puts("\n");
               tmr1(1);
           }
           else
           {
               uart_puts("jak probehne x opakovani faultu (v prvnim startu prednastaveny jenom na jedno projet�), pak zkontroluju jestli je uz vse OK SKOKEM na after fault\n");
               pom = "Checking process   ";
               current_state = after_fault;
               TCNT1 = 65520;
               fault_count = 0;
               once = 1;
               tmr1(1);
           }
           break;
        case after_fault:
            tmr1(0);
            if (fault_flag >= 1)
            {
                uart_puts("Pusteni ADC a cekani na komparaci, nyni simuluji ze vse OK pevnym nastavim fault_flag = 0\n");
                pom = "SW&RD ADC a COMP    ";
                fault_flag = 0;
                current_state = after_fault;
                TCNT1 = 250;
                tmr1(1);
            }
            else
            {
                uart_puts("pokud vse OK, pak skace sem a nastavi curent state event0, tedy umozneno startovat pres tlacitko do UP, DW pokud komparator nespusti fault \n");
                pom = "Vse OK           ";
                ptt(1);
                current_state = event0;
            }
            break;
         case test_PTT:
            tmr1(0);
            uart_puts("jsem v case PTT\n");
            uart_putc(PIND & (1<<2));
            current_state = zalozni_state;
            if (  bit_is_clear(PIND,2) && !((current_state == fault) || (current_state == after_fault)) )
            {
                uart_puts("zapinam\n");
                way = 1;
                way_up();
            }
            else if ( bit_is_set(PIND,2) && !((current_state == fault) || (current_state == after_fault)) )
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
            current_state = fault;
    }
}
