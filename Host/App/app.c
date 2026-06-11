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

/* -----------------------------------------------------------------------
 * CTRL event callback — fired by esp-hosted ctrl lib when ESP32 slave
 * has obtained an IP from the router (NETWORK_SPLIT_ENABLED mode).
 * The callback runs in the ctrl-lib's RX context, so keep it short.
 * ----------------------------------------------------------------------- */
static int dhcp_dns_event_cb(ctrl_cmd_t *event)
{
    if (!event) return -1;

    dhcp_dns_status_t *p = &event->u.dhcp_dns_status;

    if (p->dhcp_up) {
        printf("ESP32 DHCP: IP=%s NM=%s GW=%s\r\n",
               p->dhcp_ip, p->dhcp_nm, p->dhcp_gw);
        wifi_netif_set_static_ip((const char *)p->dhcp_ip,
                                 (const char *)p->dhcp_nm,
                                 (const char *)p->dhcp_gw);
    } else {
        printf("ESP32 DHCP down\r\n");
    }

    CLEANUP_CTRL_MSG(event);
    return 0;
}

/* -----------------------------------------------------------------------
 * Transport layer event handler — called from SPI ISR context.
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

    /* Wait until SPI transport is up */
    while (!transport_active)
        osDelay(100);

    /* Give ESP32 a moment to finish its own boot sequence */
    osDelay(2000);

    req = (ctrl_cmd_t *)hosted_calloc(1, sizeof(ctrl_cmd_t));
    if (!req) {
        printf("Failed to alloc ctrl_cmd_t\r\n");
        goto cleanup;
    }

    /* 1. Get STA MAC — we set this on the lwIP netif later */
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
        sscanf(resp->u.wifi_mac.mac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
        wifi_netif_set_mac(mac);
    }
    CLEANUP_CTRL_MSG(resp);

    /* 2. Set WiFi mode → STA */
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

    /* 3. Register CTRL event callback BEFORE connecting.
     *    The ESP32 fires CTRL_EVENT_DHCP_DNS_STATUS once it has an IP.
     *    Our callback calls wifi_netif_set_static_ip() which sets s_netif_up. */
    if (set_event_callback(CTRL_EVENT_DHCP_DNS_STATUS, dhcp_dns_event_cb) != CALLBACK_SET_SUCCESS) {
        printf("Failed to register DHCP event callback\r\n");
        goto cleanup;
    }
    printf("DHCP event callback registered\r\n");

    /* 4. Init the SPI RX queue and lwIP netif (no dhcp_start inside) */
    netdev_rx_queue_init();
    if (wifi_netif_init() != 0) {
        printf("wifi_netif_init failed\r\n");
        goto cleanup;
    }
    printf("lwIP netif initialized\r\n");

    /* 5. Connect to AP — ESP32 will DHCP and fire the event */
    memset(req, 0, sizeof(ctrl_cmd_t));
    req->msg_type = CTRL_REQ;
    req->msg_id   = CTRL_REQ_CONNECT_AP;
    strncpy((char *)req->u.wifi_ap_config.ssid, APP_WIFI_SSID,     SSID_LENGTH     - 1);
    strncpy((char *)req->u.wifi_ap_config.pwd,  APP_WIFI_PASSWORD, PASSWORD_LENGTH - 1);
    req->u.wifi_ap_config.is_wpa3_supported = false;
    req->cmd_timeout_sec = DEFAULT_CTRL_RESP_CONNECT_AP_TIMEOUT;
    resp = wifi_connect_ap(req);
    if (!resp || resp->resp_event_status != SUCCESS) {
        printf("Failed to connect AP (status=%d)\r\n",
               resp ? resp->resp_event_status : -1);
        goto cleanup;
    }
    printf("Connecting to AP: %s\r\n", APP_WIFI_SSID);
    CLEANUP_CTRL_MSG(resp);

    /* 6. Wait for dhcp_dns_event_cb to set the IP (up to 30 s) */
    printf("Waiting for IP from ESP32...\r\n");
    for (int timeout = 0; timeout < 300 && !wifi_netif_is_up(); timeout++)
        osDelay(100);

    if (!wifi_netif_is_up()) {
        printf("IP timeout — ESP32 did not report DHCP result\r\n");
        goto cleanup;
    }
    printf("Network up!\r\n");

    /* 7. Start MQTT */
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