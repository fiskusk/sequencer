#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include <avr/io.h>

typedef enum {
    ENABLE  = 1,
    DISABLE = !ENABLE,
} state_t;

typedef enum {
    SUCCESS = 1,
    ERROR   = !SUCCESS,
    BIG_ERROR = 2,
} result_t;

typedef enum {
    UI_INIT,
    UI_RUN,
    UI_ERROR,
} ui_state_t;

#endif // TYPES_H_INCLUDED
