#include <LPC213x.h>
#include <stdio.h>

void UartInit(void);
void UartSend(char ch);
char UartRec(void);
void UartStr(char *str);
void delay_ms(unsigned int ms);
void adc_init(void);
unsigned int adc_read(void);


void UartInit(void)
{
    PINSEL0 |= 0x00000005;   
    U0LCR = 0x83;            
    U0DLL = 97;              
    U0LCR = 0x03;           
}

//transmit data
void UartSend(char ch)
{
	while(!(U0LSR &0x20)); //wait until THR empty
	U0THR =ch;
}

//receive data
char UartRec(void)
{
	while(!(U0LSR&0x01));  //wait until data ready
	return U0RBR;    //return receiced data
}

//transmit string
void UartStr(char *str)
{
	while(*str)
	{
		UartSend(*str++);
	}
}

//delay_generation
void delay_ms(unsigned int ms) {
    unsigned int i,j;
    for(i=0;i<ms;i++) {
        for(j=0;j<10000;j++);
    }
}


// ADC (P0.28 = AD0.1)
void adc_init(void) {
  //  PINSEL1 |= (1<<24);      // P0.28 as AD0.1
	PINSEL1 |=(1<<26); //p0.29 as AD0.2
  //  AD0CR = (1<<1)            // Select channel 1
    AD0CR=(1<<2)       //select channel2
					| (4<<8)            // Clock divisor
          | (1<<21);          // Enable ADC
}
// Read  value
unsigned int adc_read(void) {
    unsigned int result;
    AD0CR |= (1<<26);         // Start conversion now
    while((AD0GDR & (1U<<31)) == 0); // Wait until done
    result = (AD0GDR >> 6) & 0x3FF;  // 10-bit result
    return result;
}

//main function
int main(void) 
{
    char buffer[16];
    unsigned int adc_val;
	unsigned char msg[]= "LM35 Sensor\r\n";
    float voltage, temp;
    adc_init();
	  UartInit();
		
		UartStr(msg);
		delay_ms(500);
		while(1) 
		{
        adc_val = adc_read();
        voltage = (adc_val * 3.3) / 1023.0; // Convert ADC to voltage
        temp = voltage * 100.0;              
				sprintf(buffer,"Temp: %.2f C  \r\n ", temp);  //convert int to strng 
				UartStr(buffer);
			
        delay_ms(500);
   }
}
