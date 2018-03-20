#include "switching.h"

switching_t switching_state;

switching_init();

ISR(TIMER1_OVF_vect)
{
    if (switching_state == SWITCHING_ON)
    {
        switch(machine_state) // switch jenom na zapnutí,... nakonec celej ten switch jedna funkce switching_on_sequenc
        {
            case EVENT0:
            E0_on_off_relay2();
            break;
            case EVENT1:
            E1_on_off_bias();
            break;
            case EVENT2:
            E2_on_ucc_off_relay1();
            break;
            case FAULT:
            fault_off_all();
            break;
            default:
            after_fault_check_status();
            break;
            //koncit to bude vypnutim timeru, chybovy stavy vyhod dopièe
        }
    }
    else
    {
        switch(machine_state) // switch jenom na zapnutí,... nakonec celej ten switch jedna funkce switching_on_sequenc
        {
            case EVENT0:
            E0_on_off_relay2();
            break;
            case EVENT1:
            E1_on_off_bias();
            break;
            case EVENT2:
            E2_on_ucc_off_relay1();
            break;
            case FAULT:
            fault_off_all();
            break;
            default:
            after_fault_check_status();
            break;
        }
    }
    
    
}