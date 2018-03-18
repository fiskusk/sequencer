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
// ========================================================

#endif // STATE_H_INCLUDED 