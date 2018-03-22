#include "adc.h"

#define ADC_SWR_VOLTAGE_MAX   800

#define ADC_TEMP_HEATSINK_MAX 600

#define ADC_TEMP_INT_MAX      1023

adc_channel_t adc_active_channel = ADC_CHANNEL_SWR; // default first channel in ADC process

volatile uint16_t adc_swr;
volatile uint16_t adc_ucc;
volatile uint16_t adc_icc;
volatile uint16_t adc_power;
volatile uint16_t adc_temp_int;
volatile uint16_t adc_temp_heatsink;

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
    ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADIE) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); // | (1 << ADATE)

    PRR &= ~(1 << PRADC);

    TIMSK2 |= 1 << TOIE2;

    DDRC  &= 0b11100000; // ADC0-4 as output
    PORTC &= 0b11100000; // ADC0-4 pull-up turn off
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
    static uint8_t count = 0;
    static volatile uint16_t sum  = 0;
    switch (adc_active_channel)
    {
        case ADC_CHANNEL_SWR:
            ++count;
            if (count > 2 && count < 6)
                sum += ADC;
            else if (count >= 6)
            {
                adc_swr = sum / 3;
                count        = 0;
                sum = 0;
                adc_active_channel = ADC_CHANNEL_TEMP_HEATSINK;
            }
            break;
        case ADC_CHANNEL_TEMP_HEATSINK:
            ++count;
            if (count > 2 && count < 6)
                sum += ADC;
            else if (count >= 6)
            {
                adc_temp_heatsink = sum / 3;
                count        = 0;
                sum = 0;
                adc_active_channel = ADC_CHANNEL_POWER;
            }
            break;
        case ADC_CHANNEL_POWER:
            ++count;
            if (count > 2 && count < 6)
                sum += ADC;
            else if (count >= 6)
            {
                adc_power = sum / 3;
                count        = 0;
                sum = 0;
                adc_active_channel = ADC_CHANNEL_UCC;
            }
            break;
        case ADC_CHANNEL_UCC:
            ++count;
            if (count > 2 && count < 6)
                sum += ADC;
            else if (count >= 6)
            {
                adc_ucc = sum / 3;
                count        = 0;
                sum          = 0;
                adc_active_channel = ADC_CHANNEL_ICC;
            }
            break;
        case ADC_CHANNEL_ICC:
            ++count;
            if (count > 2 && count < 8)
                sum += ADC;
            else if (count >= 6)
            {
                adc_icc = sum / 3;
                count   = 0;
                sum     = 0;
                adc_active_channel = ADC_CHANNEL_TEMP_INT;
            }
            break;
        default:
            ++count;
            if (count > 2 && count < 8)
                sum += ADC;
            else if (count >= 8)
            {
                adc_temp_int = sum / 3;
                count        = 0;
                sum          = 0;
                adc_active_channel = ADC_CHANNEL_SWR;
            }
            break;
    }
    ADMUX = (ADMUX & 0xF0) | adc_active_channel;
} /* adc_get_data */

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
        pom = "HI SWR";
        adc_error_timer(ENABLE);
        switching_off_sequence();
    }
    if (adc_check_temp() == ERROR ||  switching_state == SWITCHING_ON)
        SWITCHING_FAN_ON;
    else
        SWITCHING_FAN_OFF;
}

ISR(ADC_vect)
{
    cli();
    adc_get_data();
    adc_evaluation();
    sei();
    ADCSRA |= 1 << ADSC;
}

ISR(TIMER2_OVF_vect)
{
    static uint16_t timer_ovf_count = 0;
    if (++timer_ovf_count > 1225) // 20s 1225 3s 183
    {
        timer_ovf_count = 0;
        pom = "SWR OK";
        adc_error_timer(DISABLE);
        ptt_set_irq(ENABLE);
    }
}
