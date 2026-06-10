#ifndef __USER_MQTT_H
#define __USER_MQTT_H

#ifdef __cplusplus
extern "C" {
#endif

void user_mqtt_start(void);
void user_mqtt_on_message(const char *topic, const char *msg, uint16_t msg_len);

#ifdef __cplusplus
}
#endif

#endif