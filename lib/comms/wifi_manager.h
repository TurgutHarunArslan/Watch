#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#define WIFI_SSID_MAX_LEN 32
#define WIFI_PASS_MAX_LEN 64

#define WIFI_MAX_RETRY 5 
int retry_count;

#define WIFI_MANAGER_QUEUE_LEN 5

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_DISCONNECTED_BIT BIT1
#define WIFI_FAIL_BIT BIT2

extern EventGroupHandle_t wifi_event_group;

void wifi_manager_init(void);

void wifi_set_ssid_password(const char *ssid, const char *password);
#endif // WIFI_MANAGER_H
