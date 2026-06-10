#ifndef __MQTT_SHARED_H__
#define __MQTT_SHARED_H__

#include <stdint.h>

#define MQTT_HANDLER_MAX_BROKER_LEN  128
#define MQTT_HANDLER_MAX_TOPIC_LEN   64
#define MQTT_HANDLER_MAX_MSG_LEN     256

#define MSG_ID_MQTT_START    10
#define MSG_ID_MQTT_PUBLISH  11
#define MSG_ID_MQTT_DATA     12
#define MSG_ID_MQTT_STOP     13
#define MSG_ID_SLAVE_READY   14

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

#endif