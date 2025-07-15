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
static QueueHandle_t wifi_queue;

static char stored_ssid[WIFI_SSID_MAX_LEN] = {0};
static char stored_password[WIFI_PASS_MAX_LEN] = {0};

static void wifi_ip_event_handler(void *arg, esp_event_base_t event_base,
                                  int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_STA_DISCONNECTED)
        {

            wifi_event_sta_disconnected_t *evt = (wifi_event_sta_disconnected_t *)event_data;

            ESP_LOGI(TAG, "Wi-Fi disconnected");
            xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);

            if (evt->reason == WIFI_REASON_AUTH_FAIL ||
                evt->reason == WIFI_REASON_NO_AP_FOUND)
            {
                ESP_LOGI(TAG, "Connection failed");
                xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
            }
            else
            {
                xEventGroupClearBits(wifi_event_group, WIFI_FAIL_BIT);
            }
        }
    }
    else if (event_base == IP_EVENT)
    {
        if (event_id == IP_EVENT_STA_GOT_IP)
        {
            ESP_LOGI(TAG, "Got IP");
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
            xEventGroupClearBits(wifi_event_group, WIFI_FAIL_BIT);
        }
    }
}

static void wifi_manager_task(void *pvParameters)
{
    wifi_cmd_t cmd;
    while (1)
    {
        if (xQueueReceive(wifi_queue, &cmd, portMAX_DELAY))
        {
            switch (cmd.cmd)
            {
            case WIFI_CMD_CONNECT:
                esp_wifi_set_mode(WIFI_MODE_STA);
                esp_wifi_start();
                esp_wifi_connect();
                break;
            case WIFI_CMD_DISCONNECT:
                esp_wifi_disconnect();
                esp_wifi_stop();
                break;
            case WIFI_CMD_SET_SSID_PASSWORD:
                strncpy(stored_ssid, cmd.data.connect_params.ssid, sizeof(stored_ssid));
                strncpy(stored_password, cmd.data.connect_params.password, sizeof(stored_password));
                break;
            default:
                break;
            }
        }
    }
}

void wifi_manager_init(void)
{
    wifi_event_group = xEventGroupCreate();
    wifi_queue = xQueueCreate(WIFI_MANAGER_QUEUE_LEN, sizeof(wifi_cmd_t));

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_ip_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_ip_event_handler, NULL);

    xTaskCreate(wifi_manager_task, "wifi_manager_task", 4096, NULL, 5, NULL);
}

BaseType_t wifi_manager_send_cmd(const wifi_cmd_t *cmd)
{
    if (!wifi_queue)
        return pdFAIL;
    return xQueueSend(wifi_queue, cmd, portMAX_DELAY);
}