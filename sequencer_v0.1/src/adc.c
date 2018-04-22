#include "adc.h"

adc_channel_t adc_active_channel = ADC_CHANNEL_SWR; // default first channel in ADC process
adc_block_t adc_block;

volatile uint16_t adc_swr;
volatile uint16_t adc_swr_cache;
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
                adc_swr = sum / 3;
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


uint8_t adc_get_temp(void)
{
    float volt, temp_log;
    int16_t ntc_resistance, temp;
    
    volt = (ADC_REF / 1024) * ((float)adc_temp_heatsink);
    ntc_resistance = (-(volt * R_DIV) / ADC_REF) / ((volt / ADC_REF) - 1);
    temp_log = log(ntc_resistance/R_REF);
    temp = 1.0 / ( A1 + B1*temp_log + C1*temp_log*temp_log + D1*temp_log*temp_log*temp_log) - 273.15;
    
    return temp;
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

result_t adc_check_swr(void)
{
    if (adc_swr > ADC_SWR_VOLTAGE_MAX)
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
    if (adc_check_swr() == SUCCESS)
    ;
    else
    {
        adc_swr_cache = adc_swr;
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
