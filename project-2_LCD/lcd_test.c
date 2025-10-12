#include "lcd.h"
#include "delay.h"
#include "lcd_defines.h"
int main()
{
	//u8 i;
	InitLCD();
	CmdLCD(0x01);
	CmdLCD(0x83);
	StrLCD("Welcome to");
	delay_s(1);
	CmdLCD(0xC3);
	StrLCD("Bitsilica");
	CmdLCD(0x0C);
	while(1);
}
