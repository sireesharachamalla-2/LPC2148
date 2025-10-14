#include <lpc213x.h>

void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for(i = 0; i < ms; i++) {
        for(j = 0; j < 600; j++);  //delay at 12MHz 
    }
}

int main(void) {
    // Set P0.16,P0.23 as output
		int i;
    IO0DIR |= 0x00FF0000;  
		

    while(1) {
        //  Blink all LEDs together
        IO0SET = 0x00FF0000;   // Turn ON all LEDs
        delay_ms(500);
        IO0CLR = 0x00FF0000;   // Turn OFF all LEDs
        delay_ms(500);

        // Running light (one by one)
        for(i = 16; i <= 23; i++) {
            IO0SET = (1 << i);   // Turn ON one LED
            delay_ms(200);
            IO0CLR = (1 << i);   // Turn it OFF
        }

        // Alternating pattern
        IO0SET = 0x00AA0000;  // LEDs at even
        IO0CLR = 0x00550000;  // LEDs at odd
        delay_ms(500);

        IO0SET = 0x00550000;  // LEDs at odd 
        IO0CLR = 0x00AA0000;  // LEDs at even 
        delay_ms(500);
    }
}
