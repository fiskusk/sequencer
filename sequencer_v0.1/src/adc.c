#include "adc.h"
#include <avr/pgmspace.h>
#include "stdio.h"

adc_channel_t adc_active_channel = ADC_CHANNEL_SWR; // default first channel in ADC process
adc_block_t adc_block;

volatile uint16_t adc_ref;
volatile uint16_t adc_ref_cache;
volatile uint16_t adc_ucc;
volatile uint16_t adc_icc;
volatile uint16_t adc_power;
volatile uint16_t adc_temp_int;
volatile uint16_t adc_temp_heatsink;
volatile uint16_t timer_ovf_count = 0; 

extern volatile ui_state_t ui_state;

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
    // ADPrescaler Select - 2,2,4,8,16,32,64,128
    ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADIE) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); // | (1 << ADATE)

    PRR &= ~(1 << PRADC);

    TIMSK2 |= 1 << TOIE2;

    DDRC  &= 0b11100000; // ADC0-4 as output
    PORTC &= 0b11100000; // ADC0-4 pull-up turn off
}

void adc_error_timer(state_t state)
{
    if (state == ENABLE)
    {
        TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);
    }
    else
    {
        TCCR2B &= ~((1 << CS22) | (1 << CS21) | (1 << CS20));
    }
}

void adc_get_data(void)
{
    static uint8_t count = 0;
    static volatile uint16_t sum  = 0;
    switch (adc_active_channel)
    {
        case ADC_CHANNEL_SWR:
            ++count;
            if (count > 2 && count < 6)
                sum += ADC;
            else if (count >= 6)
            {
                adc_ref = sum / 3;
                count        = 0;
                sum = 0;
                adc_active_channel = ADC_CHANNEL_TEMP_HEATSINK;
            }
            break;
        case ADC_CHANNEL_TEMP_HEATSINK:
            ++count;
            if (count > 2 && count < 6)
                sum += ADC;
            else if (count >= 6)
            {
                adc_temp_heatsink = sum / 3;
                count        = 0;
                sum = 0;
                adc_active_channel = ADC_CHANNEL_POWER;
            }
            break;
        case ADC_CHANNEL_POWER:
            ++count;
            if (count > 2 && count < 6)
                sum += ADC;
            else if (count >= 6)
            {
                adc_power = sum / 3;
                count        = 0;
                sum = 0;
                adc_active_channel = ADC_CHANNEL_UCC;
            }
            break;
        case ADC_CHANNEL_UCC:
            ++count;
            if (count > 2 && count < 6)
                sum += ADC;
            else if (count >= 6)
            {
                adc_ucc = sum / 3;
                count        = 0;
                sum          = 0;
                adc_active_channel = ADC_CHANNEL_ICC;
            }
            break;
        default:
            ++count;
            if (count > 2 && count < 6)
                sum += ADC;
            else if (count >= 6)
            {
                adc_icc = sum / 3;
                count   = 0;
                sum     = 0;
                adc_active_channel = ADC_CHANNEL_SWR;
            }
            break;
    }
    ADMUX = (ADMUX & 0xF0) | adc_active_channel;
} /* adc_get_data */


int16_t adc_get_temp(void)
{
    float volt, temp_log, adc_ref = ADC_REF/1000;
    int16_t ntc_resistance, temp;
        
    volt = (adc_ref / 1024) * ((float)adc_temp_heatsink);
    ntc_resistance = (-(volt * R_DIV) / adc_ref) / ((volt / adc_ref) - 1);
    temp_log = log(ntc_resistance/R_REF);
    temp = 1.0 / ( A1 + B1*temp_log + C1*temp_log*temp_log + D1*temp_log*temp_log*temp_log) - 273.15;
    
    //temp = (-68.504) * ((adc_temp_heatsink * ADC_REF) / 1024.0) + 139.33;
    //temp = ((adc_temp_heatsink * 2.502) / 1024.0);
    
    return temp;
}

char* adc_get_swr(uint16_t pwr, uint16_t ref)
{
    float return_loss;
    uint32_t swr;
    
    uint16_t int_part;
    uint16_t dec_part;
    static char buffer[20];
    
    return_loss = sqrt((float) ref/pwr);
    
    swr = ((1 + return_loss) * 1000 ) / (1 - return_loss);
    
    if ( (swr - (swr/100) * 100) > 50)
        swr += 10;
    
    int_part = swr/1000;
    dec_part = (swr%1000)/10;
    if (int_part == 0)
        sprintf_P(buffer, PSTR(" -.-- "));
    else if (pwr < ref)
        sprintf_P(buffer, PSTR(" ERR "));
    else if (int_part > 999)
        sprintf_P(buffer, PSTR(" %4d "),int_part);
    else if (int_part > 99)
        sprintf_P(buffer, PSTR("%2d.%1d "),int_part, dec_part);
    else
        sprintf_P(buffer, PSTR("%2d.%02d "),int_part, dec_part);
    
    return buffer;
    
}

