/*service_mqtt.c*/
#include <string.h>
#include <stdio.h>
#include "cmsis_os.h"
#include "lwip/apps/mqtt.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"
#include "service_mqtt.h"

static mqtt_client_t        *s_client      = NULL;
static ip_addr_t             s_broker_ip;
static uint16_t              s_port         = 1883;
static char                  s_client_id[64];
static char                  s_broker_host[MQTT_MAX_BROKER_LEN];
static volatile uint8_t      s_connected    = 0;
static volatile uint8_t      s_dns_done     = 0;
static volatile uint8_t      s_dns_ok       = 0;
static mqtt_msg_callback_t   s_msg_cb       = NULL;
static char                  s_sub_topic[MQTT_MAX_TOPIC_LEN];

static void mqtt_connection_cb(mqtt_client_t *client, void *arg,
                                mqtt_connection_status_t status)
{
    (void)client;
    (void)arg;
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("MQTT connected\r\n");
        s_connected = 1;
    } else {
        printf("MQTT disconnected: %d\r\n", status);
        s_connected = 0;
    }
}

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
    (void)arg;
    (void)tot_len;
    strncpy(s_sub_topic, topic, MQTT_MAX_TOPIC_LEN - 1);
}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data,
                                   u16_t len, u8_t flags)
{
    (void)arg;
    if (s_msg_cb && len > 0) {
        char msg[MQTT_MAX_MSG_LEN];
        uint16_t copy_len = len < (MQTT_MAX_MSG_LEN - 1) ? len : (MQTT_MAX_MSG_LEN - 1);
        memcpy(msg, data, copy_len);
        msg[copy_len] = '\0';
        if (flags & MQTT_DATA_FLAG_LAST)
            s_msg_cb(s_sub_topic, msg, copy_len);
    }
}

static void mqtt_sub_request_cb(void *arg, err_t result)
{
    (void)arg;
    if (result == ERR_OK)
        printf("MQTT subscribe OK\r\n");
    else
        printf("MQTT subscribe failed: %d\r\n", result);
}

static void dns_found_cb(const char *name, const ip_addr_t *ipaddr, void *arg)
{
    (void)name;
    (void)arg;
    if (ipaddr) {
        s_broker_ip = *ipaddr;
        s_dns_ok    = 1;
        printf("DNS resolved: %s\r\n", ip4addr_ntoa(ip_2_ip4(ipaddr)));
    } else {
        printf("DNS resolve failed\r\n");
        s_dns_ok = 0;
    }
    s_dns_done = 1;
}

static int resolve_broker(void)
{
    s_dns_done = 0;
    s_dns_ok   = 0;

    err_t ret = dns_gethostbyname(s_broker_host, &s_broker_ip, dns_found_cb, NULL);
    if (ret == ERR_OK) {
        /* already in cache */
        printf("DNS cached: %s\r\n", ip4addr_ntoa(ip_2_ip4(&s_broker_ip)));
        return 0;
    }
    if (ret != ERR_INPROGRESS) {
        printf("dns_gethostbyname error: %d\r\n", ret);
        return -1;
    }

    /* wait for callback */
    uint32_t tick = osKernelSysTick();
    while (!s_dns_done) {
        if (osKernelSysTick() - tick > 10000) {
            printf("DNS timeout\r\n");
            return -1;
        }
        osDelay(100);
    }
    return s_dns_ok ? 0 : -1;
}

int service_mqtt_init(const char *broker_host, uint16_t port, const char *client_id)
{
    if (!broker_host || !client_id) return -1;

    strncpy(s_broker_host, broker_host, sizeof(s_broker_host) - 1);
    s_port = port;
    strncpy(s_client_id, client_id, sizeof(s_client_id) - 1);

    s_client = mqtt_client_new();
    if (!s_client) {
        printf("mqtt_client_new failed\r\n");
        return -1;
    }

    mqtt_set_inpub_callback(s_client,
                            mqtt_incoming_publish_cb,
                            mqtt_incoming_data_cb,
                            NULL);
    return 0;
}

int service_mqtt_connect(void)
{
    if (!s_client) return -1;

    if (resolve_broker() != 0) {
        printf("DNS resolve failed for %s\r\n", s_broker_host);
        return -1;
    }

    struct mqtt_connect_client_info_t ci;
    memset(&ci, 0, sizeof(ci));
    ci.client_id  = s_client_id;
    ci.keep_alive = 60;

    err_t ret = mqtt_client_connect(s_client,
                                    &s_broker_ip,
                                    s_port,
                                    mqtt_connection_cb,
                                    NULL,
                                    &ci);
    if (ret != ERR_OK) {
        printf("mqtt_client_connect failed: %d\r\n", ret);
        return -1;
    }
    return 0;
}

int service_mqtt_subscribe(const char *topic, mqtt_msg_callback_t cb)
{
    if (!s_client || !s_connected || !topic) return -1;
    s_msg_cb = cb;
    err_t ret = mqtt_subscribe(s_client, topic, 0, mqtt_sub_request_cb, NULL);
    return (ret == ERR_OK) ? 0 : -1;
}

int service_mqtt_publish(const char *topic, const char *msg,
                         uint16_t msg_len, uint8_t qos, uint8_t retain)
{
    if (!s_client || !s_connected || !topic || !msg) return -1;
    err_t ret = mqtt_publish(s_client, topic,
                             msg, msg_len,
                             qos, retain,
                             NULL, NULL);
    return (ret == ERR_OK) ? 0 : -1;
}

int service_mqtt_is_connected(void)
{
    return s_connected;
}

void service_mqtt_deinit(void)
{
    if (s_client) {
        mqtt_disconnect(s_client);
        mqtt_client_free(s_client);
        s_client = NULL;
    }
    s_connected = 0;
}