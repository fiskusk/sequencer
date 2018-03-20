#include "events.h"

void E0_on_off_relay2(void)
{
    switching_timer_set_state(DISABLE);
    if (way)
    {
        pom = "Switch ON rel 2    ";
        uart_puts("EVENT0 zapinam rele 2\n");
        TCNT1 = TSEQ;
        switching_timer_set_state(ENABLE);
        machine_state = EVENT1;
    }
    else
    {
        pom = "Switch OFF rel 2    ";
        uart_puts("EVENT0 vypinam rele 2\n");
        TCNT1        = TREL;
        machine_state = EVENT2;
        switching_timer_set_state(ENABLE);
    }
}

void E1_on_off_bias(void)
{
    switching_timer_set_state(DISABLE);
    if (way)
    {
        pom = "Switch ON bias   ";
        uart_puts("EVENT1 zapinam bias\n");
        TCNT1 = TSEQ;
        switching_timer_set_state(ENABLE);
        machine_state = EVENT2;
    }
    else
    {
        pom = "Switch OFF bias    ";
        uart_puts("EVENT 1  vypinam bias\n");
        TCNT1 = TSEQ;
        switching_timer_set_state(ENABLE);
        machine_state = EVENT0;
    }
}

void E2_on_ucc_off_relay1(void)
{
    switching_timer_set_state(DISABLE);
    if (way)
    {
        uart_puts("EVENT2 zapinam Ucc\n");
        pom = "Switch ON Ucc     ";
    }
    else
    {
        uart_puts("EVENT2 vypinam rele 1\n");
        pom = "Switch OFF rel 1";
    }
}

void fault_off_all(void)
{
    switching_timer_set_state(DISABLE);                                             // TIMER1 stops
    ptt_set_irq(DISABLE);                                           // ISR from PTT button disable, block transmit
    if (once_fault_event == 1 && ((fault_flag == 1) || (fault_flag == 2))) // do this only once - turn all down if fault
    {
        // PORTC ^= (1<<5);
        uart_puts("nastal FAULT - FAULT postupne vse vypnu a drz?m delsi dobu, tedy tohle vse vcetne vypinani vseho delam znovu...\n");
        fault_flag = 3; // set fault flag to after_fault state
        adc_set_state(ENABLE);

        // test prints


        // set Timer counter value register to time delay for block transmit
        TCNT1 = TFAULT;
        // number of repeats to achieve aim delay time about 20 seconds
        // if first run, fault_count set to short time delay
        fault_count++;
        machine_state = FAULT; // go to fault in ISR timer1

        once_fault_event = loop_repeat(DISABLE);
        switching_timer_set_state(ENABLE); // set on timer1
    }
    // if fault flag came from ADC, after previous loop jump here and repeats to FCOUNT
    // FCOUNT is set to keep this block for 20 second
    else if (fault_count < FCOUNT)
    {
        TCNT1 = TFAULT;
        fault_count++;
        machine_state = FAULT; // still hold here

        // test prints
        uart_puts("FAULT_count: ");
        uart_putc(fault_count);
        uart_puts("\n");

        switching_timer_set_state(ENABLE);
    }
    // when accomplish previous rule, program jump here and set state to check status PA ability
    else
    {
        // test prints
        uart_puts("vse vypnuto, delay dosahnut. Nasleduje kontrola v AFTER_FAULT\n");

        machine_state = AFTER_FAULT; // go to test device

        once_fault_event = loop_repeat(ENABLE);
        TIFR1 |= 1 << TOV1;       // jump quickly to ISR Timer1 into AFTER_FAULT
        switching_timer_set_state(ENABLE); // run TIMER1
    }
} /* fault_off_all */

// this function set ADC on and check status of PA ability
void after_fault_check_status(void)
{
    switching_timer_set_state(DISABLE);
    if (fault_flag > 0)
    {
        fault_flag = 1;
        uart_puts("Byla chyba, kontrola jestli je vse OK\n");
        pom = "SW&RD ADC a COMP    ";
    }
    else
    {
        uart_puts("Vse je OK, nyni je povoleno PTT\n");
        pom = "Vse OK           ";
        ptt_set_irq(ENABLE);
        machine_state = EVENT0;
    }
}
