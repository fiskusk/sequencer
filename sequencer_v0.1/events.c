#include "events.h"

/*
* Event for PTT is pushed up and device turn relay1
* and FAN independently directly.
* Next state is EVENT0 - turn relay2.
* They should be turned on simultaneously with relay1.
*/
void event_PTT_pushed_up_on_relay1_on_FAN(void)
{
    timer1_set_state(DISABLE);
    pom = "sw rel1+turn fan   ";
    actual_state = EVENT0;
    TCNT1        = TREL;

    // test prints
    uart_puts("bylo zmacknuto tlacitko, zapinam tedy rele 1 a vetron\n");

    timer1_set_state(ENABLE);
}

/*
* Event for PTT is pushed down and device turn off Ucc.
* Next state is EVENT1 - (way = 0) turn off bias.
*/
void event_PTT_pushed_down_off_Ucc(void)
{
    timer1_set_state(DISABLE);
    actual_state = EVENT1;
    TCNT1        = TSEQ;

    // test prints
    pom = "Switch OFF Ucc    ";
    uart_puts("bylo pusteno tlacitko, vypinam Ucc\n");

    timer1_set_state(ENABLE);
}

/*
* If pressed or push down PTT and was fault, these function block
*/
void error(void)
{
    timer1_set_state(DISABLE);

    // test prints
    uart_puts("Tlacis ale nezapnu se, mam poruchu\n");
    pom = "nastala chyba       ";

    button_ptt_set_irq(DISABLE);        // deny next PTT interrupt
    actual_state = FAULT;                // next EVENT will be FAULT
    TCNT1 = 65520;                      // jump immediately o ISR_TIMER1
    timer1_set_state(ENABLE);
   }

void E0_on_off_relay2(void)
{
    timer1_set_state(DISABLE);
    if (way)
    {
        pom = "Switch ON rel 2    ";
        uart_puts("EVENT0 zapinam rele 2\n");
        TCNT1 = TSEQ;
        timer1_set_state(ENABLE);
        actual_state = EVENT1;
    }
    else
    {
        pom = "Switch OFF rel 2    ";
        uart_puts("EVENT0 vypinam rele 2\n");
        TCNT1        = TREL;
        actual_state = EVENT2;
        timer1_set_state(ENABLE);
    }
}

void E1_on_off_bias(void)
{
    timer1_set_state(DISABLE);
    if (way)
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
}

