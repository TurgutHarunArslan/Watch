#include "wifi_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs.h"
#include <string.h>

static const char *TAG = "wifi_manager";
#define WIFI_NAMESPACE "wifi_cfg"

int retry_count = 0;

static EventGroupHandle_t wifi_event_group;
static QueueHandle_t wifi_queue;

static char stored_ssid[WIFI_SSID_MAX_LEN] = {0};
static char stored_password[WIFI_PASS_MAX_LEN] = {0};

static void save_credentials_from_wifi_config(const wifi_config_t *wifi_config)
{
    nvs_handle_t nvs_handle;
    if (nvs_open("wifi_cfg", NVS_READWRITE, &nvs_handle) == ESP_OK)
    {
        nvs_set_str(nvs_handle, "ssid", (const char *)wifi_config->sta.ssid);
        nvs_set_str(nvs_handle, "password", (const char *)wifi_config->sta.password);
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
        ESP_LOGI(TAG, "Saved Wi-Fi credentials from wifi_config to NVS");
    }
    else
    {
        ESP_LOGW(TAG, "Failed to open NVS for writing");
    }
}

static bool load_credentials_from_nvs(char *ssid_out, size_t ssid_len,
                                      char *pass_out, size_t pass_len)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(WIFI_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK)
        return false;

    err = nvs_get_str(nvs_handle, "ssid", ssid_out, &ssid_len);
    if (err != ESP_OK)
    {
        nvs_close(nvs_handle);
        return false;
    }

    err = nvs_get_str(nvs_handle, "password", pass_out, &pass_len);
    nvs_close(nvs_handle);
    return err == ESP_OK;
}

static void wifi_ip_event_handler(void *arg, esp_event_base_t event_base,
                                  int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            ESP_LOGI(TAG, "Wi-Fi disconnected");
            xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);

            if (retry_count < WIFI_MAX_RETRY)
            {
                retry_count++;
                ESP_LOGI(TAG, "Retrying Wi-Fi... (%d/%d)", retry_count, WIFI_MAX_RETRY);
                esp_wifi_connect();
            }
            else
            {
                xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
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
            retry_count = 0;
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
            wifi_config_t wifi_config = {0};
            strncpy((char *)wifi_config.sta.ssid, cmd.data.connect_params.ssid, sizeof(wifi_config.sta.ssid));
            strncpy((char *)wifi_config.sta.password, cmd.data.connect_params.password, sizeof(wifi_config.sta.password));

            esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
            esp_wifi_connect();

            save_credentials_from_wifi_config(&wifi_config);
            switch (cmd.cmd)
            {
            case WIFI_CMD_SET_SSID_PASSWORD:
                wifi_config_t wifi_config = {0};
                strncpy((char *)wifi_config.sta.ssid, cmd.data.connect_params.ssid, sizeof(wifi_config.sta.ssid));
                strncpy((char *)wifi_config.sta.password, cmd.data.connect_params.password, sizeof(wifi_config.sta.password));

                esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
                esp_wifi_connect();

                save_credentials_from_wifi_config(&wifi_config);
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

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

    if (load_credentials_from_nvs(stored_ssid, sizeof(stored_ssid),
                                  stored_password, sizeof(stored_password)))
    {
        ESP_LOGI(TAG, "Loaded Wi-Fi config from NVS: SSID='%s'", stored_ssid);

        wifi_config_t wifi_config = {0};
        strncpy((char *)wifi_config.sta.ssid, stored_ssid, sizeof(wifi_config.sta.ssid));
        strncpy((char *)wifi_config.sta.password, stored_password, sizeof(wifi_config.sta.password));

        esp_wifi_set_config(WIFI_IF_STA, &wifi_config);

        esp_wifi_connect();
    }
    else
    {
        ESP_LOGI(TAG, "No Wi-Fi credentials found in NVS");
    }

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_ip_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_ip_event_handler, NULL);

    xTaskCreate(wifi_manager_task, "wifi_queue_task", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "Wi-Fi initialized and modem sleep enabled");
}

BaseType_t wifi_manager_send_cmd(const wifi_cmd_t *cmd)
{
    if (!wifi_queue)
        return pdFAIL;
    return xQueueSend(wifi_queue, cmd, portMAX_DELAY);
}