/*
 * op_button.c
 *
 * Created: 23.6.2018 16:21:11
 *  Author: fkla
 */ 

#include <avr/interrupt.h>
#include "op_button.h"
#include "switching.h"

void operate_button_init(void)
{
    // INT1 init
    OPERATE_BUTTON_DDR  &= ~(1 << OPERATE_BUTTON_PIN_NUM);    // operate button port as input
    OPERATE_BUTTON_PORT |= 1 << OPERATE_BUTTON_PIN_NUM;     // operate button on port have pull-up
    EICRA    |= 1 << ISC11;             // falling edge generates interrupt
    
}/* operate_button_init */

void operate_button_set_irq(state_t state)
{
    (state == ENABLE) ? (EIMSK |= 1 << INT1) : (EIMSK &= ~(1 << INT1));
    
}/* ptt_set_irq */

ISR(INT1_vect)
{
    if (ui_state != UI_OPERATE)
    {
        ui_state = UI_OPERATE;
        switching_operate_stby_led(ENABLE);     // turn on LED, which indicate operate mod.
        mode = "RX";
        switching_fault_led(DISABLE);
        ptt_set_irq(ENABLE);            // enable PTT interruption
    }
    else
    {
        ptt_set_irq(DISABLE);           // disable interrupt from PTT
        ui_state = UI_STANDBY;
        switching_state = SWITCHING_OFF;
        switching_off_sequence();       // turn off all
        switching_operate_stby_led(DISABLE);     // turn on LED, which indicate operate mod.
        mode = "SB";
        switching_fault_led(DISABLE);
     }
    
}

