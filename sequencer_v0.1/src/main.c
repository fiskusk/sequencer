#include "settings.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "lcd.h"
#include "uart.h"

#include "adc.h"
#include "ptt.h"
#include "switching.h"

// global variables
char *pom; // auxiliary variable for sending message to UART or LCD

void setup(void)
{
    // setup ports
    DDRB |= 0b01111111;          // set display pins as output (H output)
    DDRC  = (1 << 5) | (1 << 6); // set PORTC5 as output

    ptt_init();
    adc_init();
    switching_init();

    // setup LCD and UART
    lcd_init(LCD_DISP_ON); // initialization display
    lcd_clrscr();          // clear display
    uart_init();           // initialization UART

    sei(); // enable all interrupts
}

int main(void)
{
    uint8_t cela_cast  = 0;
    uint16_t desetinna = 0;
    float des_tvar     = 0;
    char buffer[9], buffer2[9], buffer3[9];

    setup();

    // test prints
    pom = "Pok";
    uart_puts("Start , vse vypne skokem do FAULT a provede prvni test v AFTER_FAULT\n");


    // in infinite loop print info to LCD
    while (1)
    {
        lcd_gotoxy(0, 0);
        lcd_puts(pom);

        lcd_gotoxy(0, 1);
        lcd_puts("    ");
        lcd_gotoxy(0, 1);
        // for (uint8_t i = 0,i<=)
        itoa(adc_power, buffer3, 10);
        lcd_puts(buffer3);

        des_tvar  = (adc_power * 1.133) / 1024.0;
        cela_cast = des_tvar;
        desetinna = (des_tvar - (float) cela_cast) * 1000;
        itoa(desetinna, buffer2, 10);
        itoa(cela_cast, buffer, 10);

        lcd_gotoxy(9, 1);
        lcd_puts(buffer);
        lcd_putc(',');
        lcd_puts(buffer2);
        lcd_puts(" V  ");

        _delay_ms(200);
    }
    return 0;
} /* main */
