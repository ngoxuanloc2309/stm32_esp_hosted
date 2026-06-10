#include "cmsis_os.h"
#include "transport_drv.h"
#include "ctrl_api.h"
#include "platform_wrapper.h"
#include "trace.h"
#include "app.h"
#include "app_config.h"
#include "mqtt_shared.h"
#include <string.h>
#include <stdio.h>

static osThreadId app_task_id = 0;
static volatile uint8_t transport_active = 0;
static volatile uint8_t wifi_connected   = 0;
static volatile uint8_t dhcp_got_ip = 0;
static volatile uint8_t slave_ready = 0;

static void app_task(void const *arg);
static void transport_event_handler(uint8_t event);
static int  wifi_event_handler(ctrl_cmd_t *event);
static int  mqtt_data_event_handler(ctrl_cmd_t *event);

static void transport_event_handler(uint8_t event)
{
    switch (event) {
        case TRANSPORT_ACTIVE:
            printf("Transport active\r\n");
            transport_active = 1;
            int ret = init_hosted_control_lib();
            printf("init_hosted_control_lib ret=%d\r\n", ret);
            break;
        default:
            break;
    }
}

static int wifi_event_handler(ctrl_cmd_t *event)
{
    if (!event) return -1;
    if (event->msg_id == CTRL_EVENT_STATION_CONNECTED_TO_AP) {
        printf("WiFi connected event!\r\n");
        wifi_connected = 1;
    } else if (event->msg_id == CTRL_EVENT_STATION_DISCONNECT_FROM_AP) {
        printf("WiFi disconnected event!\r\n");
        wifi_connected = 0;
        dhcp_got_ip = 0;
    } else if (event->msg_id == CTRL_EVENT_DHCP_DNS_STATUS) {
        printf("DHCP got IP!\r\n");
        dhcp_got_ip = 1;
    }
    return 0;
}

static int mqtt_data_event_handler(ctrl_cmd_t *event)
{
    if (!event) return -1;
    custom_rpc_unserialised_data_t *data = &event->u.custom_rpc_unserialised_data;
    if (data->custom_msg_id == MSG_ID_MQTT_DATA &&
        data->data && data->data_len >= sizeof(mqtt_incoming_t)) {
        mqtt_incoming_t *incoming = (mqtt_incoming_t *)data->data;
        printf("MQTT RX topic=[%s] msg=[%s]\r\n",
               incoming->topic, incoming->message);
    }
    if (data->custom_msg_id == MSG_ID_SLAVE_READY) {
        printf("Slave ready!\r\n");
        slave_ready = 1;
    }
    if (event->free_buffer_handle && event->free_buffer_func) {
        event->free_buffer_func(event->free_buffer_handle);
    }
    hosted_free(event);
    return 0;
}

