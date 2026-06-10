#include <string.h>
#include "mqtt_handler.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "slave_control.h"

static const char *TAG = "mqtt_handler";

/* MSG_ID phải match với STM32 side */
#define MSG_ID_MQTT_DATA  12

static esp_mqtt_client_handle_t s_client = NULL;
static char s_topic[MQTT_HANDLER_MAX_TOPIC_LEN] = {0};

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                                int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT connected");
        if (s_topic[0]) {
            esp_mqtt_client_subscribe(s_client, s_topic, 0);
            ESP_LOGI(TAG, "Subscribed to %s", s_topic);
        }
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT disconnected");
        break;

    case MQTT_EVENT_DATA: {
        ESP_LOGI(TAG, "MQTT data received topic=%.*s msg=%.*s",
            event->topic_len, event->topic,
            event->data_len, event->data);

        /* Forward về STM32 qua custom RPC event */
        mqtt_incoming_t incoming = {0};
        int tlen = event->topic_len < (int)(sizeof(incoming.topic)-1)
                   ? event->topic_len : (int)(sizeof(incoming.topic)-1);
        int mlen = event->data_len < (int)(sizeof(incoming.message)-1)
                   ? event->data_len : (int)(sizeof(incoming.message)-1);
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
        ESP_LOGE(TAG, "MQTT error");
        break;

    default:
        break;
    }
}

esp_err_t mqtt_handler_start(const mqtt_start_cmd_t *cmd)
{
    if (s_client) {
        ESP_LOGW(TAG, "MQTT already started, stopping first");
        mqtt_handler_stop();
    }

    snprintf(s_topic, sizeof(s_topic), "%s", cmd->topic);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = cmd->broker_url,
    };

    s_client = esp_mqtt_client_init(&mqtt_cfg);
    if (!s_client) {
        ESP_LOGE(TAG, "Failed to init MQTT client");
        return ESP_FAIL;
    }

    esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID,
                                   mqtt_event_handler, NULL);
    esp_mqtt_client_start(s_client);
    ESP_LOGI(TAG, "MQTT started, broker=%s topic=%s",
             cmd->broker_url, cmd->topic);
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
    ESP_LOGI(TAG, "Published to %s: %s (msg_id=%d)",
             cmd->topic, cmd->message, msg_id);
    return ESP_OK;
}

esp_err_t mqtt_handler_stop(void)
{
    if (s_client) {
        esp_mqtt_client_stop(s_client);
        esp_mqtt_client_destroy(s_client);
        s_client = NULL;
    }
    return ESP_OK;
}