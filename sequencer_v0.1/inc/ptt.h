#ifndef PTT_H_INCLUDED
#define PTT_H_INCLUDED

#include "settings.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#include "types.h"
#include "main.h"
#include "uart.h"
#include "permitters.h"


void ptt_init(void);

#endif // PTT_H_INCLUDED
