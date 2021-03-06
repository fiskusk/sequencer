#include "adc.h"
#include <avr/pgmspace.h>
#include "stdio.h"
#include <util/delay.h>
#include "op_button.h"

adc_channel_t adc_active_channel = ADC_CHANNEL_REFLECTED; // default first channel in ADC process
adc_block_t adc_block;

volatile uint16_t adc_reflected;
volatile uint16_t adc_ucc;
volatile uint16_t adc_icc;
volatile uint16_t adc_power;
volatile uint16_t adc_temp_heatsink;
volatile uint16_t timer_ovf_count = 0; 

extern volatile ui_state_t ui_state;

/************************************************************************/
/*  initialization function for ADC                                     */
/************************************************************************/
void adc_init(void)
{
    // setup ADC
    // internally reference
    // 0. external AREF (internal Vref disabled),
    // 1. AVCC with external cap at AREF pin,
    // 2. reserved
    // 3. Internal 1,1V voltage ref. with external cap on AREF pin
    
    // active is AREF, Internal Vref turned OFF now!!!!!!
    
    //ADMUX = (1 << REFS1) | (1 << REFS0); 

    // ADENable, ADStart Conversion, ADInterrupt Enable
    // when set ADATE - ADCH MSB, ADCL LSB
    // ADPrescaler Select - 2,2,4,8,16,32,64,128 (now div 32)
    ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADIE) | (1<< ADPS2) | (1 << ADPS0); // | (1 << ADATE)

    PRR &= ~(1 << PRADC);           // power reduction register ADC, deactivate-> turn on ADC


    DDRC  &= 0b11100000;            // ADC0-4 as input
    PORTC &= 0b11100000;            // ADC0-4 pull-up turn off
} /* adc_init */

/************************************************************************/
/*  This block function can do tho types of block PA                    */
/*  when input argument is BLOCK_TIMER, set block for certain time      */
/*  when input argument is BLOCK_ONLY, set block without timer          */
/************************************************************************/
void adc_block_pa(void)
{
    switching_state = SWITCHING_OFF;
    ptt_set_irq(DISABLE);           // disable interrupt from PTT
    switching_off_sequence();       // turn off all
    mode = "HI";                     // status window message set to HI means hight
    switching_fault_led(ENABLE);
    switching_operate_stby_led(DISABLE);
} /* adc_block_pa */

/************************************************************************/
/*  These function switches inc. the measuring inputs of the ADC        */
/*  first two sample are throw away                                     */   
/*  next each value are sampled three times and averaged                */                                                    
/************************************************************************/
void adc_get_data(void)
{
    //switch-case structure for relevant states
    static uint8_t count = 0;
    static uint16_t count2 = 0;
    volatile static uint32_t sum = 0;
    switch (adc_active_channel) 
    {
        case ADC_CHANNEL_REFLECTED:               // channel for measuring reflected power
            adc_reflected =  ADC;
            count++;
            if (count == 1)
                 adc_active_channel = ADC_CHANNEL_POWER; //next state
            else if (count == 2)
                adc_active_channel = ADC_CHANNEL_TEMP_HEATSINK;
            else if (count == 3)
                adc_active_channel = ADC_CHANNEL_UCC;
            else
            {
                count = 0;
                adc_active_channel = ADC_CHANNEL_ICC;
            }
            ADMUX = (ADMUX & 0xF0) | adc_active_channel; // switching adc channel 
            if (adc_check_ref() != SUCCESS && ui_state != UI_INIT)
            {
                adc_block_pa();                     // block
                ui_state = UI_HI_REF;               // ui state for print on screen
            }
            break;
            
        case ADC_CHANNEL_POWER:             // channel for measuring radiated power
            adc_power = ADC;
            adc_active_channel = ADC_CHANNEL_REFLECTED;
            ADMUX = (ADMUX & 0xF0) | adc_active_channel; // switching adc channel
            if (adc_check_pwr() != SUCCESS && ui_state != UI_INIT)
            {
                adc_block_pa();                     // block
                ui_state = UI_HI_PWR;               // ui state for print on screen
            }
            break;
        case ADC_CHANNEL_TEMP_HEATSINK:     // channel for measuring temperature
            adc_temp_heatsink = ADC;
            adc_active_channel = ADC_CHANNEL_REFLECTED;
            ADMUX = (ADMUX & 0xF0) | adc_active_channel; // switching adc channel
            if (adc_check_temp() == BIG_ERROR && ui_state != UI_INIT)
            {
                ui_state = UI_HI_TEMP;
                SWITCHING_FAN_ON;
                adc_block_pa();
            }
            else if ((adc_check_temp() == ERROR || switching_state == SWITCHING_ON) && ui_state != UI_INIT)
                SWITCHING_FAN_ON;
            else if (ui_state != UI_INIT)
                SWITCHING_FAN_OFF;
            break;
        case ADC_CHANNEL_UCC:               // channel for measuring voltage
            adc_ucc = ADC;
            adc_active_channel = ADC_CHANNEL_REFLECTED;
            ADMUX = (ADMUX & 0xF0) | adc_active_channel; // switching adc channel
            if (adc_check_ucc() != SUCCESS)
            {
                ui_state = UI_VOLTAGE_BEYOND_LIM;
                adc_block_pa();
            }
            break;
        default:                            // channel for measurring current
            ++count2;
            if (count2 < 513)
            {
                adc_active_channel = ADC_CHANNEL_REFLECTED;
                ADMUX = (ADMUX & 0xF0) | adc_active_channel; // switching adc channel
                sum += ADC;
            }
            else if (count2 >= 513)
            {
                adc_icc = (sum >> 9);
                count2   = 0;
                sum     = 0;
                adc_active_channel = ADC_CHANNEL_REFLECTED;
                ADMUX = (ADMUX & 0xF0) | adc_active_channel; // switching adc channel
                if (adc_check_icc() != SUCCESS)
                {
                    ui_state = UI_CURRENT_OVERLOAD;
                    adc_block_pa();
                }
            }
            break;
    }
    
} /* adc_get_data */

