#include <string.h>
#include "mqtt_handler.h"
#include "mqtt_shared.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "slave_control.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

static const char *TAG = "mqtt_handler";
static esp_mqtt_client_handle_t s_client = NULL;
static char s_topic[MQTT_HANDLER_MAX_TOPIC_LEN] = {0};
static char s_broker_url[MQTT_HANDLER_MAX_BROKER_LEN] = {0};
static uint8_t s_mqtt_started = 0;

static EventGroupHandle_t s_wifi_event_group = NULL;
#define WIFI_CONNECTED_BIT BIT0

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        ESP_LOGI(TAG, "WiFi STA connected event");
        if (s_wifi_event_group)
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                                int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT connected to %s", s_broker_url);
        if (s_topic[0]) {
            esp_mqtt_client_subscribe(s_client, s_topic, 0);
            ESP_LOGI(TAG, "Subscribed to %s", s_topic);
        }
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT disconnected");
        break;

    case MQTT_EVENT_DATA: {
        ESP_LOGI(TAG, "MQTT RX topic=%.*s msg=%.*s",
            event->topic_len, event->topic,
            event->data_len, event->data);

        mqtt_incoming_t incoming = {0};
        int tlen = event->topic_len < (int)(sizeof(incoming.topic) - 1)
                   ? event->topic_len : (int)(sizeof(incoming.topic) - 1);
        int mlen = event->data_len < (int)(sizeof(incoming.message) - 1)
                   ? event->data_len : (int)(sizeof(incoming.message) - 1);
        memcpy(incoming.topic, event->topic, tlen);
        memcpy(incoming.message, event->data, mlen);

        custom_rpc_unserialised_data_t evt = {0};
        evt.custom_msg_id = MSG_ID_MQTT_DATA;
        evt.data = (uint8_t *)&incoming;
        evt.data_len = sizeof(mqtt_incoming_t);
        evt.free_func = NULL;
        send_custom_rpc_unserialised_event(&evt);
        break;
    }

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT error type=%d",
                 event->error_handle->error_type);
        break;

    default:
        break;
    }
}

static void mqtt_start_task(void *pvParam)
{
    ESP_LOGI(TAG, "mqtt_start_task: broker=[%s] topic=[%s]",
             s_broker_url, s_topic);

    /* Kiểm tra broker_url hợp lệ */
    if (strlen(s_broker_url) == 0) {
        ESP_LOGE(TAG, "broker_url is empty!");
        s_mqtt_started = 0;
        vTaskDelete(NULL);
        return;
    }

    /* Tạo event group */
    s_wifi_event_group = xEventGroupCreate();
    if (!s_wifi_event_group) {
        ESP_LOGE(TAG, "Failed to create event group");
        s_mqtt_started = 0;
        vTaskDelete(NULL);
        return;
    }

    /* Register wifi event */
    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED,
                                wifi_event_handler, NULL);

    /* Check nếu WiFi đã connected rồi */
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        ESP_LOGI(TAG, "WiFi already connected SSID=%s", ap_info.ssid);
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    } else {
        ESP_LOGI(TAG, "Waiting for WiFi...");
    }

    /* Đợi WiFi connected tối đa 30s */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                            WIFI_CONNECTED_BIT,
                                            pdFALSE, pdTRUE,
                                            pdMS_TO_TICKS(30000));

    esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED,
                                  wifi_event_handler);
    vEventGroupDelete(s_wifi_event_group);
    s_wifi_event_group = NULL;

    if (!(bits & WIFI_CONNECTED_BIT)) {
        ESP_LOGE(TAG, "Timeout waiting for WiFi");
        s_mqtt_started = 0;
        vTaskDelete(NULL);
        return;
    }

    /* Delay để network stack ổn định */
    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI(TAG, "Starting MQTT client...");

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = s_broker_url,
    };

    s_client = esp_mqtt_client_init(&mqtt_cfg);
    if (!s_client) {
        ESP_LOGE(TAG, "Failed to init MQTT client");
        s_mqtt_started = 0;
        vTaskDelete(NULL);
        return;
    }

    esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID,
                                   mqtt_event_handler, NULL);

    esp_err_t err = esp_mqtt_client_start(s_client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start MQTT client: %d", err);
        esp_mqtt_client_destroy(s_client);
        s_client = NULL;
        s_mqtt_started = 0;
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "MQTT client started OK");
    vTaskDelete(NULL);
}

esp_err_t mqtt_handler_start(const mqtt_start_cmd_t *cmd)
{
    if (s_mqtt_started) {
        ESP_LOGW(TAG, "MQTT already started");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "mqtt_handler_start: broker=[%s] topic=[%s]",
             cmd->broker_url, cmd->topic);

    /* Copy vào static buffer */
    memset(s_broker_url, 0, sizeof(s_broker_url));
    memset(s_topic, 0, sizeof(s_topic));
    memcpy(s_broker_url, cmd->broker_url,
           strnlen(cmd->broker_url, sizeof(s_broker_url) - 1));
    memcpy(s_topic, cmd->topic,
           strnlen(cmd->topic, sizeof(s_topic) - 1));

    ESP_LOGI(TAG, "Copied: broker=[%s] topic=[%s]", s_broker_url, s_topic);

    s_mqtt_started = 1;
    xTaskCreate(mqtt_start_task, "mqtt_start", 8192, NULL, 5, NULL);
    return ESP_OK;
}

esp_err_t mqtt_handler_publish(const mqtt_publish_cmd_t *cmd)
{
    if (!s_client) {
        ESP_LOGE(TAG, "MQTT not started");
        return ESP_FAIL;
    }
    int msg_id = esp_mqtt_client_publish(s_client, cmd->topic,
                                          cmd->message, 0, 0, 0);
    ESP_LOGI(TAG, "Published topic=%s msg=%s msg_id=%d",
             cmd->topic, cmd->message, msg_id);
    return ESP_OK;
}

esp_err_t mqtt_handler_stop(void)
{
    s_mqtt_started = 0;
    if (s_client) {
        esp_mqtt_client_stop(s_client);
        esp_mqtt_client_destroy(s_client);
        s_client = NULL;
    }
    return ESP_OK;
}