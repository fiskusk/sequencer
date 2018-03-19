#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED


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


#endif // TYPES_H_INCLUDED
