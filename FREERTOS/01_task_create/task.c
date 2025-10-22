#include <LPC21xx.h>
#include "FreeRTOS.h"
#include "task.h"
void UART0_Init(void) {
    VPBDIV = 0x01;
    PINSEL0 |= 0x00000005;
    U0LCR = 0x83;
    U0DLM = 0x00;
    U0DLL = 97;
    U0LCR = 0x03;
}

void UART0_SendChar(char c) {
    while (!(U0LSR & 0x20));
    U0THR = c;
}

void UART0_SendString(const char *s) {
    while (*s) UART0_SendChar(*s++);
}

int main(void) {
    UART0_Init();
    UART0_SendString("task created\r\n");
    while (1);
}