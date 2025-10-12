#include "delay.h"
#include"types.h"
void delay_s(u32 ds)
{
	//~1s @CCLK=60MHZ
	for(ds*=12000000;ds>0;ds--);
}
void delay_ms(u32 dms)
{
	//~1ms @CCLK=60MHZ
	for(dms*=12000;dms>0;dms--);
}
void delay_us(u32 dus)
{
	//~1us @CCLK=60MHZ
	for(dus*=12;dus>0;dus--);
}
