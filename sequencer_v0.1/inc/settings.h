#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED


#ifndef F_CPU
# define  F_CPU 16000000UL
#endif

// define NTC
#define R_DIV   10e03           // dividers upper resistance
#define R_REF   22e03           // NTC reference resistance
#define A1		3.354016e-03    // datasheet coefficients
#define B1		2.744032e-04
#define C1		3.666944e-06
#define D1		1.375492e-07

// define ADC limits
#define ADC_REFLECTED_VOLTAGE_MAX   800    // maximum reflected power 800 is aprox. 100W reflected power
#define ADC_POWER_VOLTAGE_MAX       973     // maximum output power 973 is aprox. 1300W output power

#define ADC_UCC_MIN                 600     // minumum supply voltage
#define ADC_UCC_MAX                 744     // maximum supply voltage

#define ADC_ICC_MAX                 819     // maximum current

#define ADC_TEMP_HEATSINK_MAX       605     // where turn fan 605 -> 35 °C
#define ADC_TEMP_HEATSINK_ABS_MAX   220     // Overheat, turn off all, block and turn fan  250 -> 78°C

// define timers
#define TSEQ 62411  // Time delay between of two sequence
#define TREL 65535  // Delay between servo1 and servo2. They must switch simultaneously.

extern char *mode;

#endif // SETTINGS_H_INCLUDED
