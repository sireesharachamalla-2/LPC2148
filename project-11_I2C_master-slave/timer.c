#include <LPC214x.h>
#include "timer.h"

void delay_ms(unsigned int ms) {
    T0PR = 15000 - 1;    // 15 MHz / 15000 = 1 kHz (1 ms tick)
    T0TCR = 0x02;        // Reset Timer0
    T0TCR = 0x01;        // Enable Timer0

    while (T0TC < ms);   // Wait until Timer Counter reaches 'ms'
    T0TCR = 0x00;        // Stop Timer0
}
