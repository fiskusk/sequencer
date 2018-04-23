#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED


#ifndef F_CPU
# define  F_CPU 16000000UL
#endif

// define NTC
#define R_DIV   10e03
#define R_REF   22e03
#define A1		3.354016e-03
#define B1		2.744032e-04
#define C1		3.666944e-06
#define D1		1.375492e-07

// define limits
#define ADC_REF_VOLTAGE_MAX         1000

#define ADC_UCC_MIN                 80              // 80 -> 5.4885 V
#define ADC_UCC_MAX                 180            // 170 -> 11.663 V, 180 -> 12,31V

#define ADC_ICC_MAX                 819

#define ADC_TEMP_HEATSINK_MAX       670             // Turn fan
#define ADC_TEMP_HEATSINK_ABS_MAX   600             // Overheat, turn all, block, turn fan

#define ADC_TEMP_INT_MAX            1023
#define ADC_TEMP_INT_ABS_MAX        1023

// define timers
#define TSEQ 62411  // Time delay between of two sequence
#define TREL 65535 // Delay between servo1 and servo2. They must switch simultaneously.

extern char *pom;

#endif // SETTINGS_H_INCLUDED
