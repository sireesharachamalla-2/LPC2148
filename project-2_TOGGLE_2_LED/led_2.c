#include<LPC21xx.h>
#include "delay.h"
#define LED1 3 //p0.3
#define LED2 23 //p1.23
int main()
{
	IODIR0|=(0x01<<LED1);
	IODIR1|=(0x01<<LED2);
	while(1)
	{
		IOSET0|=(0x01<<LED1);
		IOCLR1|=(0x01<<LED2);
		delay_ms(200);
		IOCLR0|=(0x01<<LED1);
		IOSET1|=(0x01<<LED2);
		delay_ms(200);
	}
}