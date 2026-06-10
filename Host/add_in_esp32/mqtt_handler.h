#ifndef __MQTT_HANDLER_H__
#define __MQTT_HANDLER_H__

#include <stdint.h>
#include "esp_err.h"

#define MQTT_HANDLER_MAX_BROKER_LEN  128
#define MQTT_HANDLER_MAX_TOPIC_LEN   64
#define MQTT_HANDLER_MAX_MSG_LEN     256

typedef struct {
    char broker_url[MQTT_HANDLER_MAX_BROKER_LEN];
    char topic[MQTT_HANDLER_MAX_TOPIC_LEN];
} mqtt_start_cmd_t;

typedef struct {
    char topic[MQTT_HANDLER_MAX_TOPIC_LEN];
    char message[MQTT_HANDLER_MAX_MSG_LEN];
} mqtt_publish_cmd_t;

typedef struct {
    char topic[MQTT_HANDLER_MAX_TOPIC_LEN];
    char message[MQTT_HANDLER_MAX_MSG_LEN];
} mqtt_incoming_t;

esp_err_t mqtt_handler_start(const mqtt_start_cmd_t *cmd);
esp_err_t mqtt_handler_publish(const mqtt_publish_cmd_t *cmd);
esp_err_t mqtt_handler_stop(void);

#endif