/************************************************************************/
/*  Function to tranfer measured voltage on temperature input           */
/*  to degree Celsius                                                   */                    
/************************************************************************/
int16_t adc_get_temp(void)
{
    float volt, temp_log, adc_ref = ADC_REF/1000.0;
    float ntc_resistance, temp;
    
    // transfer ADC register value to volts    
    volt = (adc_ref / 1024) * ((float)adc_temp_heatsink);
    
    // transfer measure voltage to resistance of NTC termistor
    // R_DIV is resistor in divider with termistor
    ntc_resistance = (-(volt * R_DIV) / adc_ref) / ((volt / adc_ref) - 1);
    
    // natural logarithm
    temp_log = log(ntc_resistance/R_REF);
    
    // transfer to resulting final temperature in degree of Celsius
    temp = 1.0 / ( A1 + B1*temp_log + C1*temp_log*temp_log + D1*temp_log*temp_log*temp_log) - 273.15;
    
    //temp = (-68.504) * ((adc_temp_heatsink * ADC_REF) / 1024.0) + 139.33;
    //temp = ((adc_temp_heatsink * 2.502) / 1024.0);
    
    return temp;
} /* adc_get_temp */

/************************************************************************/
/* This function calculate vswr from calc radiated power and ref. power */
/************************************************************************/
char* adc_get_swr(uint16_t pwr, uint16_t ref)
{
    float return_loss;
    uint32_t swr;
    
    uint16_t int_part;
    uint16_t dec_part;
    static char buffer[20];
    
    // calculation of return loss
    return_loss = sqrt((float) ref/pwr);
    
    // transfer to vswr value
    swr = ((1 + return_loss) * 1000 ) / (1 - return_loss);
    
    // rounding
    if ( swr <= 99999 && (swr - (swr/10) * 10) >= 5)
        swr += 10;
    else if ( swr > 99999 && swr <= 999999 && (swr - (swr/100) * 100) >= 50)
        swr += 10;
   
    // get integer and decimal part
    int_part = swr/1000;
    dec_part = (swr%1000);
    
    // correct form message to print of LCD
    if (int_part == 0)
        sprintf_P(buffer, PSTR(" -.-- "));
    else if (pwr < ref)
        sprintf_P(buffer, PSTR(" ERR "));
    else if (int_part > 999)
        sprintf_P(buffer, PSTR(" %4d "),int_part);
    else if (int_part > 99)
        sprintf_P(buffer, PSTR("%3d.%1d "),int_part, dec_part/100);
    else
        sprintf_P(buffer, PSTR("%2d.%02d "),int_part, dec_part/10);
    
    return buffer;
    
} /* adc_get_swr */

