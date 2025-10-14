#include <lpc214x.h>
#define SW_LED 9 //p0.9
void delay_ms(unsigned int ms) {

     unsigned int i,j;
    for(i=0;i<ms;i++)
        for(j=0;j<5000;j++);
	
}
unsigned char segment_code[10] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};

int main(void) {
	unsigned int i=0;
    IODIR0 |= 0x7F; 
		IODIR0&=~(1<<SW_LED);
	   
    while (1) {
        //for ( i = 0; i < 10; i++) {
          IOCLR0 = 0x7F;       // Clear previous val
					if((IOPIN0&(1<<SW_LED))==0)
					{
            IOSET0 = segment_code[i++];      
            delay_ms(500);             
						if(i==9)
						{
							i=0;
						}
					}
    }
}
