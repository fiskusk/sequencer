#include "adc.h"

#define ADC_SWR_VOLTAGE_MAX   600

#define ADC_TEMP_HEATSINK_MAX 600

#define ADC_TEMP_INT_MAX      600

adc_channel_t adc_active_channel = ADC_CHANNEL_SWR; // default first channel in ADC process

uint16_t adc_swr;
uint16_t adc_ucc;
uint16_t adc_icc;
uint16_t adc_power;
uint16_t adc_temp_int;
uint16_t adc_temp_heatsink;

void adc_init(void)
{
    // setup ADC
    // internally reference
    // 0. external AREF (internal Vref disabled),
    // 1. AVCC with external cap at AREF pin,
    // 2. reserved
    // 3. Internal 1,1V voltage ref. with external cap on AREF pin
    ADMUX = (1 << REFS1) | (1 << REFS0);

    // ADENable, ADStart Conversion, ADInterrupt Enable
    // when set ADATE - ADCH MSB, ADCL LSB
    // ADPrescaler Select - 2,2,4,8,16,32,64,128
    ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADATE) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);

    PRR &= ~(1 << PRADC);

    TIMSK2 |= 1 << TOIE2;
}

void adc_error_timer(state_t state)
{
    if (state == ENABLE)
    {
        TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);
    }
    else
    {
        TCCR2B &= ~((1 << CS22) | (1 << CS21) | (1 << CS20));
    }
}

void adc_get_data(void)
{
    switch (adc_active_channel)
    {
        case ADC_CHANNEL_SWR:
            adc_swr = ADC;
            adc_active_channel = ADC_CHANNEL_TEMP_HEATSINK;
            break;
        case ADC_CHANNEL_TEMP_HEATSINK:
            adc_temp_heatsink  = ADC;
            adc_active_channel = ADC_CHANNEL_POWER;
            break;
        case ADC_CHANNEL_POWER:
            PORTC    ^= (1 << 5);
            adc_power = ADC;
            adc_active_channel = ADC_CHANNEL_UCC;
            break;
        case ADC_CHANNEL_UCC:
            adc_ucc = ADC;
            adc_active_channel = ADC_CHANNEL_ICC;
            break;
        case ADC_CHANNEL_ICC:
            adc_icc = ADC;
            adc_active_channel = ADC_CHANNEL_TEMP_INT;
            break;
        default:
            adc_temp_int       = ADC;
            adc_active_channel = ADC_CHANNEL_SWR;
            break;
    }
    ADMUX = (ADMUX & 0xF0) | adc_active_channel;
}

result_t adc_check_swr(void)
{
    if (adc_swr > ADC_SWR_VOLTAGE_MAX)
        return ERROR;

    return SUCCESS;
}

result_t adc_check_temp(void)
{
    if (adc_temp_heatsink > ADC_TEMP_HEATSINK_MAX)
        return ERROR;
    else if (adc_temp_int > ADC_TEMP_INT_MAX)
        return ERROR;

    return SUCCESS;
}

void adc_evaluation(void)
{
    if (adc_check_swr() == ERROR)
    {
        switching_state = SWITCHING_OFF;
        ptt_set_irq(DISABLE);
        adc_error_timer(ENABLE);
        switching_off_sequence();
    }
    if (adc_check_temp() == ERROR)
        SWITCHING_FAN_ON;
}

ISR(ADC_vect)
{
    adc_get_data();
    adc_evaluation();
}

ISR(TIMER2_OVF_vect)
{
    static uint16_t timer_ovf_count = 0;
    if (++timer_ovf_count > 1225) // 20s 1225
    {
        timer_ovf_count = 0;
        adc_error_timer(DISABLE);
        ptt_set_irq(ENABLE);
    }
}