/*
 * Name: HD44780.h
 * Chip: ATMega328p
 * Created: 10.3.2018 20:11:55
 * Author : KLAPIL Filip
 * Hardware: HD44780 4 bit bus
 */

 /* define of macros */
#ifndef HD44780_H_INCLUDED
#define HD44780_H_INCLUDED
 
#include <avr/io.h>
#include <util/delay.h>

#define LCD_Data PORTB
#define LCD_SetEnable() PORTB |= 0x02;      //PORTC1 is E - starts data write/read (Enable H, Disable L) HW pin 24
#define LCD_ClrEnable() PORTB &=~0x02;
#define LCD_SetData() PORTB |= 0x01;      //PORTC0 is RS (Data H, command L) HW pin 23
#define LCD_SetCommand() PORTB &=~0x01;

#define DDR(x) (*(&x - 1))

#define ENABLE_DELAY_PULSE 2                          // enable signal pulse width in micro seconds
#define LCD_DELAY_BOOTUP   16                         // delay in mili seconds after power-on  
#define LCD_DELAY_INIT 5100                           // delay in micro seconds after initialization command sent 
#define LCD_DELAY_INIT_REP 80                         // delay in micro seconds after initialization command repeated
#define LCD_DELAY_INIT_4BIT 80                        //< delay in micro seconds after setting 4-bit mode 


//======================================================
extern void lcd_init(void);                                              // initialization of display
extern void lcd_clrscr(void);                                             // clear display and set cursor on default pos
void LCD_WriteDC(unsigned char val,unsigned char DC);             // write a command code to display
void toggle_e(void);
extern void lcd_gotoxy(unsigned char y, unsigned char x);                  // set position of cursor
extern void lcd_puts(const char *s);                               // draw on display a constant string
//======================================================

#endif //HD44780_H_INCLUDED


