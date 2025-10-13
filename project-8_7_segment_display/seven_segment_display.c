#include <lpc214x.h>

void delay_ms(unsigned int ms) {

    for(ms*=12000;ms>0; ms--);
	
}
unsigned char segment_code[10] = {
    0x3F,																  // 0
    0x06, 													  		      // 1
    0x5B, 																 // 2
    0x4F,																 // 3
    0x66,																 // 4
    0x6D, 																 // 5
    0x7D, 																 // 6
    0x07, 																 // 7
    0x7F, 																 // 8
    0x6F  																 // 9
};
int main(void) {
	int i;
    IO0DIR |= 0x7F; 

    while (1) {
        for ( i = 0; i < 10; i++) {
            IO0CLR = 0x7F;                 // Clear previous output
            IO0SET = segment_code[i];      // Send segment code
            delay_ms(500);                 // Delay 
        }
    }
}
