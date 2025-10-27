#include <LPC214x.h>
#include <stdio.h>
#include <string.h>

//lcd pins
#define RS (1<<16)   // P0.16
#define EN (1<<17)   // P0.17
#define D4 (1<<18)   // P0.18
#define D5 (1<<19)   // P0.19
#define D6 (1<<20)   // P0.20
#define D7 (1<<21)   // P0.21

void delay_ms(unsigned int ms)
{
    unsigned int i, j;
    for(i=0; i<ms; i++)
        for(j=0; j<6000; j++);
}

void write_enable(void)
{
    IOSET0 = EN;
    delay_ms(1);
    IOCLR0 = EN;
}

void LCD_write(unsigned char data)
{
    if(data & 0x10) 
			IOSET0 = D4; 
		else 
			IOCLR0 = D4;
    if(data & 0x20)
			IOSET0 = D5; 
		else 
			IOCLR0 = D5;
    if(data & 0x40) 
			IOSET0 = D6; 
		else
			IOCLR0 = D6;
    if(data & 0x80) 
			IOSET0 = D7; 
		else
			IOCLR0 = D7;
    write_enable();
}

void LCD_cmd(unsigned char cmd)
{
    IOCLR0 = RS;               
    LCD_write(cmd & 0xF0);
    LCD_write((cmd << 4) & 0xF0);
    delay_ms(2);
}

void LCD_data(unsigned char data)
{
    IOSET0 = RS;               
    LCD_write(data & 0xF0);
    LCD_write((data << 4) & 0xF0);
    delay_ms(2);
}

void LCD_init(void)
{
    IODIR0 |= RS | EN | D4 | D5 | D6 | D7;   // output

    delay_ms(20);
    LCD_cmd(0x02);  
    LCD_cmd(0x28);  
    LCD_cmd(0x0C);  
    LCD_cmd(0x06);  //move cursor to right
    LCD_cmd(0x01);  // Clear display
}

void LCD_str(const char *str)
{
    while(*str)
        LCD_data(*str++);
}

void LCD_gotoxy(unsigned char row, unsigned char col)
{
    unsigned char pos;
    if(row == 1)
        pos = 0x80 + (col - 1);
    else
        pos = 0xC0 + (col - 1);
    LCD_cmd(pos);
}


void LCD_draw_bar(unsigned char length)
{
    unsigned char i;
    for(i = 0; i < length; i++)
        LCD_data(0xFF);  
}

int main(void)
{
    unsigned int counter = 0;
    LCD_init();

    while(1)
    {
        LCD_cmd(0x01);  // Clear display
        LCD_gotoxy(1, 1);
        LCD_str("Bar Graph Demo");

        LCD_gotoxy(2, 1);
        LCD_draw_bar(counter % 16);  // Draw bar on LCD

        delay_ms(300);
        counter++;
        if(counter > 16) 
					counter = 0;
    }
}
