#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>
#include <LPC214x.h>

void init_uart(unsigned int baudrate);
void uart_tx(unsigned char data);
void uart_tx_string(char *str);
unsigned char uart_rx(void);

void sender(void *q);
void receiver(void *q);


xQueueHandle myqueue;

int main(void) {
    init_uart(9600);

    myqueue = xQueueCreate(5, sizeof(unsigned char)); 

    if (myqueue != NULL) {
        xTaskCreate(sender, "Sender", 128, NULL, 1, NULL);
        xTaskCreate(receiver, "Receiver", 128, NULL, 1, NULL);
        vTaskStartScheduler();
    }

    while (1); 
}

void sender(void *q) {
    unsigned char num = 0;
    while (1) {
        xQueueSendToBack(myqueue, &num, portMAX_DELAY);
        num++;
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}

void receiver(void *q) {
    unsigned char receivedValue;
    char buffer[10];
    while (1) {
        if (xQueueReceive(myqueue, &receivedValue, portMAX_DELAY) == pdPASS) {
            sprintf(buffer, "%d\r\n", receivedValue);
            uart_tx_string(buffer);
        }
    }
}


void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    uart_tx_string("Stack Overflow!\r\n");
    while (1);
}


void init_uart(unsigned int baudrate) {
    unsigned int divisor = (60000000 / (16 * baudrate)); 
    PINSEL0 |= 0x00000005;   
    U0LCR = 0x83;            
    U0DLL = divisor & 0xFF;
    U0DLM = (divisor >> 8) & 0xFF;
    U0LCR = 0x03;            
}

void uart_tx(unsigned char data) {
    while (!(U0LSR & (1 << 5))); 
    U0THR = data;
}

void uart_tx_string(char *str) {
    while (*str) {
        uart_tx(*str++);
    }
}

unsigned char uart_rx(void) {
    while (!(U0LSR & 0x01));
    return U0RBR;
}
