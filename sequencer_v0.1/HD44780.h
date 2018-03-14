/*
 * Name: HD44780.h
 * Chip: ATMega328p
 * Created: 10.3.2018 20:11:55
 * Author : KLAPIL Filip
 * Hardware: HD44780 4 bit bus
 */

 /* define of macros */
#define LCD_Data		PORTB
#define LCD_SetEnable()	PORTB |= 0x02;      //PORTC1 is E - starts data write/read (Enable H, Disable L) HW pin 24
#define LCD_ClrEnable() PORTB &=~0x02;
#define LCD_SetData()	PORTB |= 0x01;      //PORTC0 is RS (Data H, command L) HW pin 23
#define LCD_SetCommand()PORTB &=~0x01;

void lcd_init(void);                                              // initialization of display
void lcd_clrscr(void);                                             // clear display and set cursor on default pos
void LCD_WriteDC(unsigned char val,unsigned char DC);             // write a command code to display
void lcd_gotoxy(unsigned char y, unsigned char x);	              // set position of cursor
void lcd_puts(const char *s);                               // draw on display a constant string



