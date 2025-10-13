#include <LPC214x.h>
#include "header.h"

void uart_init(unsigned int baudrate) {
    unsigned int Fdiv;
    PINSEL0 |= 0x00000005;      // P0.0 = TXD0, P0.1 = RXD0

    U0LCR = 0x83;               // 8-bit, 1 stop, no parity, DLAB=1
    Fdiv = (15000000 / 16) / baudrate;  // PCLK=15MHz
    U0DLM = Fdiv / 256;
    U0DLL = Fdiv % 256;
    U0LCR = 0x03;               // DLAB = 0
}

void uart_tx(unsigned char ch) {
    while (!(U0LSR & 0x20));    // Wait until THR empty
    U0THR = ch;
}

void uart_tx_string(char *str) {
    while (*str) {
        uart_tx(*str++);
    }
}
