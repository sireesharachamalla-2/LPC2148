#include<LPC21xx.h>
#include "delay.h"
#define LED 11
int main()
{
	IODIR0|=(0x01<<LED); //p0.11
	while(1)
	{
		IOSET0|=(0x01<<LED);
		delay_s(1);
		IOCLR0|=(0x01<<LED);
		delay_ms(500);
 }
}

