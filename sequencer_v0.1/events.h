/*
 * events.h
 * Functions of each event
 * Created: 16.3.2018 15:18:14
 *  Author: fkla
 */

#ifndef EVENTS_H_INCLUDED
#define EVENTS_H_INCLUDED

#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "main.h"
#include "types.h"
#include "permitters.h"

/* Functions of each events. When PTT button status changed, 
*  call function Event_PTT_button_status_changed(). 
* First
*/

// ========================================================
void event_PTT_pushed_up_on_relay1_on_FAN(void);    // next state - on relay2
void event_PTT_pushed_down_off_Ucc(void);           // next state - turn off bias
void error(void);
void E0_on_off_relay2(void);                        // next state - bias
void E1_on_off_bias(void);
void E2_on_Ucc_off_relay1(void);
void fault_off_all(void);
void after_fault_check_status(void);
void test_state_of_PTT_button(void);
void event_PTT_button_status_changed(void);
void processing_adc_data(void);
// ========================================================

#endif // EVENTS_H_INCLUDED
