/*wifi_netif.c*/
#include <string.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/etharp.h"
#include "lwip/timeouts.h"
#include "lwip/ip4_addr.h"
#include "netif/ethernet.h"
#include "wifi_netif.h"
#include "netdev_ext.h"
#include "transport_drv.h"

#define WIFI_NETIF_MTU                1500
#define WIFI_NETIF_INPUT_TASK_STACK   1024
#define WIFI_NETIF_INPUT_TASK_PRIO    osPriorityNormal
#define WIFI_NETIF_TIMEOUT_TASK_STACK 512
#define WIFI_NETIF_TIMEOUT_TASK_PRIO  osPriorityLow

struct netif wifi_netif;

static uint8_t          s_mac[6]   = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
static volatile uint8_t s_netif_up = 0;

/* ---------- low-level output: copy pbuf chain → send over SPI ---------- */
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    (void)netif;

    uint8_t *buf = malloc(p->tot_len);
    if (!buf) return ERR_MEM;

    uint16_t offset = 0;
    for (struct pbuf *q = p; q != NULL; q = q->next) {
        memcpy(buf + offset, q->payload, q->len);
        offset += q->len;
    }

    int ret = send_to_slave(ESP_STA_IF, 0, buf, p->tot_len);
    if (ret != 0) {
        free(buf);
        return ERR_IF;
    }
    return ERR_OK;
}

/* ---------- netif init callback -------------------------------------- */
static err_t wifi_netif_low_init(struct netif *netif)
{
    netif->hwaddr_len = ETH_HWADDR_LEN;
    memcpy(netif->hwaddr, s_mac, ETH_HWADDR_LEN);
    netif->mtu        = WIFI_NETIF_MTU;
    netif->flags      = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
    netif->output     = etharp_output;
    netif->linkoutput = low_level_output;
    netif->name[0]    = 'w';
    netif->name[1]    = '0';
    return ERR_OK;
}

/* ---------- status callback: fires when IP is set ------------------- */
static void netif_status_callback(struct netif *netif)
{
    if (netif_is_up(netif) && !ip4_addr_isany_val(*netif_ip4_addr(netif))) {
        printf("IP:  %s\r\n", ip4addr_ntoa(netif_ip4_addr(netif)));
        printf("GW:  %s\r\n", ip4addr_ntoa(netif_ip4_gw(netif)));
        printf("NM:  %s\r\n", ip4addr_ntoa(netif_ip4_netmask(netif)));
        s_netif_up = 1;
    } else {
        s_netif_up = 0;
    }
}

/* ---------- RX task: dequeue frames from SPI driver → lwIP ---------- */
static void wifi_netif_input_task(void const *arg)
{
    (void)arg;
    printf("wifi_input_task started\r\n");

    QueueHandle_t q = netdev_get_rx_queue();
    if (!q) {
        printf("ERROR: rx queue is NULL!\r\n");
        vTaskSuspend(NULL);
        return;
    }

    struct pbuf *raw = NULL;
    for (;;) {
        if (xQueueReceive(q, &raw, portMAX_DELAY) != pdTRUE)
            continue;

        struct pbuf *p = pbuf_alloc(PBUF_RAW, raw->len, PBUF_POOL);
        if (!p) {
            free(raw->payload);
            free(raw);
            continue;
        }

        memcpy(p->payload, raw->payload, raw->len);
        free(raw->payload);
        free(raw);

        if (wifi_netif.input(p, &wifi_netif) != ERR_OK)
            pbuf_free(p);
    }
}

/* ---------- timer task: drive lwIP internal timeouts ---------------- */
static void wifi_netif_timeout_task(void const *arg)
{
    (void)arg;
    for (;;) {
        sys_check_timeouts();
        osDelay(10);
    }
}

/* ---------- public API ---------------------------------------------- */

void wifi_netif_set_mac(uint8_t *mac)
{
    if (mac) memcpy(s_mac, mac, 6);
}

int wifi_netif_is_up(void)
{
    return s_netif_up;
}

/**
 * wifi_netif_set_static_ip
 *
 * Called from the CTRL_EVENT_DHCP_DNS_STATUS callback in app.c after the
 * ESP32 slave has obtained an IP from the router.  Sets the lwIP netif
 * address directly — no DHCP needed on the STM32 side.
 *
 * @param ip  dotted-decimal string, e.g. "10.43.137.37"
 * @param nm  dotted-decimal string, e.g. "255.255.255.0"
 * @param gw  dotted-decimal string, e.g. "10.43.137.137"
 */
void wifi_netif_set_static_ip(const char *ip, const char *nm, const char *gw)
{
    ip4_addr_t ipaddr, netmask, gateway;

    if (!ip4addr_aton(ip, &ipaddr)  ||
        !ip4addr_aton(nm, &netmask) ||
        !ip4addr_aton(gw, &gateway)) {
        printf("wifi_netif_set_static_ip: invalid address string\r\n");
        return;
    }

    /* netif_set_addr triggers netif_status_callback, which sets s_netif_up */
    netif_set_addr(&wifi_netif, &ipaddr, &netmask, &gateway);
}

int wifi_netif_init(void)
{
    printf("lwip initializing...\r\n");
    lwip_init();

    ip4_addr_t ipaddr, netmask, gw;
    ip4_addr_set_zero(&ipaddr);
    ip4_addr_set_zero(&netmask);
    ip4_addr_set_zero(&gw);

    if (!netif_add(&wifi_netif, &ipaddr, &netmask, &gw,
                   NULL, wifi_netif_low_init, ethernet_input)) {
        printf("netif_add failed\r\n");
        return -1;
    }

    netif_set_default(&wifi_netif);
    netif_set_status_callback(&wifi_netif, netif_status_callback);
    netif_set_up(&wifi_netif);
    netif_set_link_up(&wifi_netif);
    /* NOTE: no dhcp_start() — IP is assigned by wifi_netif_set_static_ip()
     *       once the ESP32 slave reports its DHCP result via ctrl event.   */

    osThreadDef(wifi_input, wifi_netif_input_task,
                WIFI_NETIF_INPUT_TASK_PRIO, 0, WIFI_NETIF_INPUT_TASK_STACK);
    osThreadCreate(osThread(wifi_input), NULL);

    osThreadDef(lwip_timeout, wifi_netif_timeout_task,
                WIFI_NETIF_TIMEOUT_TASK_PRIO, 0, WIFI_NETIF_TIMEOUT_TASK_STACK);
    osThreadCreate(osThread(lwip_timeout), NULL);

    printf("lwip netif ready (waiting for IP from ESP32)\r\n");
    return 0;
}