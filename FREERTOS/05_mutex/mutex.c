#include <LPC214x.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


void init_pll(void);
void init_uart(unsigned int baud);
void uart_tx_string(const char *str);
void task1(void *pvParameters);
void task2(void *pvParameters);


SemaphoreHandle_t xMutex;

int main(void)
{
    init_pll();
    init_uart(9600);
	  uart_tx_string("task start\r\n");
    xMutex = xSemaphoreCreateMutex();

    if (xMutex != NULL)
    {
        xTaskCreate(task1, "Task1", 128, NULL, 1, NULL);
        xTaskCreate(task2, "Task2", 128, NULL, 1, NULL);
        vTaskStartScheduler();
    }

    while (1); 
}


void task1(void *pvParameters)
{
    while (1)
    {
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
        {
            uart_tx_string("Task 1 running\r\n");
            xSemaphoreGive(xMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


void task2(void *pvParameters)
{
    while (1)
    {
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
        {
            uart_tx_string("Task 2 running\r\n");
            xSemaphoreGive(xMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


void init_pll(void)
{
    PLL0CON = 0x01;
    PLL0CFG = 0x24;        
    PLL0FEED = 0xAA;
    PLL0FEED = 0x55;
    while (!(PLL0STAT & (1 << 10))); 
    PLL0CON = 0x03;        
    PLL0FEED = 0xAA;
    PLL0FEED = 0x55;
    VPBDIV = 0x01;         
}


void init_uart(unsigned int baud)
{
    unsigned long pclk = 15000000;
    unsigned int divisor = pclk / (16 * baud);

    PINSEL0 |= 0x00000005;  
    U0LCR = 0x83;           
    U0DLL = divisor & 0xFF;
    U0DLM = (divisor >> 8) & 0xFF;
    U0LCR = 0x03;        
    U0FCR = 0x07;           
}


void uart_tx_string(const char *str)
{
    while (*str)
    {
        while (!(U0LSR & 0x20)); 
        U0THR = *str++;
    }
}


void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    uart_tx_string("STACK OVERFLOW\r\n");
    while (1);
}
