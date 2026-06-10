#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

#define APP_WIFI_SSID        "Spotdog123"
#define APP_WIFI_PASSWORD    "123456789"
#define APP_TASK_STACK_SIZE  4096
#define MQTT_HOST            "broker.hivemq.com" // "broker.hivemq.com"
#define MQTT_PORT            1883
#define MQTT_TOPIC           "test/topic"
#define MQTT_CLIENT_ID       "esp32_stm32_client"
#define MQTT_USERNAME        "" // Optional, if your broker requires authentication
#define MQTT_PASSWORD        "" // Optional, if your broker requires authentication

#endif