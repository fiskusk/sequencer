#ifndef EVENTS_H_INCLUDED
#define EVENTS_H_INCLUDED

#include "settings.h"
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
void E0_on_off_relay2(void);                        // next state - bias
void E1_on_off_bias(void);
void E2_on_ucc_off_relay1(void);
void fault_off_all(void);
void after_fault_check_status(void);
// ========================================================

#endif // EVENTS_H_INCLUDED
