/*
 * sequencer_v0.1.c
 *
 * Created: 10.3.2018 20:11:55
 * Author : fkla
 */

// include used libraries
#define  F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

// include my libraries
#include "main.h"           // main library
#include "lcd.h"            // library for LCD display
#include "uart.h"           // library for UART to debuging
#include "events.h"         // library functions for events

// global variables
uint8_t way = 0;                    // default way is to turning off (normally when PTT push off)
uint8_t fault_count = 9;            // presets the fault counter to the default value (only one looú)
uint8_t fault_flag = 2;             // 0 bez poruchy, 1 porucha, 2 prvni zapnuti-test
char *pom;                          // auxiliary variable for sending message to UART or LCD

volatile uint8_t cela_cast = 0;
volatile uint16_t desetinna = 0;
volatile float des_tvar = 0;
char buffer[9], buffer2[9], buffer3[9];
                   
sequencer_t old_state;              
sequencer_t actual_state = FAULT;   // default after start up device, go to fault event

void setup(void)
{
    // setup ports
    DDRB |= 0b01111111;    // set display pins as output (H output)
    DDRD &= ~(1<<2);       // INT0 as input (L)
    PORTD |= 1<<2;         // INT0 H pull-up, L Hi-impedance
    
    // setup TIMER1
    EICRA |= 1<<ISC00;     // any logical change INT0 generate interrupt
    TIMSK1 |= 1<<TOIE1;    // enable interrupt when overflow Timer1
    
    // setup ADC
    // internally reference
    // 0. external AREF (internal Vref disabled),
    // 1. AVCC with external cap at AREF pin,
    // 2. reserved
    // 3. Internal 1,1V voltage ref. with external cap on AREF pin
    ADMUX = (1<<REFS1) | (1<<REFS0);
    
    // ADENable, ADStart Conversion, ADInterrupt Enable
    // when set ADATE - ADCH MSB, ADCL LSB
    // ADPrescaler Select - 2,2,4,8,16,32,64,128
    ADCSRA = (1<<ADEN) | (1<<ADSC) | (1<<ADATE) | (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2);
    
    PRR &= ~(1<<PRADC);
    
    // setup LCD and UART
    lcd_init(LCD_DISP_ON); // initialization display
    lcd_clrscr();          // clear display
    uart_init();           // initialization UART
    
    sei();                 // enable all interrupts
}

int main(void)
{
    setup();
    
    //test prints
    pom = "Pok";            
    uart_puts("Start , vse vypne skokem do FAULT a provede prvni test v AFTER_FAULT\n");
    
	// after startup, actual_state was set in setup() to fault
    // now jump immediately to ISR_timer1 to execute routine of fault
    TIFR1 |= 1<<TOV1;
    fault_flag = 2;             // flag set to identifing first startup device
    timer1_set_state(ENABLE);   // Timer1 GO!
    
    // in infinite loop print info to LCD
    while (1)
    {
        lcd_gotoxy(0,0);
        lcd_puts(pom);
        
        lcd_gotoxy(0,1);
        lcd_puts("    ");
        lcd_gotoxy(0,1);
        itoa(des_tvar,buffer3,10);
        lcd_puts(buffer3);
        
        des_tvar = (des_tvar*1.1)/1023.0;
        cela_cast = des_tvar;
        desetinna = (des_tvar - (float)cela_cast)*1000;
        itoa(desetinna,buffer2,10);
        itoa(cela_cast,buffer,10);
        
        lcd_gotoxy(9,1);
        lcd_puts(buffer);
        lcd_putc(',');
        lcd_puts(buffer2);
        lcd_puts(" V  ");
        
        _delay_ms(200);        
    };
    return 0;
}

ISR(INT0_vect)
{
    Event_PTT_button_status_changed(); 
}

ISR(TIMER1_OVF_vect)
{
    switch(actual_state)
    {
        case EVENT0:
            E0_on_off_relay2();
            break;
        case EVENT1:
            E1_on_off_bias();
            break;
        case EVENT2:
            E2_on_Ucc_off_relay1();
            break;
        case FAULT:
            fault_off_all();
            break;
        case AFTER_FAULT:
            after_fault_check_status();
            break;
        case TEST_PTT:
            test_state_of_PTT_button();
            break;
        default:
            actual_state = FAULT;
    }
}

ISR(ADC_vect)
{
    cli();
    // test prints
       
    // conversion to display 
    des_tvar = ADC;  
    if ((ADC<UMIN) || (ADC>UMAX))
    {
        adc_set_state(DISABLE);
        uart_puts("ADC hodnota mimo rozsah, generuji chybu\n");
        actual_state = FAULT;
        TIFR1 |= 1<<TOV1;
        fault_flag = 1;
        fault_count = 0;
        timer1_set_state(ENABLE);
    }
    else if (fault_flag>0)
    {
        adc_set_state(DISABLE);
        uart_puts("hodnota ADC je OK, vracim fault_flag = 0\n");
        actual_state = AFTER_FAULT;
        fault_flag = 0;
        TIFR1 |= 1<<TOV1;
        timer1_set_state(ENABLE);
    }
    sei();
}
