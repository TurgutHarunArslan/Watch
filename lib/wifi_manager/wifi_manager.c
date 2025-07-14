#include "wifi_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "wifi_manager";

static EventGroupHandle_t wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define MAX_RETRY_COUNT 5

static int retry_num = 0;

static char stored_ssid[WIFI_SSID_MAX_LEN] = {0};
static char stored_password[WIFI_PASS_MAX_LEN] = {0};

// conncet to wifi when necesarry and disconncet when not 

void wifi_manager_init(void)
{
    // setup
}