uint16_t adc_get_pwr(void)
{
    uint16_t pwr;
    float volts = (float) adc_power * (ADC_REF/1000) / 1024.0; //(double) adc_power * ADC_REF / 1024UL;
    //volts ;
    pwr = -5.518777275*volts*volts*volts + 26.63047832*volts*volts + 0.1985508811*volts + 0.7206280062;
    
    return pwr;
}

uint16_t adc_get_ref(void)
{
    uint16_t ref;
    float volts = (float) adc_ref * (ADC_REF/1000) / 1024.0; //(double) adc_power * ADC_REF / 1024UL;
    //volts ;
    ref = -5.518777275*volts*volts*volts + 26.63047832*volts*volts + 0.1985508811*volts + 0.7206280062;
   
   return ref;
}

uint16_t adc_get_icc(void)
{
    uint16_t icc;
    icc = (((uint32_t) adc_icc * ADC_REF) / 1024) * 20;
    
    return icc;
}

uint16_t adc_get_ucc(void)
{
    uint16_t ucc;
    ucc = ( ((uint32_t) adc_ucc * ADC_REF) / 1024) * (280899/10000); // * 28.08988764 or  27.92008197
    
    return ucc;
}

void adc_block_pa(adc_block_t adc_block)
{
    switching_state = SWITCHING_OFF;
    timer_ovf_count = 1220;
    ptt_set_irq(DISABLE);
    switching_off_sequence();
    pom = "HI";
    if (adc_block == BLOCK_TIMER)
    {
        timer_ovf_count = 0;
    }
    adc_error_timer(ENABLE);
}

result_t adc_check_ref(void)
{
    if (adc_ref > ADC_REF_VOLTAGE_MAX)
        return ERROR;

    return SUCCESS;
}

result_t adc_check_temp(void)
{
    if (adc_temp_heatsink < ADC_TEMP_HEATSINK_MAX && adc_temp_heatsink > ADC_TEMP_HEATSINK_ABS_MAX)
        return ERROR;
    else if (adc_temp_heatsink <= ADC_TEMP_HEATSINK_ABS_MAX)
        return BIG_ERROR;
    
    return SUCCESS;
}

result_t adc_check_ucc(void)
{
    if (adc_ucc < ADC_UCC_MIN)
        return ERROR;
    else if (adc_ucc > ADC_UCC_MAX)
        return ERROR;
    
    return SUCCESS;
}

result_t adc_check_icc(void)
{
    if (adc_icc > ADC_ICC_MAX)
        return ERROR;
    
    return SUCCESS;
}

void adc_evaluation(void)
{
    if (adc_check_ref() == SUCCESS)
    ;
    else
    {
        adc_ref_cache = adc_ref;
        ui_state = UI_HI_SWR;
        adc_block_pa(BLOCK_TIMER);
    }
    
    if (adc_check_ucc() == SUCCESS)
    ;
    else
    {
        ui_state = UI_VOLTAGE_BEYOND_LIM;
        adc_block_pa(BLOCK_ONLY);
    }
    if (adc_check_icc() == SUCCESS)
    ;
    else
    {
        ui_state = UI_CURRENT_OVERLOAD;
        adc_block_pa(BLOCK_TIMER);
    }
    if (adc_check_temp() == SUCCESS || adc_check_temp() == ERROR)
    ;
    else
    {
        ui_state = UI_HI_TEMP;
        adc_block_pa(BLOCK_TIMER);
    }
    if (adc_check_temp() == ERROR || adc_check_temp() == BIG_ERROR || switching_state == SWITCHING_ON)
        SWITCHING_FAN_ON;
    else
        SWITCHING_FAN_OFF;
        
}

ISR(ADC_vect)
{
    cli();
    adc_get_data();
    if (ui_state != UI_INIT)
        adc_evaluation();
    sei();
    ADCSRA |= 1 << ADSC;
}

ISR(TIMER2_OVF_vect)
{
    if (++timer_ovf_count > 1225) // 20s 1225 3s 183
    {
        timer_ovf_count = 0;
        pom = "OK";
        adc_error_timer(DISABLE);
        ui_state = UI_RUN;
        ptt_set_irq(ENABLE);
    }
}
