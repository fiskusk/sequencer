/*
 * Name: HD44780.c
 * Chip: ATMega328p
 * Created: 10.3.2018 20:11:55
 * Author : KLAPIL Filip (Assumed from POSPISIlL Frantisek)
 * Hardware: HD44780 4 bit bus
 */

/* Headliners */

#include "HD44780.h"

#ifndef F_CPU
    #define F_CPU 16000000UL
#endif

void lcd_init(void) // initialization of display
{
    DDR(LCD_Data) |= 0b00111111;
    _delay_ms(LCD_DELAY_BOOTUP);
    LCD_ClrEnable();
    LCD_Data = 0b00001100;
    toggle_e();
    _delay_us(LCD_DELAY_INIT);
    toggle_e();
    _delay_us(LCD_DELAY_INIT_REP);
    toggle_e();
    _delay_us(LCD_DELAY_INIT_REP);

    // Function set
    // 0 0 1 DL N F x x DL0=4bit, DL1=8bit; N0=1lines, N1=2lines; F0=5×8dots, F1=5×10dots
    LCD_WriteDC(0b00101000, 0);

    // Display on/off function (first hold off)
    // 0000 1 D C B D0=dis off, D1=dis on, C0=cursor off, C1=cursor on, B0=blinks off, B1=blinks on
    LCD_WriteDC(0b00001000, 0);
    lcd_clrscr();

    // Entry mode set
    // 0000 01 ID S ID0=decrement ID1=increment, S1=Accompanies display shift
    LCD_WriteDC(0b00000110, 0);

    // Now turn on display and sets on of function
    // 0000 1 D C B D0=dis off, D1=dis on, C0=cursor off, C1=cursor on, B0=blinks off, B1=blinks on
    LCD_WriteDC(0b00001100, 0);
}

void lcd_clrscr(void) // clear display and set position of cursor on default value
{
    LCD_WriteDC(1, 0);
    _delay_ms(250);
}

void LCD_WriteDC(unsigned char val, unsigned char DC) // write a data or a command code to display
{
    if (DC == 0){
        LCD_SetCommand();}
    else
        LCD_SetData();
    _delay_us(80);
    LCD_Data = (LCD_Data & 0b11000011) | ((val >> 2) & 0b00111100);
    _delay_us(80);
    LCD_SetEnable();
    _delay_us(80);
    LCD_ClrEnable();
    _delay_us(80);
    LCD_Data = (LCD_Data & 0b11000011) | ((val << 2) & 0b00111100);
    _delay_us(80);
    LCD_SetEnable();
    _delay_us(80);
    LCD_ClrEnable();
    _delay_us(100);
}

void lcd_gotoxy(unsigned char y, unsigned char x) // set position of cursor
{
    switch (y)
    {
        case 0:
            x += 0x00;
            break;
        case 1:
            x += 0x40;
            break;
        case 2:
            x += 0x14;
            break;
        case 3:
            x += 0x54;
            break;
    }
    LCD_WriteDC(x | 0x80, 0); // x is DDRAM address of position and 0x80 is command to set DDRAM address
}

void lcd_puts(const char *s) // draw on display a constant string
{
    while (*s != '\0')
    {
        LCD_WriteDC(*s, 1);
        s += 1;
    }
}

void toggle_e(void)
{
    LCD_SetEnable();
    _delay_us(ENABLE_DELAY_PULSE);
    LCD_ClrEnable();
}