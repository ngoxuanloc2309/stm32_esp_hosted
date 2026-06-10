#ifndef __SERVICE_MQTT_H
#define __SERVICE_MQTT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MQTT_MAX_TOPIC_LEN   64
#define MQTT_MAX_MSG_LEN     256
#define MQTT_MAX_BROKER_LEN  128

typedef void (*mqtt_msg_callback_t)(const char *topic, const char *msg, uint16_t msg_len);

int  service_mqtt_init(const char *broker_ip, uint16_t port, const char *client_id);
int  service_mqtt_connect(void);
int  service_mqtt_subscribe(const char *topic, mqtt_msg_callback_t cb);
int  service_mqtt_publish(const char *topic, const char *msg, uint16_t msg_len, uint8_t qos, uint8_t retain);
int  service_mqtt_is_connected(void);
void service_mqtt_deinit(void);

#ifdef __cplusplus
}
#endif

#endif