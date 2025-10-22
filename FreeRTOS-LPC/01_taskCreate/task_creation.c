#include <LPC214x.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* ----- UART ----- */
void UART0_Init(void) {
    VPBDIV = 0x01;                      /* PCLK = CCLK */
    PINSEL0 |= 0x00000005;              /* TxD0, RxD0 */
    U0LCR = 0x83;                       /* 8 bit, DLAB = 1 */
    U0DLM = 0x00;
    U0DLL = 97;                         /* approx 9600 baud for PCLK used here; tune as required */
    U0LCR = 0x03;                       /* DLAB = 0 */
}

/* Wait until THR empty then write */
void UART0_SendChar(char c) {
    while (!(U0LSR & 0x20));    /* Wait for THR empty */
    U0THR = c;
}

/* Thread-safe SendString using a mutex externally */
void UART0_SendString(const char *s) {
    while (*s) UART0_SendChar(*s++);
}

/* ----- FreeRTOS objects ----- */
static SemaphoreHandle_t xUARTMutex;

/* Wrapper that takes mutex, sends string, then gives mutex. */
void SafeSendString(const char *s) {
    if (xUARTMutex != NULL) {
        if (xSemaphoreTake(xUARTMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
            UART0_SendString(s);
            xSemaphoreGive(xUARTMutex);
        }
    } else {
        /* If mutex not created, fall back to direct send (not thread-safe) */
        UART0_SendString(s);
    }
}

/* ----- Tasks ----- */
void Task1(void *pvParameters) {
    SafeSendString("Task1: started\r\n");
    for (;;) {
        SafeSendString("Task1 running\r\n");
        vTaskDelay(pdMS_TO_TICKS(500)); /* yield and allow time slicing */
    }
}

void Task2(void *pvParameters) {
    SafeSendString("Task2: started\r\n");
    for (;;) {
        SafeSendString("Task2 running\r\n");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void Task3(void *pvParameters) {
    SafeSendString("Task3: started\r\n");
    for (;;) {
        SafeSendString("Task3 running\r\n");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* ----- main ----- */
int main(void) {
    UART0_Init();
    /* Show boot message before scheduler */
    UART0_SendString("Before scheduler\r\n");

    /* Create the UART mutex (binary semaphore used as mutex) */
    xUARTMutex = xSemaphoreCreateMutex();
    if (xUARTMutex == NULL) {
        UART0_SendString("Mutex creation failed!\r\n");
        for(;;); /* memory issue: adjust configTOTAL_HEAP_SIZE */
    }

    /* Create tasks with the SAME priority (1) so scheduling is fair */
    BaseType_t r;
    r = xTaskCreate(Task1, "T1", 256, NULL, 1, NULL);
    if (r != pdPASS) { UART0_SendString("T1 create fail\r\n"); for(;;); }

    r = xTaskCreate(Task2, "T2", 256, NULL, 1, NULL);
    if (r != pdPASS) { UART0_SendString("T2 create fail\r\n"); for(;;); }

    r = xTaskCreate(Task3, "T3", 256, NULL, 1, NULL);
    if (r != pdPASS) { UART0_SendString("T3 create fail\r\n"); for(;;); }

    /* Start the scheduler (this must be called) */
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running and the following line
       will never be reached. If it does reach here, memory for idle/Timer task
       was probably not available. */
    UART0_SendString("Scheduler failed to start!\r\n");
    for (;;);
    return 0;
}
