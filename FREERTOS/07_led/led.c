#include <LPC214x.h>
#include "FreeRTOS.h"
#include "task.h"
#define LED_PIN1 (1 << 8) 
#define LED_PIN2 (1 << 9) 


void led_task(void *pvParams)
{
    IO0DIR |= LED_PIN1|LED_PIN2;  

    while (1)
    {
        IO0SET = LED_PIN1; 
        vTaskDelay(pdMS_TO_TICKS(500));

        IO0CLR = LED_PIN1; 
        vTaskDelay(pdMS_TO_TICKS(500));
			 IO0SET = LED_PIN2; 
        vTaskDelay(pdMS_TO_TICKS(500));

        IO0CLR = LED_PIN2; 
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(void)
{
    xTaskCreate(led_task, "LED", 128, NULL, 1, NULL);
    vTaskStartScheduler();

    while (1); 
}
