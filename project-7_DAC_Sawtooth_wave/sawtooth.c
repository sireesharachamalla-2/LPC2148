#include<LPC214x.h>
void delay_us(unsigned int dus)
{
	for(dus=dus*12;dus>0;dus--);
}
void analogWrite(unsigned int DACdata)
{
	PINSEL1|=(2<<18);   //po0.25 as analog output
	DACR=(DACdata<<6);
}
int main()
{
	unsigned int i;
	while(1)
	{
		for(i=0;i<1000;i++)
		{
			analogWrite(i*10);
			delay_us(200);
		}
	}
}
