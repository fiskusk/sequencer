#include "ptt.h"

#define PTT_DDR DDRD
#define PTT_PORT PORTD
#define PTT_PIN 2


void ptt_init(void)
{
    // INT0 init
    PTT_DDR &= ~(1<<PTT_PIN);
    PTT_PORT |= 1<<PTT_PIN;

    // timer 2 init

}
