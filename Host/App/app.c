/*app.c*/
#include <string.h>
#include <stdio.h>
#include "lwip/pbuf.h"
#include "cmsis_os.h"
#include "transport_drv.h"
#include "ctrl_api.h"
#include "platform_wrapper.h"
#include "trace.h"
#include "app.h"
#include "app_config.h"
#include "wifi_netif.h"
#include "user_mqtt.h"
#include "netdev_ext.h"

static osThreadId        app_task_id      = 0;
static volatile uint8_t  transport_active = 0;

static void app_task(void const *arg);
static void transport_event_handler(uint8_t event);

static void transport_event_handler(uint8_t event)
{
    if (event == TRANSPORT_ACTIVE) {
        printf("Transport active\r\n");
        transport_active = 1;
        int ret = init_hosted_control_lib();
        printf("init_hosted_control_lib ret=%d\r\n", ret);
    }
}

static void app_task(void const *arg)
{
    (void)arg;
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

    /* 1. Get MAC */
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

    /* parse MAC string xx:xx:xx:xx:xx:xx and set on netif */
    {
        uint8_t mac[6];
        sscanf(resp->u.wifi_mac.mac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
        wifi_netif_set_mac(mac);
    }
    CLEANUP_CTRL_MSG(resp);

    /* 2. Set WiFi mode STA */
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

    /* 3. Init lwIP netif before connecting — DHCP will start immediately */
    netdev_rx_queue_init();
    printf("netdev rx queue inited\r\n");

    if (wifi_netif_init() != 0) {
        printf("wifi_netif_init failed\r\n");
        goto cleanup;
    }
    printf("lwIP netif initialized\r\n");

    /* 4. Connect AP */
    memset(req, 0, sizeof(ctrl_cmd_t));
    req->msg_type = CTRL_REQ;
    req->msg_id   = CTRL_REQ_CONNECT_AP;
    strncpy((char *)req->u.wifi_ap_config.ssid, APP_WIFI_SSID, SSID_LENGTH - 1);
    strncpy((char *)req->u.wifi_ap_config.pwd,  APP_WIFI_PASSWORD, PASSWORD_LENGTH - 1);
    req->u.wifi_ap_config.is_wpa3_supported = false;
    req->cmd_timeout_sec = DEFAULT_CTRL_RESP_CONNECT_AP_TIMEOUT;
    resp = wifi_connect_ap(req);
    if (!resp || resp->resp_event_status != SUCCESS) {
        printf("Failed to connect AP status=%d\r\n",
               resp ? resp->resp_event_status : -1);
        goto cleanup;
    }
    printf("Connecting to AP: %s\r\n", APP_WIFI_SSID);
    CLEANUP_CTRL_MSG(resp);

    /* 5. Wait for DHCP IP */
    printf("Waiting for DHCP IP...\r\n");
    int timeout = 0;
    while (!wifi_netif_is_up() && timeout < 300) {
        osDelay(100);
        timeout++;
    }
    if (!wifi_netif_is_up()) {
        printf("DHCP timeout!\r\n");
        goto cleanup;
    }
    printf("Network up!\r\n");

    /* 6. Start MQTT */
    user_mqtt_start();

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