void E2_on_Ucc_off_relay1(void)
{
    timer1_set_state(DISABLE);
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

/*
* In these function deciding, if button after 10ms sequence
* was pressed/push off.
* According to this, it calls the appropriate functions.
*/
void test_state_of_PTT_button(void)
{
    timer1_set_state(DISABLE);

    // test prints
    uart_puts("jsem v case PTT\n");

    // recover main statement
    actual_state = old_state;

    // if button is pressed and at the same time was not fault or after_fault
    if (button_ptt_is_pressed() && actual_state != FAULT && actual_state != AFTER_FAULT)
    {
        //test prints
        uart_puts("zapinam\n");

        way = 1;
        event_PTT_pushed_up_on_relay1_on_FAN();
    }
    // if button is push down and at the same time was not fault or after_fault
    else if (!button_ptt_is_pressed() && actual_state != FAULT && actual_state != AFTER_FAULT)
    {
        // test prints
        uart_puts("vypinam\n");

        way = 0;
        event_PTT_pushed_down_off_Ucc();
    }
    // when was fault or after_fault
    else
    {
        error();
    }
}

/* In this function set TIMER1 to achieve 10ms delay
*  after change status of button and set state TEST_PTT.
* State TEST_PTT deciding, if button after three 10ms sequence
* was pressed/push off or during these 30 seconds was button glitch.
* According to this, it calls the appropriate functions.
*/
void event_PTT_button_status_changed(void)
{
    timer1_set_state(DISABLE);
    TCNT1 = 64910;
    if (once_PTT_event == 1)
    {
        old_state = actual_state;
        once_PTT_event = loop_repeat(DISABLE);
    }
    actual_state = TEST_PTT;
    uart_puts("Preruseni ISR INT0, skace do test_PTT\n");
    timer1_set_state(ENABLE);
}

void fault_off_all(void)
{
    timer1_set_state(DISABLE);   // TIMER1 stops
    button_ptt_set_irq(DISABLE); // ISR from PTT button disable, block transmit
    if (once_fault_event == 1 && ((fault_flag == 1) || (fault_flag == 2)))                    // do this only once - turn all down if fault
    {
        //PORTC ^= (1<<5);
        uart_puts("nastal FAULT - FAULT postupne vse vypnu a drz?m delsi dobu, tedy tohle vse vcetne vypinani vseho delam znovu...\n");
        fault_flag = 3;                 // set fault flag to after_fault state
        adc_set_state(ENABLE);

        // test prints


        // set Timer counter value register to time delay for block transmit
        TCNT1 = TFAULT;
        // number of repeats to achieve aim delay time about 20 seconds
        // if first run, fault_count set to short time delay
        fault_count++;
        actual_state = FAULT;     // go to fault in ISR timer1

        once_fault_event = loop_repeat(DISABLE);
        timer1_set_state(ENABLE); // set on timer1
    }
    // if fault flag came from ADC, after previous loop jump here and repeats to FCOUNT
    // FCOUNT is set to keep this block for 20 second
    else if (fault_count < FCOUNT)
    {
        TCNT1 = TFAULT;
        fault_count++;
        actual_state = FAULT;       // still hold here

        // test prints
        uart_puts("FAULT_count: ");
        uart_putc(fault_count);
        uart_puts("\n");

        timer1_set_state(ENABLE);
    }
    // when accomplish previous rule, program jump here and set state to check status PA ability
    else
    {
        // test prints
        uart_puts("vse vypnuto, delay dosahnut. Nasleduje kontrola v AFTER_FAULT\n");

        actual_state = AFTER_FAULT;     // go to test device

        once_fault_event = loop_repeat(ENABLE);
        TIFR1 |= 1<<TOV1;      // jump quickly to ISR Timer1 into AFTER_FAULT
        timer1_set_state(ENABLE); // run TIMER1
    }
} /* fault_off_all */

// this function set ADC on and check status of PA ability
void after_fault_check_status(void)
{
    timer1_set_state(DISABLE);
    if (fault_flag>0)
    {
        fault_flag = 1;
        uart_puts("Byla chyba, kontrola jestli je vse OK\n");
        pom = "SW&RD ADC a COMP    ";

    }
    else
    {
        uart_puts("Vse je OK, nyni je povoleno PTT\n");
        pom = "Vse OK           ";
        button_ptt_set_irq(ENABLE);
        actual_state = EVENT0;
    }
}

void processing_adc_data(void)
{
    if (fault_flag == 2)            // if first run after start up device, read ADC value as ADC_SWR
    {
        uart_puts("first start - copy ADC to ADC_SWR\n");
        ADC_SWR = ADC;
    }
    if (((ADC_SWR<UMIN) || (ADC_SWR>UMAX)) && ((fault_flag == 0) || (fault_flag == 1)))
    {
        //uart_puts("ADC hodnota ");
        //uart_puts(buffer4);
        //uart_puts(" je mimo rozsah, generuji fault flag\n");
        //PORTC ^= (1<<5);
        actual_state = FAULT;
        TIFR1 |= 1<<TOV1;
        fault_flag = 1;
        fault_count = 0;
        timer1_set_state(ENABLE);
    }
    else if (fault_flag == 1)
    {
        //uart_puts("hodnota ADC ");
        //uart_puts(buffer4);
        //uart_puts(" je OK, vracim fault_flag = 0\n");
        actual_state = AFTER_FAULT;
        fault_flag = 0;
        TIFR1 |= 1<<TOV1;
        timer1_set_state(ENABLE);
    }
    else if (fault_flag == 0 || fault_flag == 3)
    {
        switch (adc_active_channel) {
        case ADC_CHANNEL_SWR:
            ADC_TEMP_INT = ADC;
            adc_active_channel = ADC_CHANNEL_TEMP_HEATSINK;
            break;
        case ADC_CHANNEL_TEMP_HEATSINK:
            ADC_SWR = ADC;
            adc_active_channel = ADC_CHANNEL_POWER;
            break;
        case ADC_CHANNEL_POWER:
            PORTC ^= (1<<5);
            ADC_TEMP_HEATSINK = ADC;
            adc_active_channel = ADC_CHANNEL_Ucc;
            break;
        case ADC_CHANNEL_Ucc:
            ADC_POWER = ADC;
            adc_active_channel = ADC_CHANNEL_Icc;
            break;
        case ADC_CHANNEL_Icc:
            ADC_Ucc = ADC;
            adc_active_channel = ADC_CHANNEL_TEMP_INT;
            break;
        case ADC_CHANNEL_TEMP_INT:
            ADC_Icc = ADC;
            adc_active_channel = ADC_CHANNEL_SWR;
            break;
        default:
            adc_active_channel = ADC_CHANNEL_SWR;
            break;
        }
    }
    ADMUX = ADMUX & 0xF0 | adc_active_channel;
}