/************************************************************************/
/* Function to get power in watts from ADC value                        */
/************************************************************************/
uint16_t adc_get_pwr(void)
{
    float pwr;
    //float volts = (float) adc_power * (ADC_REF) / 1024.0; //(double) adc_power * ADC_REF / 1024UL;
    //volts ;
    
    // transfer measured voltage to power in watts using polynomial function
    //pwr = APWR*volts*volts + BPWR*volts - CPWR;
    
    if(adc_power < 5)
        pwr = 0;
    else
        pwr = 9.3725e-4*adc_power*adc_power + 3.865e-1*adc_power + 37.22;
    
    //pwr = adc_power;
    
    return pwr;
} /* adc_get_pwr */

/************************************************************************/
/* Function to get reflected power in watts from ADC value              */
/************************************************************************/
uint16_t adc_get_reflected(void)
{
    float reflected;
    // transfer measrured adc_reflected power to reflection in watts
    /*if (adc_reflected > 0)
        reflected =  4.537319514e-05*adc_reflected*adc_reflected + 2.981010572e-02*adc_reflected + 1.70300274;
    else
        reflected = 0;*/
    //ref = -5.518777275*volts*volts*volts + 26.63047832*volts*volts + 0.1985508811*volts + 0.7206280062;
   
    if (adc_reflected == 0)
    {
        reflected = 0;
    }
    else if (adc_reflected < 20)
        reflected = -1.885116627e-3*adc_reflected*adc_reflected + 1.387656694e-1*adc_reflected + 2.011027296;
    else
        reflected = 7.930636126e-5*adc_reflected*adc_reflected + 5.22582337e-2*adc_reflected + 3.184718159;
    //reflected = adc_reflected;
    return reflected;
}/* adc_get_ref */

/************************************************************************/
/*  Get current in Ampers from ADC value                                */
/************************************************************************/
uint16_t adc_get_icc(void)
{
    uint16_t icc;
    
    // linear regresion function
    icc = (((uint32_t) adc_icc * ADC_REF) / 1024) * 19.476 + 0.031;
    
    return icc;
}/* adc_get_icc */

/************************************************************************/
/*  Get voltage in Volts from ADC value                                 */
/************************************************************************/
uint16_t adc_get_ucc(void)
{
    uint16_t ucc;
    
    // linear regresion function
    ucc = ( ((uint32_t) adc_ucc * ADC_REF) / 1024) * 28; // * 28.08988764 or  27.92008197
    
    return ucc;
} /* adc_get_ucc */

/************************************************************************/
/*  This function evaluates if reflected power exceed limit             */
/************************************************************************/
result_t adc_check_ref(void)
{
    if (adc_reflected > ADC_REFLECTED_VOLTAGE_MAX)
        return ERROR;

    return SUCCESS;
}/* adc_check_ref */

result_t adc_check_pwr(void)
{
    if (adc_power > ADC_POWER_VOLTAGE_MAX)
        return ERROR;

    return SUCCESS;
}/* adc_check_ref */
    
/************************************************************************/
/*  This function evaluates if heatsink temperature exeed limits        */
/*  Evaluates three states:                                             */
/*  ERROR is set, if temperature exceed limit for turn on FAN           */
/*  BIG_ERROR is set, when temp on heatsing exceed absulute maximum temp*/
/*  SUCCESS is set, when temperature is in limits                       */
/************************************************************************/
result_t adc_check_temp(void)
{
    if (adc_temp_heatsink < ADC_TEMP_HEATSINK_MAX && adc_temp_heatsink > ADC_TEMP_HEATSINK_ABS_MAX)
        return ERROR;
    else if (adc_temp_heatsink <= ADC_TEMP_HEATSINK_ABS_MAX)
        return BIG_ERROR;
    
    return SUCCESS;
}/* adc_check_temp */

/************************************************************************/
/*  Function for evaluates input power supply voltage                   */
/************************************************************************/
result_t adc_check_ucc(void)
{
    if (adc_ucc < ADC_UCC_MIN)
        return ERROR;
    else if (adc_ucc > ADC_UCC_MAX)
        return ERROR;
    
    return SUCCESS;
}

/************************************************************************/
/*  Function for evaluates output power supply current                  */
/************************************************************************/
result_t adc_check_icc(void)
{
    if (adc_icc > ADC_ICC_MAX)
        return ERROR;
    
    return SUCCESS;
}

/************************************************************************/
/*  ADC routine, when the transfer is complete                          */
/************************************************************************/
ISR(ADC_vect)
{
    cli();
    adc_get_data();             // func for get data
    //_delay_us(5);
    ADCSRA |= 1 << ADSC;        // run ad conversion!
    sei();                      // enable all interruptions
}
