/*user_mqtt.c*/
#include <stdio.h>
#include <string.h>
#include "cmsis_os.h"
#include "service_mqtt.h"
#include "user_mqtt.h"
#include "app_config.h"

#define MQTT_CONNECT_TIMEOUT_MS  10000
#define MQTT_TASK_STACK_SIZE     1024

static void mqtt_task(void const *arg)
{
    (void)arg;

    /* init MQTT service */
    if (service_mqtt_init(MQTT_HOST, MQTT_PORT, MQTT_CLIENT_ID) != 0) {
        printf("MQTT init failed\r\n");
        vTaskSuspend(NULL);
        return;
    }

    /* connect to broker */
    printf("Connecting to MQTT broker %s:%d...\r\n", MQTT_HOST, MQTT_PORT);
    if (service_mqtt_connect() != 0) {
        printf("MQTT connect call failed\r\n");
        vTaskSuspend(NULL);
        return;
    }

    /* wait for connection */
    uint32_t tick = osKernelSysTick();
    while (!service_mqtt_is_connected()) {
        if (osKernelSysTick() - tick > MQTT_CONNECT_TIMEOUT_MS) {
            printf("MQTT connect timeout\r\n");
            vTaskSuspend(NULL);
            return;
        }
        osDelay(100);
    }
    printf("MQTT connected!\r\n");

    /* subscribe */
    service_mqtt_subscribe(MQTT_TOPIC, user_mqtt_on_message);
    printf("Subscribed to %s\r\n", MQTT_TOPIC);

    /* publish hello */
    char hello[128];
    snprintf(hello, sizeof(hello), "Hello from STM32! id=%s", MQTT_CLIENT_ID);
    service_mqtt_publish(MQTT_TOPIC, hello, strlen(hello), 0, 0);
    printf("Published: %s\r\n", hello);

    /* loop */
    for (;;) {
        osDelay(1000);
    }
}

void user_mqtt_on_message(const char *topic, const char *msg, uint16_t msg_len)
{
    (void)msg_len;
    printf("MQTT RX topic=[%s] msg=[%s]\r\n", topic, msg);
}

void user_mqtt_start(void)
{
    osThreadDef(mqtt_thread, mqtt_task, osPriorityNormal, 0, MQTT_TASK_STACK_SIZE);
    osThreadCreate(osThread(mqtt_thread), NULL);
}