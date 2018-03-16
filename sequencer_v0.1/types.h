/*
 * state.h
 *
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
	EVENT0,	        // on.off rele2 tak, aby se seplo ve stejny cas jako rele 1
	EVENT1,         // on.off bias
	EVENT2,         // on Ucc nebo off rele1
	FAULT,          // chybovy stav, vse vypne... po startu vykona rychle, v pripade chyby vykonava opakovane, tak aby to zabralo
	AFTER_FAULT,    // zapne ADC, vycte hodnoty, zkontroluje stav, pokud OK nastavi vychozi stav na EVENT0, pøi zmaèknuti PTT se bude dat spustit..
	TEST_PTT        // rozhoduje co je s tlacitkem
} sequencer_t;
// ========================================================

#endif // STATE_H_INCLUDED 