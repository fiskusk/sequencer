#include "switching.h"
#include "settings.h"

sequencer_t machine_state;
switching_t switching_state;

/************************************************************************/
/*  Initialization function for switching.                              */
/*  Set port as output and pull off to logical zero / non-active state  */
/************************************************************************/
void switching_init(void)
{
    // setup ports as outputs
    SWITCHING_STATUS_LED_DDR    |= 1 << SWITCHING_STATUS_LED_PIN_NUM;
    SWITCHING_RELAY1_DDR        |= 1 << SWITCHING_RELAY1_PIN_NUM;
    SWITCHING_RELAY2_DDR        |= 1 << SWITCHING_RELAY2_PIN_NUM;
    SWITCHING_BIAS_DDR          |= 1 << SWITCHING_BIAS_PIN_NUM;
    SWITCHING_UCC_DDR           |= 1 << SWITCHING_UCC_PIN_NUM;
    SWITCHING_FAN_DDR           |= 1 << SWITCHING_FAN_PIN_NUM;
    
    // predefine outputs as non-active
    SWITCHING_STATUS_LED_PORT    &= ~(1 << SWITCHING_STATUS_LED_PIN_NUM);
    SWITCHING_RELAY1_PORT       &= ~(1 << SWITCHING_RELAY1_PIN_NUM);
    SWITCHING_RELAY2_PORT       &= ~(1 << SWITCHING_RELAY2_PIN_NUM);
    SWITCHING_BIAS_PORT         &= ~(1 << SWITCHING_BIAS_PIN_NUM);
    SWITCHING_UCC_PORT          &= ~(1 << SWITCHING_UCC_PIN_NUM);
    SWITCHING_FAN_PORT          &= ~(1 << SWITCHING_FAN_PIN_NUM);

    TIMSK1 |= 1 << TOIE1;                   // enable TC1

    machine_state   = RELAY1_AND_FAN;       // first state when PTT generate interrupt
    switching_state = SWITCHING_OFF;        // default state is switch off
    
}/* switching_init */

void switching(void)
{
    switching_timer(DISABLE);           // disable timer1
    if (button_ptt_is_pressed() && adc_check_ref() == SUCCESS
    && adc_check_temp() != BIG_ERROR && adc_check_ucc() == SUCCESS
    && adc_check_icc() == SUCCESS)
    {
        switching_state = SWITCHING_ON; // state of switching is ON
        pom = "TX";                     // print on display status its TX
        switching_on_sequence();
    }
    else
    {
        switching_state = SWITCHING_OFF;// state of switching is OFF
        pom = "RX";                     // print on display status its TX
        switching_off_sequence();
    }
}

/************************************************************************/
/*  These function turn on or turn of SWITCHING TC1                     */
/*   description of interrupt behavior at ISR routine                    */
/************************************************************************/
void switching_timer(state_t state) // switch, which turn on timer1, or turn off
{
    (state == ENABLE) ? (TCCR1B |= (1 << CS12)) : (TCCR1B &= ~(1 << CS12)); // div 256
    
}/* switching_timer */

/************************************************************************/
/*  Remote status LED                                                   */
/************************************************************************/
void switching_status_led(state_t state)
{
    (state == ENABLE) ? SWITCHING_STATUS_LED_ON : SWITCHING_STATUS_LED_OFF;
}/* switching_status_led */

/************************************************************************/
/*  Remote relay 1                                                      */
/************************************************************************/
void switching_relay1(state_t state)
{
    //uart_puts("rele 1\n");
    if (adc_check_ref() == SUCCESS)
        (state == ENABLE) ? SWITCHING_RELAY1_ON : SWITCHING_RELAY1_OFF;
    else
        SWITCHING_RELAY1_OFF;
        
}/* switching_relay1 */

