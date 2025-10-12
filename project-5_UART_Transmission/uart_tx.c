#include <lpc21xx.h>
#include <stdint.h>
 
void UartInit(void)
{
    PINSEL0 |= 0x00000005;  
 
    U0LCR = 0x83;    
    U0DLL = 97;     
    U0LCR = 0x03; 
	  return;
}
 
void UartSend(uint8_t* ucData, uint16_t size)
{
	uint8_t idx = 0;
	while(idx < size)
	{
	  while (!(U0LSR & 0x20));
	  U0THR = ucData[idx];
		idx += 1;
	}  
}
 
void delay(uint32_t ms)
{
	unsigned int i, j;
	for(i = 0; i < ms; ++i)
	{
		for(j = 0; j < 6000; ++j);
	}
}
 
int main() { 
	uint8_t msg[] = "Hello world\r\n"; 
  UartInit();
    while (1) {
			UartSend(msg,sizeof(msg));
			delay(500);
    }
}
