#ifndef __MQTT_HANDLER_H__
#define __MQTT_HANDLER_H__

#include "esp_err.h"
#include "../add_on_stm32/mqtt_shared.h"

esp_err_t mqtt_handler_start(const mqtt_start_cmd_t *cmd);
esp_err_t mqtt_handler_publish(const mqtt_publish_cmd_t *cmd);
esp_err_t mqtt_handler_stop(void);

#endif