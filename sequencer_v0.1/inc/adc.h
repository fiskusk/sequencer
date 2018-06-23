#ifndef ADC_H_INCLUDED
#define ADC_H_INCLUDED

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

#include "types.h"
#include "ptt.h"
#include "switching.h"
#include "ui.h"
#include "settings.h"

#define ADC_REF 2506                    // reference voltage in millivolts

// coeff. for power polynomial function
#define APWR 1.123453822e-5             
#define BPWR 1.060541075e-2
#define CPWR 0.5780925282

// structure for ADC channels
typedef enum {
    ADC_CHANNEL_REFLECTED           = 1,
    ADC_CHANNEL_TEMP_HEATSINK = 0,
    ADC_CHANNEL_POWER         = 2,
    ADC_CHANNEL_UCC           = 7,
    ADC_CHANNEL_ICC           = 6,
} adc_channel_t;

// structure for blocking states
typedef enum {
    BLOCK_ONLY,
    BLOCK_TIMER,
} adc_block_t;

// ADC saved values
extern volatile uint16_t adc_reflected;           // reflected power
extern volatile uint16_t adc_ref_cache;     // cache reflected power
extern volatile uint16_t adc_ucc;           // voltage
extern volatile uint16_t adc_icc;           // current
extern volatile uint16_t adc_power;         // rediated power
extern volatile uint16_t adc_temp_heatsink; // temperature on heatsink
extern volatile uint16_t timer_ovf_count;   // repetion counter of interrupts


//functions
void adc_init(void);
void adc_get_data(void);
int16_t adc_get_temp(void);
char* adc_get_swr(uint16_t pwr, uint16_t ref);

uint16_t adc_get_pwr(void);
uint16_t adc_get_reflected(void);
uint16_t adc_get_icc(void);
uint16_t adc_get_ucc(void);

result_t adc_check_ref(void);
result_t adc_check_temp(void);
result_t adc_check_ucc(void);
result_t adc_check_icc(void);


#endif // ADC_H_INCLUDED
