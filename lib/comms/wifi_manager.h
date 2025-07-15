#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#define WIFI_SSID_MAX_LEN 32
#define WIFI_PASS_MAX_LEN 64

#define WIFI_MANAGER_QUEUE_LEN 5

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_DISCONNECTED_BIT BIT1
#define WIFI_FAIL_BIT BIT2

extern EventGroupHandle_t wifi_event_group;

typedef enum
{
    WIFI_CMD_CONNECT,
    WIFI_CMD_DISCONNECT,
    WIFI_CMD_SET_SSID_PASSWORD,
} wifi_cmd_type_t;

typedef struct
{
    wifi_cmd_type_t cmd;

    union
    {
        struct
        {
            char ssid[32];
            char password[64];
        } connect_params;

    } data;

} wifi_cmd_t;

void wifi_manager_init(void);

BaseType_t wifi_manager_send_cmd(wifi_cmd_t cmd);

#endif // WIFI_MANAGER_H