static void app_task(void const *arg)
{
    ctrl_cmd_t *req  = NULL;
    ctrl_cmd_t *resp = NULL;

    while (!transport_active)
        osDelay(100);

    osDelay(2000);

    req = (ctrl_cmd_t *)hosted_calloc(1, sizeof(ctrl_cmd_t));
    if (!req) {
        printf("Failed to alloc\r\n");
        goto cleanup;
    }

    /* 1. Register event callbacks */
    set_event_callback(CTRL_EVENT_STATION_CONNECTED_TO_AP,
                       wifi_event_handler);
    set_event_callback(CTRL_EVENT_STATION_DISCONNECT_FROM_AP,
                       wifi_event_handler);
    set_event_callback(CTRL_EVENT_CUSTOM_RPC_UNSERIALISED_MSG,
                       mqtt_data_event_handler);
    set_event_callback(CTRL_EVENT_DHCP_DNS_STATUS, wifi_event_handler);

    /* 2. Get MAC */
    req->msg_type        = CTRL_REQ;
    req->msg_id          = CTRL_REQ_GET_MAC_ADDR;
    req->u.wifi_mac.mode = WIFI_MODE_STA;
    req->cmd_timeout_sec = DEFAULT_CTRL_RESP_TIMEOUT;
    resp = wifi_get_mac(req);
    if (!resp || resp->resp_event_status != SUCCESS) {
        printf("Failed to get MAC\r\n");
        goto cleanup;
    }
    printf("MAC: %s\r\n", resp->u.wifi_mac.mac);
    CLEANUP_CTRL_MSG(resp);

    /* 3. Set WiFi mode STA */
    memset(req, 0, sizeof(ctrl_cmd_t));
    req->msg_type         = CTRL_REQ;
    req->msg_id           = CTRL_REQ_SET_WIFI_MODE;
    req->u.wifi_mode.mode = WIFI_MODE_STA;
    req->cmd_timeout_sec  = DEFAULT_CTRL_RESP_TIMEOUT;
    resp = wifi_set_mode(req);
    if (!resp || resp->resp_event_status != SUCCESS) {
        printf("Failed to set WiFi mode\r\n");
        goto cleanup;
    }
    printf("WiFi mode STA OK\r\n");
    CLEANUP_CTRL_MSG(resp);

    /* 4. Connect AP */
    memset(req, 0, sizeof(ctrl_cmd_t));
    req->msg_type = CTRL_REQ;
    req->msg_id   = CTRL_REQ_CONNECT_AP;
    strncpy((char*)req->u.wifi_ap_config.ssid, APP_WIFI_SSID, SSID_LENGTH-1);
    strncpy((char*)req->u.wifi_ap_config.pwd,  APP_WIFI_PASSWORD, PASSWORD_LENGTH-1);
    req->u.wifi_ap_config.is_wpa3_supported = false;
    req->cmd_timeout_sec = DEFAULT_CTRL_RESP_CONNECT_AP_TIMEOUT;
    resp = wifi_connect_ap(req);
    if (!resp || resp->resp_event_status != SUCCESS) {
        printf("Failed to connect AP status=%d\r\n",
               resp ? resp->resp_event_status : -1);
        goto cleanup;
    }
    printf("Connect AP sent: %s\r\n", APP_WIFI_SSID);
    CLEANUP_CTRL_MSG(resp);

    /* 5. Wait WiFi connected event max 30s */
    printf("Waiting WiFi connected event...\r\n");
    int timeout = 0;
    while (!slave_ready && timeout < 300) {
        osDelay(100);
        timeout++;
    }
    if (!wifi_connected) {
        printf("WiFi connect timeout!\r\n");
        goto cleanup;
    }
    printf("Slave ready confirmed!\r\n");
    osDelay(1000);

    /* 6. Send MQTT start */
    mqtt_start_cmd_t mqtt_start = {0};
    snprintf(mqtt_start.broker_url, sizeof(mqtt_start.broker_url),
             "mqtt://%s:%d", MQTT_HOST, MQTT_PORT);
    strncpy(mqtt_start.topic, MQTT_TOPIC, sizeof(mqtt_start.topic)-1);
    printf("MQTT broker: %s\r\n", mqtt_start.broker_url);

    memset(req, 0, sizeof(ctrl_cmd_t));
    req->msg_type = CTRL_REQ;
    req->msg_id   = CTRL_REQ_CUSTOM_RPC_UNSERIALISED_MSG;
    req->u.custom_rpc_unserialised_data.custom_msg_id = MSG_ID_MQTT_START;
    req->u.custom_rpc_unserialised_data.data     = (uint8_t *)&mqtt_start;
    req->u.custom_rpc_unserialised_data.data_len = sizeof(mqtt_start_cmd_t);
    req->cmd_timeout_sec = DEFAULT_CTRL_RESP_TIMEOUT;
    resp = send_custom_rpc_unserialised_req_to_slave(req);
    if (!resp || resp->resp_event_status != SUCCESS) {
        printf("Failed to send MQTT start\r\n");
        goto cleanup;
    }
    printf("MQTT start sent!\r\n");
    CLEANUP_CTRL_MSG(resp);

    /* 7. Wait mqtt connect OK (WiFi connected + 2s delay ESP32) */
    osDelay(5000);

    /* 8. Publish test message */
    mqtt_publish_cmd_t mqtt_pub = {0};
    strncpy(mqtt_pub.topic, MQTT_TOPIC, sizeof(mqtt_pub.topic)-1);
    snprintf(mqtt_pub.message, sizeof(mqtt_pub.message),
             "Hello from STM32! id=%s", MQTT_CLIENT_ID);

    memset(req, 0, sizeof(ctrl_cmd_t));
    req->msg_type = CTRL_REQ;
    req->msg_id   = CTRL_REQ_CUSTOM_RPC_UNSERIALISED_MSG;
    req->u.custom_rpc_unserialised_data.custom_msg_id = MSG_ID_MQTT_PUBLISH;
    req->u.custom_rpc_unserialised_data.data     = (uint8_t *)&mqtt_pub;
    req->u.custom_rpc_unserialised_data.data_len = sizeof(mqtt_publish_cmd_t);
    req->cmd_timeout_sec = DEFAULT_CTRL_RESP_TIMEOUT;
    resp = send_custom_rpc_unserialised_req_to_slave(req);
    if (!resp || resp->resp_event_status != SUCCESS) {
        printf("Failed to publish\r\n");
        goto cleanup;
    }
    printf("MQTT publish sent!\r\n");
    CLEANUP_CTRL_MSG(resp);

    /* 9. Loop receive mqtt message */
    printf("Waiting for MQTT messages...\r\n");
    for (;;) {
        osDelay(1000);
    }

cleanup:
    CLEANUP_CTRL_MSG(resp);
    if (req) hosted_free(req);
    vTaskSuspend(NULL);

}

void app_main(void)
{
    printf("Starting STM32 app...\r\n");
    transport_init(transport_event_handler);
    printf("Transport inited\r\n");
    osThreadDef(app_thread, app_task, osPriorityNormal, 0, APP_TASK_STACK_SIZE);
    app_task_id = osThreadCreate(osThread(app_thread), NULL);
    assert(app_task_id);
}