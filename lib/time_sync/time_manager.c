#include "esp_sntp.h"
#include "wifi_manager.h"
#include "esp_log.h"
static const char *TAG = "wifi_manager";

void time_sync_callback(struct timeval *tv)
{
    // set rtc time from here
    ESP_LOGI(TAG, "Time synchronized");
}

void time_manager_init(void)
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_callback);

    xTaskCreate(time_manager_ntp_task, "ntp_init_task", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "Time Manager Initilized");
}

void time_manager_ntp_task(void *pvParameters)
{
    EventBits_t bits = xEventGroupWaitBits(
        wifi_event_group,
        WIFI_CONNECTED_BIT,
        pdFALSE, // Do NOT clear bits on exit
        pdFALSE, // Wait for any bit
        pdMS_TO_TICKS(60000));

    if (bits & WIFI_CONNECTED_BIT)
    {
        sntp_init();
    }
}