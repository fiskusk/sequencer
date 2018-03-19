#include "adc.h"

#define UMIN   0
#define UMAX   600

adc_channel_t adc_active_channel = ADC_CHANNEL_SWR;     // default first channel in ADC process

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
    ADMUX = (1<<REFS1) | (1<<REFS0);

    // ADENable, ADStart Conversion, ADInterrupt Enable
    // when set ADATE - ADCH MSB, ADCL LSB
    // ADPrescaler Select - 2,2,4,8,16,32,64,128
    ADCSRA = (1<<ADEN) | (1<<ADSC) | (1<<ADATE) | (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2);

    PRR &= ~(1<<PRADC);
}

void adc_processing_data(void)
{
    if (fault_flag == 2) // if first run after start up device, read ADC value as ADC_SWR
    {
        uart_puts("first start - copy ADC to ADC_SWR\n");
        adc_swr = ADC;
    }
    if (((adc_swr < UMIN) || (adc_swr > UMAX)) && ((fault_flag == 0) || (fault_flag == 1)))
    {
        // uart_puts("ADC hodnota ");
        // uart_puts(buffer4);
        // uart_puts(" je mimo rozsah, generuji fault flag\n");
        // PORTC ^= (1<<5);
        actual_state = FAULT;
        TIFR1       |= 1 << TOV1;
        fault_flag   = 1;
        fault_count  = 0;
        timer1_set_state(ENABLE);
    }
    else if (fault_flag == 1)
    {
        // uart_puts("hodnota ADC ");
        // uart_puts(buffer4);
        // uart_puts(" je OK, vracim fault_flag = 0\n");
        actual_state = AFTER_FAULT;
        fault_flag   = 0;
        TIFR1       |= 1 << TOV1;
        timer1_set_state(ENABLE);
    }
    else if (fault_flag == 0 || fault_flag == 3)
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
            case ADC_CHANNEL_TEMP_INT:
                adc_temp_int       = ADC;
                adc_active_channel = ADC_CHANNEL_SWR;
                break;
        }
    }
    ADMUX = (ADMUX & 0xF0) | adc_active_channel;
} /* processing_adc_data */