/************************************************************************/
/*  Remote relay 2                                                      */
/************************************************************************/
void switching_relay2(state_t state)
{
    //uart_puts("rele 2\n");
    if (adc_check_ref() == SUCCESS)
        (state == ENABLE) ? (SWITCHING_RELAY2_ON) : (SWITCHING_RELAY2_OFF);
    else
        SWITCHING_RELAY2_OFF;
}/* switching_relay2 */

/************************************************************************/
/*  Remote bias                                                         */
/************************************************************************/
void switching_bias(state_t state)
{
    //uart_puts("bias\n");
    if (adc_check_ref() ==  SUCCESS)
        (state == ENABLE) ? (SWITCHING_BIAS_ON) : (SWITCHING_BIAS_OFF);
    else
        SWITCHING_BIAS_OFF;
        
}/* switching_bias */

/************************************************************************/
/*  Remote power supply                                                 */
/************************************************************************/
void switching_ucc(state_t state)
{
    if (adc_check_ref() == SUCCESS)
        (state == ENABLE) ? (SWITCHING_UCC_ON) : (SWITCHING_UCC_OFF);
    else
        SWITCHING_UCC_OFF;
        
}/* switching_ucc */

/************************************************************************/
/*  Remote fan                                                          */
/************************************************************************/
void switching_fan(state_t state)
{
    //uart_puts("fan\n");
    if (adc_check_temp() ==  SUCCESS)
        (state == ENABLE) ? (SWITCHING_FAN_ON) : (SWITCHING_FAN_OFF);
    else
        SWITCHING_FAN_ON;
  
}/* switching_fan */

/************************************************************************/
/*  Function to turn on PA                                              */
/************************************************************************/
void switching_on_sequence(void)
{
    switch (machine_state)
    {
        case RELAY1_AND_FAN:
            machine_state = RELAY2;     // next state 
            TCNT1         = TREL;       
            switching_relay1(ENABLE);
            switching_fan(ENABLE);
            switching_timer(ENABLE);
            break;
        case RELAY2:
            TCNT1         = TSEQ;
            machine_state = UCC;
            switching_relay2(ENABLE);
            switching_timer(ENABLE);
            break;
        case UCC:
            TCNT1         = TSEQ;
            machine_state = BIAS;
            switching_ucc(ENABLE);
            switching_timer(ENABLE);
            break;
        case BIAS:
            switching_bias(ENABLE);
            break;
    }
    
}/* switching_on_sequence */

/************************************************************************/
/*  Function to rurn on PA                                              */
/************************************************************************/
void switching_off_sequence(void)
{
    switch (machine_state)
    {
        case BIAS:
            TCNT1         = TSEQ;
            machine_state = UCC;        // next state
            switching_bias(DISABLE);
            switching_timer(ENABLE);
            break;
        case UCC:
            TCNT1         = TSEQ;
            machine_state = RELAY2;
            switching_ucc(DISABLE);
            switching_timer(ENABLE);
            break;
        case RELAY2:
            TCNT1         = TREL;
            machine_state = RELAY1_AND_FAN;
            switching_relay2(DISABLE);
            switching_timer(ENABLE);
            break;
        case RELAY1_AND_FAN:
            switching_relay1(DISABLE);
            switching_fan(DISABLE);
            break;
    }
    
}/* switching_off_sequence */

/************************************************************************/
/*  TIMER1 routine, where it is determined whether the button is pressed*/
/************************************************************************/
ISR(TIMER1_OVF_vect)
{
    switching();
    //switching_timer(DISABLE);           // disable timer1
    //if (button_ptt_is_pressed() && adc_check_ref() == SUCCESS       
        //&& adc_check_temp() != BIG_ERROR && adc_check_ucc() == SUCCESS
        //&& adc_check_icc() == SUCCESS)
    //{
        //switching_state = SWITCHING_ON; // state of switching is ON
        //pom = "TX";                     // print on display status its TX
        //switching_on_sequence();
    //}
    //else
    //{
        //switching_state = SWITCHING_OFF;// state of switching is OFF
        //pom = "RX";                     // print on display status its TX
        //switching_off_sequence();
    //}
}
