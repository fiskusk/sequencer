#include "switching.h"
#include "settings.h"

sequencer_t machine_state;
switching_t switching_state;

void switching_init(void)
{
    SWITCHING_STATUS_LED_DDR    |= 1 << SWITCHING_STATUS_LED_PIN_NUM;
    SWITCHING_RELAY1_DDR        |= 1 << SWITCHING_RELAY1_PIN_NUM;
    SWITCHING_RELAY2_DDR        |= 1 << SWITCHING_RELAY2_PIN_NUM;
    SWITCHING_BIAS_DDR          |= 1 << SWITCHING_BIAS_PIN_NUM;
    SWITCHING_UCC_DDR           |= 1 << SWITCHING_UCC_PIN_NUM;
    SWITCHING_FAN_DDR           |= 1 << SWITCHING_FAN_PIN_NUM;

    SWITCHING_STATUS_LED_PORT    &= ~(1 << SWITCHING_STATUS_LED_PIN_NUM);
    SWITCHING_RELAY1_PORT       &= ~(1 << SWITCHING_RELAY1_PIN_NUM);
    SWITCHING_RELAY2_PORT       &= ~(1 << SWITCHING_RELAY2_PIN_NUM);
    SWITCHING_BIAS_PORT         &= ~(1 << SWITCHING_BIAS_PIN_NUM);
    SWITCHING_UCC_PORT          &= ~(1 << SWITCHING_UCC_PIN_NUM);
    SWITCHING_FAN_PORT          &= ~(1 << SWITCHING_FAN_PIN_NUM);

    TIMSK1 |= 1 << TOIE1;

    machine_state   = RELAY1_AND_FAN;
    switching_state = SWITCHING_OFF;
}

void switching_timer(state_t state) // switch, which turn on (1) timer1, or turn off (0)
{
    (state == ENABLE) ? (TCCR1B |= (1 << CS12)) : (TCCR1B &= ~(1 << CS12)); // div 256
}

void switching_status_led(state_t state)
{
    (state == ENABLE) ? SWITCHING_STATUS_LED_ON : SWITCHING_STATUS_LED_OFF;
}

void switching_relay1(state_t state)
{
    //uart_puts("rele 1\n");
    if (adc_check_swr() == SUCCESS)
        (state == ENABLE) ? SWITCHING_RELAY1_ON : SWITCHING_RELAY1_OFF;
    else
        SWITCHING_RELAY1_OFF;
}

void switching_relay2(state_t state)
{
    //uart_puts("rele 2\n");
    if (adc_check_swr() == SUCCESS)
        (state == ENABLE) ? (SWITCHING_RELAY2_ON) : (SWITCHING_RELAY2_OFF);
    else
        SWITCHING_RELAY2_OFF;
}

void switching_bias(state_t state)
{
    //uart_puts("bias\n");
    if (adc_check_swr() ==  SUCCESS)
        (state == ENABLE) ? (SWITCHING_BIAS_ON) : (SWITCHING_BIAS_OFF);
    else
        SWITCHING_BIAS_OFF;
}

void switching_ucc(state_t state)
{
    if (adc_check_swr() == SUCCESS)
        (state == ENABLE) ? (SWITCHING_UCC_ON) : (SWITCHING_UCC_OFF);
    else
        SWITCHING_UCC_OFF;
}

void switching_fan(state_t state)
{
    //uart_puts("fan\n");
    if (adc_check_temp() ==  SUCCESS)
        (state == ENABLE) ? (SWITCHING_FAN_ON) : (SWITCHING_FAN_OFF);
    else
        SWITCHING_FAN_ON;
    
}

void switching_on_sequence(void)
{
    switch (machine_state)
    {
        case RELAY1_AND_FAN:
            machine_state = RELAY2;
            TCNT1         = TREL;
            switching_relay1(ENABLE);
            switching_fan(ENABLE);
            switching_timer(ENABLE);
            break;
        case RELAY2:
            TCNT1         = TSEQ;
            machine_state = BIAS;
            switching_relay2(ENABLE);
            switching_timer(ENABLE);
            break;
        case BIAS:
            TCNT1         = TSEQ;
            machine_state = UCC;
            switching_bias(ENABLE);
            switching_timer(ENABLE);
            break;
        case UCC:
            switching_ucc(ENABLE);
            break;
    }
}

void switching_off_sequence(void)
{
    switch (machine_state)
    {
        case UCC:
            TCNT1         = TSEQ;
            machine_state = BIAS;
            switching_ucc(DISABLE);
            switching_timer(ENABLE);
            break;
        case BIAS:
            TCNT1         = TSEQ;
            machine_state = RELAY2;
            switching_bias(DISABLE);
            switching_timer(ENABLE);
            break;
        case RELAY2:
            TCNT1         = TREL;
            machine_state = RELAY1_AND_FAN;
            switching_relay2(DISABLE);
            switching_timer(ENABLE);
            break;
        case RELAY1_AND_FAN:
            switching_relay1(DISABLE);
            switching_fan(DISABLE);
            break;
    }
}

ISR(TIMER1_OVF_vect)
{
    switching_timer(DISABLE);
    if (button_ptt_is_pressed())
    {
        switching_state = SWITCHING_ON;
        pom = "TX";
        switching_on_sequence();
    }
    else
    {
        switching_state = SWITCHING_OFF;
        pom = "RX";
        switching_off_sequence();
    }
}
