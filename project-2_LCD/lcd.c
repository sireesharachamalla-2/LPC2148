//lcd.c
#include "types.h"
#include <LPC21xx.h>
#include "lcd_defines.h"
#include "defines.h"
#include "delay.h"
void WriteLCD(u8 byte)
{
	
	SCLRBIT(IOCLR0,LCD_RW);
	WRITEBYTE(IOPIN0,LCD_DATA,byte);
	
	SSETBIT(IOSET0,LCD_EN);
	
	delay_us(1);
	SCLRBIT(IOCLR0,LCD_EN);
	delay_ms(2);
}

void CmdLCD(u8 cmd)
{
	SCLRBIT(IOCLR0,LCD_RS);	 	
	WriteLCD(cmd);
}

void InitLCD(void)
{
  
  WRITEBYTE(IODIR0,LCD_DATA,0XFF);
  
  SETBIT(IODIR0,LCD_RS);
  
  SETBIT(IODIR0,LCD_RW);
	
  SETBIT(IODIR0,LCD_EN);
	
	delay_ms(15);
	
	CmdLCD(0x30);
	
	delay_ms(1);
	
	CmdLCD(0x30);
	
	delay_us(100);
	
	CmdLCD(0x30);

	CmdLCD(MODE_8BIT_2LINE);	//CmdLCD(0x38);

	CmdLCD(DSP_ON_CUR_BLINK);	//CmdLCD(0x0F);
	
	CmdLCD(CLEAR_LCD);	//CmdLCD(0x01);
	
	CmdLCD(SHIFT_CUR_RIGHT);	//CmdLCD(0x06);
}

void CharLCD(u8 asciiVal)
{
	
	SSETBIT(IOSET0,LCD_RS);	

	WriteLCD(asciiVal);
}	

void SetCursor(u8 lineNo,u8 pos)
{
	if(lineNo==1)
		CmdLCD(GOTO_LINE1_POS0+pos);
	else if(lineNo==2)
		CmdLCD(GOTO_LINE2_POS0+pos);
}	

void StrLCD(s8 *str)
{
	while(*str)
		CharLCD(*str++);
}
