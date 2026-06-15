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
static volatile uint8_t  sta_connected    = 0;

static void app_task(void const *arg);
static void transport_event_handler(uint8_t event);

/* -----------------------------------------------------------------------
 * CTRL event callback - fired when ESP32 slave has associated with the AP.
 * NOTE: with NETWORK_SPLIT_ENABLED, slave may still run its own DHCP.
 * We don't rely on slave's IP - the STM32 runs its own lwIP/DHCP.
 * ----------------------------------------------------------------------- */
static int wifi_connected_cb(ctrl_cmd_t *event)
{
    if (event && event->msg_id == CTRL_EVENT_STATION_CONNECTED_TO_AP) {
        printf("STA connected event from ESP32\r\n");
        sta_connected = 1;
    }
    CLEANUP_CTRL_MSG(event);
    return 0;
}

/* -----------------------------------------------------------------------
 * Transport layer event handler - called from SPI ISR context.
 * Only set the flag here; do real work in app_task.
 * ----------------------------------------------------------------------- */
static void transport_event_handler(uint8_t event)
{
    if (event == TRANSPORT_ACTIVE) {
        printf("Transport active\r\n");
        transport_active = 1;
        int ret = init_hosted_control_lib();
        printf("init_hosted_control_lib ret=%d\r\n", ret);
    }
}

/* -----------------------------------------------------------------------
 * Main application task
 * ----------------------------------------------------------------------- */
static void app_task(void const *arg)
{
    (void)arg;
    ctrl_cmd_t *req  = NULL;
    ctrl_cmd_t *resp = NULL;

    while (!transport_active)
        osDelay(100);

    osDelay(2000);

    /* Init RX queue early so packets are not dropped */
    netdev_rx_queue_init();

    req = (ctrl_cmd_t *)hosted_calloc(1, sizeof(ctrl_cmd_t));
    if (!req) {
        printf("Failed to alloc ctrl_cmd_t\r\n");
        goto cleanup;
    }

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
    {
        uint8_t mac[6];
        const char *s = resp->u.wifi_mac.mac;
        for (int i = 0; i < 6; i++) {
            mac[i] = (uint8_t)strtol(s, NULL, 16);
            s += 3;
        }
        wifi_netif_set_mac(mac);
    }
    CLEANUP_CTRL_MSG(resp);

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

    if (set_event_callback(CTRL_EVENT_STATION_CONNECTED_TO_AP, wifi_connected_cb) != CALLBACK_SET_SUCCESS) {
        printf("Failed to register STA connected callback\r\n");
        goto cleanup;
    }

    /* Init lwIP netif before connecting so input task is ready */
    if (wifi_netif_init() != 0) {
        printf("wifi_netif_init failed\r\n");
        goto cleanup;
    }
    printf("lwIP netif initialized\r\n");

    memset(req, 0, sizeof(ctrl_cmd_t));
    req->msg_type = CTRL_REQ;
    req->msg_id   = CTRL_REQ_CONNECT_AP;
    strncpy((char *)req->u.wifi_ap_config.ssid, APP_WIFI_SSID,     SSID_LENGTH     - 1);
    strncpy((char *)req->u.wifi_ap_config.pwd,  APP_WIFI_PASSWORD, PASSWORD_LENGTH - 1);
    req->u.wifi_ap_config.is_wpa3_supported = false;
    req->cmd_timeout_sec = DEFAULT_CTRL_RESP_CONNECT_AP_TIMEOUT;
    resp = wifi_connect_ap(req);
    if (!resp || resp->resp_event_status != SUCCESS) {
        printf("Failed to connect AP (status=%ld)\r\n",
               resp ? (long)resp->resp_event_status : -1L);
        goto cleanup;
    }
    printf("Connecting to AP: %s\r\n", APP_WIFI_SSID);
    CLEANUP_CTRL_MSG(resp);

    printf("Waiting for DHCP IP...\r\n");
    for (int timeout = 0; timeout < 300 && !wifi_netif_is_up(); timeout++)
        osDelay(100);

    if (!wifi_netif_is_up()) {
        printf("DHCP timeout\r\n");
        goto cleanup;
    }
    printf("Network up!\r\n");

    user_mqtt_start();

cleanup:
    CLEANUP_CTRL_MSG(resp);
    if (req) hosted_free(req);
    vTaskSuspend(NULL);
}

/* -----------------------------------------------------------------------
 * Entry point called from main() / freertos.c
 * ----------------------------------------------------------------------- */
void app_main(void)
{
    printf("Starting STM32 app...\r\n");
    transport_init(transport_event_handler);
    printf("Transport inited\r\n");
    osThreadDef(app_thread, app_task, osPriorityNormal, 0, APP_TASK_STACK_SIZE);
    app_task_id = osThreadCreate(osThread(app_thread), NULL);
    assert(app_task_id);
}