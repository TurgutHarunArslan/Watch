#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void print_task(void *pvParameter)
{
    int count = 0;
    while (1) {
        printf("Task running, count = %d\n", count++);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1 second
    }
}

void app_main(void)
{
    xTaskCreate(
        print_task,     // Task function
        "Print Task",   // Name of the task (for debugging)
        2048,           // Stack size in words
        NULL,           // Task input parameter
        5,              // Priority
        NULL            // Task handle
    );
}