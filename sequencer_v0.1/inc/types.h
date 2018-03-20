#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include <avr/io.h>

typedef enum {
	ENABLE = 1,
	DISABLE = !ENABLE,
} state_t;

typedef enum {
	SUCCESS = 1,
	ERROR = !SUCCESS,
} result_t;

typedef enum{
	EVENT0,
	EVENT1,
	EVENT2,
	FAULT,
	AFTER_FAULT,
} sequencer_t;


#endif // TYPES_H_INCLUDED
