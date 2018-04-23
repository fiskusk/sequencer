#ifndef ADC_H_INCLUDED
#define ADC_H_INCLUDED

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

#include "types.h"
#include "uart.h"
#include "ptt.h"
#include "switching.h"
#include "ui.h"
#include "settings.h"

#define ADC_REF 2501

typedef enum {
    ADC_CHANNEL_SWR           = 1,
    ADC_CHANNEL_TEMP_HEATSINK = 0,
    ADC_CHANNEL_POWER         = 2,
    ADC_CHANNEL_UCC           = 7,
    ADC_CHANNEL_ICC           = 6,
} adc_channel_t;

typedef enum {
    BLOCK_ONLY,
    BLOCK_TIMER,
} adc_block_t;


extern volatile uint16_t adc_ref;
extern volatile uint16_t adc_ref_cache;
extern volatile uint16_t adc_ucc;
extern volatile uint16_t adc_icc;
extern volatile uint16_t adc_power;
extern volatile uint16_t adc_temp_int;
extern volatile uint16_t adc_temp_heatsink;
extern volatile uint16_t timer_ovf_count; 


void adc_init(void);

void adc_get_data(void);
int16_t adc_get_temp(void);
char* adc_get_swr(uint16_t pwr, uint16_t ref);
uint16_t adc_get_pwr(void);
uint16_t adc_get_ref(void);
uint16_t adc_get_icc(void);
uint16_t adc_get_ucc(void);

result_t adc_check_ref(void);
result_t adc_check_temp(void);
result_t adc_check_ucc(void);
result_t adc_check_icc(void);


#endif // ADC_H_INCLUDED
