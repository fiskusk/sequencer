/*
 * events.h
 *
 * Created: 16.3.2018 15:18:14
 *  Author: fkla
 */

#ifndef EVENTS_H_INCLUDED
#define EVENTS_H_INCLUDED

#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "main.h"
#include "state.h"
#include "permitters.h"


// ========================================================
extern void Event_PTT_pushed_up_oN_relay1_oN_FAN(void);
extern void Event_PTT_pushed_down_oFF_Ucc(void);
extern void error(void);
extern void E0_oNoFF_relay2(void);
extern void E1_oNoFF_bias(void);
extern void E2_oN_Ucc_oFF_relay1(void);
extern void Fault_oFF_all(void);
extern void After_Fault_check_status(void);
extern void Test_state_of_PTT_button(void);
extern void Event_PTT_button_status_changed(void);
// ========================================================

#endif // EVENTS_H_INCLUDED