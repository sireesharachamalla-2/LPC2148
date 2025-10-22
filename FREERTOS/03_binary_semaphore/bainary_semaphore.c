#include <LPC214x.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


void initserial(void);
void sendsserial(const char *p);

void initserial(void) {
    VPBDIV = 0x01;                   
    PINSEL0 |= 0x00000005;           
    U0LCR = 0x83;                    
    U0DLM = 0x00;
    U0DLL = 97;                     
    U0LCR = 0x03;                  
}

void sendsserial(const char *p) {
    while (*p) {
        while (!(U0LSR & 0x20));     
        U0THR = *p++;
    }
}

void initpll(void) {
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


void task1(void *p);
void task2(void *p);

SemaphoreHandle_t binarysem;

//----------  MAIN---------------------
int main(void) {
    initpll();
    initserial();

    binarysem = xSemaphoreCreateBinary();
    if (binarysem != NULL) {
        xSemaphoreGive(binarysem);   //give once initially
    }

    xTaskCreate(task1, "task1", 128, NULL, 1, NULL);
    xTaskCreate(task2, "task2", 128, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1);  
}
void task1(void *p) {
    while (1) {
        if (xSemaphoreTake(binarysem, portMAX_DELAY) == pdTRUE) {
            sendsserial("Task1 functioning\r\n");
            xSemaphoreGive(binarysem);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));   // delay 1s
    }
}

void task2(void *p) {
    while (1) {
        if (xSemaphoreTake(binarysem, portMAX_DELAY) == pdTRUE) {
            sendsserial("Task2 functioning\r\n");
            xSemaphoreGive(binarysem);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));   
    }
}
