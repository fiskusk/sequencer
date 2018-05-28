#ifndef UI_H_INCLUDED
#define UI_H_INCLUDED

#include <avr/interrupt.h>
#include <avr/io.h>

#include "types.h"
#include "switching.h"

typedef enum {
    UI_INIT,
    UI_RUN,
    UI_HI_REF,
    UI_HI_TEMP,
    UI_VOLTAGE_BEYOND_LIM,
    UI_CURRENT_OVERLOAD,
} ui_state_t;

extern volatile ui_state_t ui_state;
extern volatile state_t print_func;


// functions
void ui_init(void);
void ui_handle(void);



#endif // UI_H_INCLUDED 