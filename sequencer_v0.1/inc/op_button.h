/*
 * op_button.h
 *
 * Created: 23.6.2018 16:21:29
 *  Author: fkla
 */ 

#include "types.h"

#ifndef OP_BUTTON_H_
#define OP_BUTTON_H_

// set port where is connected PTT button
#define OPERATE_BUTTON_DDR     DDRD
#define OPERATE_BUTTON_PORT    PORTD
#define OPERATE_BUTTON_PIN     PIND
#define OPERATE_BUTTON_PIN_NUM 3       // pin where is PTT connected

//functions
void operate_button_init(void);
void operate_button_set_irq(state_t state);



#endif /* OP_BUTTON_H_ */