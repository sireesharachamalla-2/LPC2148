#include <lpc21xx.h>
#include <stdint.h>
 
void UartInit(void)
{
    PINSEL0 |= 0x00000005;   // P0.0 = TXD0, P0.1 = RXD0
    U0LCR = 0x83;            // 8-bit, 1 stop, no parity, DLAB=1
    U0DLL = 97;              // Baud rate 9600 (for PCLK=60MHz)
    U0LCR = 0x03;            // DLAB = 0
}
 
void UartSend(uint8_t* ucData, uint16_t size)
{
    uint8_t idx = 0;
    while(idx < size)
    {
        while (!(U0LSR & 0x20));  // Wait until THR is empty
        U0THR = ucData[idx];      // Transmit character
        idx++;
    }  
}

// Function to send a single character
void UartSendChar(uint8_t ch)
{
    while (!(U0LSR & 0x20));
    U0THR = ch;
}

// Function to receive a single character
uint8_t UartReceive(void)
{
    while (!(U0LSR & 0x01));   // Wait until data ready
    return U0RBR;              // Return received byte
}
 
void delay(uint32_t ms)
{
    unsigned int i, j;
    for(i = 0; i < ms; ++i)
    {
        for(j = 0; j < 6000; ++j);
    }
		
}

int main() 
{ 
    uint8_t rx_data;
    uint8_t msg[] = "UART Ready. Type something\r\n";  

    UartInit();
    UartSend(msg, sizeof(msg));

    while (1) 
    {
        rx_data = UartReceive();    // Wait & receive character
        UartSendChar(rx_data);      // Echo back received character
    }
}
