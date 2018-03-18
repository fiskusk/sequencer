/*
 * state.h
 * My own types
 * Created: 16.3.2018 15:18:14
 *  Author: fkla
 */ 


#ifndef STATE_H_INCLUDED
#define STATE_H_INCLUDED

// ========================================================
typedef enum {
	ENABLE = 1,
	DISABLE = !ENABLE,
}state_t;

typedef enum{
	EVENT0,	        
	EVENT1,         
	EVENT2,         
	FAULT,          
	AFTER_FAULT,    
	TEST_PTT        
} sequencer_t;

typedef enum {
    ADC_CHANNEL_SWR = 0,
    ADC_CHANNEL_TEMP_HEATSINK = 1,
    ADC_CHANNEL_POWER = 2,
    ADC_CHANNEL_Ucc = 3,
    ADC_CHANNEL_Icc = 4,
    ADC_CHANNEL_TEMP_INT = 8
}adc_channel_t;
// ========================================================

#endif // STATE_H_INCLUDED 