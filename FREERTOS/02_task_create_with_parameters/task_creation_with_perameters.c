#include <LPC214x.h>
#include "FreeRTOS.h"
#include "task.h" 

void task1(void *param);
void task2(void *param);
void init_pll(void);
void initserial(unsigned int baudrate);
void sendserial(char c);


static unsigned long PCLK;


int main(void)
{
    init_pll();             
    initserial(9600);       

    
    xTaskCreate(task1, "Task1", 128, NULL, 2, NULL);
    xTaskCreate(task2, "Task2", 128, NULL, 1, NULL);

    
    vTaskStartScheduler();

    
    while (1);
}


void task1(void *param)
{
    const char *msg = "Task 1 is running\r\n";
    while (1)
    {
        const char *p = msg;
        while (*p)
            sendserial(*p++);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


void task2(void *param)
{
    const char *msg = "Task 2 is running\r\n";
    while (1)
    {
        const char *p = msg;
        while (*p)
            sendserial(*p++);
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

void initserial(unsigned int baudrate)
{
    unsigned int divisor;

    VPBDIV = 0x01;              
    PCLK   = 60000000;          

    PINSEL0 |= 0x05;            
    U0LCR   = 0x83;            

    divisor = (PCLK / (16 * baudrate));
    U0DLL   = divisor & 0xFF;
    U0DLM   = (divisor >> 8) & 0xFF;

    U0LCR   = 0x03;             
}

void sendserial(char c)
{
    while (!(U0LSR & (1 << 5)));  
    U0THR = c;
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
    PCLK   = 60000000;
}

void vApplicationMallocFailedHook(void)
{
    while (1);   
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    while (1);   
}

void vApplicationIdleHook(void)
{
    // Do nothing 
}
