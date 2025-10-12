#include "delay.h"
#include"types.h"
void delay_s(u32 ds)
{
	for(ds*=12000000;ds>0;ds--);
}
void delay_ms(u32 dms)
{
	for(dms*=12000;dms>0;dms--);
}
void delay_us(u32 dus)
{
	for(dus*=12;dus>0;dus--);
}
