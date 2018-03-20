#ifndef ADC_H_INCLUDED
#define ADC_H_INCLUDED

#include "settings.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#include "types.h"
#include "uart.h"
#include "ptt.h"
#include "switching.h"


typedef enum {
    ADC_CHANNEL_SWR           = 0,
    ADC_CHANNEL_TEMP_HEATSINK = 1,
    ADC_CHANNEL_POWER         = 2,
    ADC_CHANNEL_UCC           = 3,
    ADC_CHANNEL_ICC           = 4,
    ADC_CHANNEL_TEMP_INT      = 8
} adc_channel_t;


extern volatile uint16_t adc_swr;
extern volatile uint16_t adc_ucc;
extern volatile uint16_t adc_icc;
extern volatile uint16_t adc_power;
extern volatile uint16_t adc_temp_int;
extern volatile uint16_t adc_temp_heatsink;


void adc_init(void);
void adc_get_data(void);
result_t adc_check_swr(void);
result_t adc_check_temp(void);


#endif // ADC_H_INCLUDED
