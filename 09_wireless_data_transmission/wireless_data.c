#include <LPC214x.h>     
#include <stdio.h>
#include <string.h>

void UART0_Init(void);
void UART0_TxChar(char ch);
void UART0_SendString(char *str);
char UART0_RxChar(void);
void delay_ms(unsigned int ms);
void ADC_Init(void);
unsigned int ADC_Read(void);


int main(void) {
    unsigned int adc_value;
	  int i = 0;
    char buffer[30];
    char rx_cmd[10];

    IO0DIR |= (1 << 10);     // P0.10 as output for LED

    UART0_Init();            // Initialize UART0 for Bluetooth
    ADC_Init();              // Initialize ADC

    UART0_SendString("\r\nBluetooth Wireless UART\r\n");
    UART0_SendString("Send 'LED ON' or 'LED OFF' from phone\r\n");
  while (1) 
		{
    
        adc_value = ADC_Read();           // Get ADC 10-bit value

        sprintf(buffer, "Sensor: %u\r\n", adc_value);
        UART0_SendString(buffer);         // Send to Bluetooth 
        delay_ms(1000);
        while (U0LSR & 0x01)
					{            // Data available in Rx buffer
            rx_cmd[i++] = UART0_RxChar();
            if (i >= sizeof(rx_cmd) - 1) 
							break;
        }
        rx_cmd[i] = '\0';

        if (strstr(rx_cmd, "ON")) {
            IO0SET = (1 << 10);           // Turn ON LED
            UART0_SendString("LED Turned ON\r\n");
					
        } 
				else if (strstr(rx_cmd, "OFF")) 
					{
            IO0CLR = (1 << 10);           // Turn OFF LED
            UART0_SendString("LED Turned OFF\r\n");
        }
    }
}
void UART0_Init(void) {
    PINSEL0 |= 0x00000005;   // Enable TxD0 (P0.0) and RxD0 (P0.1)
    U0LCR = 0x83;            // 8-bit data, 1 stop, no parity, DLAB=1
    U0DLL = 97;              // 9600 baud @ 15MHz PCLK
    U0DLM = 0;
    U0LCR = 0x03;            // DLAB = 0
}

void UART0_TxChar(char ch) {
    while (!(U0LSR & 0x20)); // Wait for THR empty
    U0THR = ch;
}

void UART0_SendString(char *str) {
    while (*str)
        UART0_TxChar(*str++);
}

char UART0_RxChar(void) {
    while (!(U0LSR & 0x01)); // Wait for data ready
    return U0RBR;
}
void ADC_Init(void) {
    PINSEL1 |= (1 << 24);    // P0.28 as AD0.1
    AD0CR = (1 << 1) |       // Select channel AD0.1
            (4 << 8) |       // CLKDIV
            (1 << 21);       // ADC enabled
}

unsigned int ADC_Read(void) {
    unsigned int val;
    AD0CR |= (1 << 24);                // Start conversion
    while ((AD0DR1 & (1 << 31)) == 0); // Wait until DONE bit set
    val = (AD0DR1 >> 6) & 0x3FF;       // Extract 10-bit result
    return val;
}

void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 2000; j++);
}
