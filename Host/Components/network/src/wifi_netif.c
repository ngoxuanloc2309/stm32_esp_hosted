/*wifi_netif.c*/
#include <string.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/timeouts.h"
#include "netif/ethernet.h"
#include "wifi_netif.h"
#include "netdev_ext.h"
#include "transport_drv.h"

#define WIFI_NETIF_MTU          1500
#define WIFI_NETIF_INPUT_TASK_STACK 1024
#define WIFI_NETIF_INPUT_TASK_PRIO  osPriorityNormal
#define WIFI_NETIF_TIMEOUT_TASK_STACK 512
#define WIFI_NETIF_TIMEOUT_TASK_PRIO  osPriorityLow

struct netif wifi_netif;

static uint8_t s_mac[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
static volatile uint8_t s_netif_up = 0;

/* called by transport_drv when STM32 wants to send a frame to ESP32 */
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    (void)netif;

    uint8_t *buf = malloc(p->tot_len);
    if (!buf) return ERR_MEM;

    uint16_t offset = 0;
    struct pbuf *q;
    for (q = p; q != NULL; q = q->next) {
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

static err_t wifi_netif_low_init(struct netif *netif)
{
    netif->hwaddr_len = ETH_HWADDR_LEN;
    memcpy(netif->hwaddr, s_mac, ETH_HWADDR_LEN);
    netif->mtu        = WIFI_NETIF_MTU;
    netif->flags      = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
    netif->output     = etharp_output;
    netif->linkoutput = low_level_output;
    netif->name[0]    = 'w';
    netif->name[1]    = '0';
    return ERR_OK;
}

static void netif_status_callback(struct netif *netif)
{
    if (netif_is_up(netif) && !ip4_addr_isany_val(*netif_ip4_addr(netif))) {
        printf("DHCP got IP: %s\r\n", ip4addr_ntoa(netif_ip4_addr(netif)));
        printf("GW: %s\r\n",          ip4addr_ntoa(netif_ip4_gw(netif)));
        printf("NM: %s\r\n",          ip4addr_ntoa(netif_ip4_netmask(netif)));
        s_netif_up = 1;
    } else {
        s_netif_up = 0;
    }
}

/* task: poll rx queue and feed frames into lwIP */
static void wifi_netif_input_task(void const *arg)
{
    (void)arg;
    printf("wifi_input_task started\r\n");
    QueueHandle_t q = netdev_get_rx_queue();
    printf("rx_queue=%p\r\n", q);
    if (!q) {
        printf("ERROR: rx queue is NULL!\r\n");
        vTaskSuspend(NULL);
        return;
    }
    struct pbuf *raw = NULL;

    for (;;) {
        printf("waiting for rx packet...\r\n");
        if (xQueueReceive(q, &raw, portMAX_DELAY) != pdTRUE)
            continue;
        printf("got rx packet len=%d\r\n", raw->len);

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

/* task: drive lwIP timers */
static void wifi_netif_timeout_task(void const *arg)
{
    (void)arg;
    for (;;) {
        sys_check_timeouts();
        osDelay(10);
    }
}

void wifi_netif_set_mac(uint8_t *mac)
{
    if (mac) memcpy(s_mac, mac, 6);
}

int wifi_netif_is_up(void)
{
    return s_netif_up;
}

int wifi_netif_init(void)
{
    printf("lwip initlizing ...\r\n");
    lwip_init();
    printf("lwip init done\r\n");

    ip4_addr_t ipaddr, netmask, gw;

    printf("lwip set zero ip, netmask, gw\r\n");
    ip4_addr_set_zero(&ipaddr);
    ip4_addr_set_zero(&netmask);
    ip4_addr_set_zero(&gw);
    printf("lwip set zero ip, netmask, gw DONE\r\n");

    if (!netif_add(&wifi_netif, &ipaddr, &netmask, &gw,
                   NULL, wifi_netif_low_init, ethernet_input)) {
        printf("netif_add failed\r\n");
        return -1;
    }

    printf("netif set default\r\n");
    netif_set_default(&wifi_netif);
    printf("netif set default OK\r\n");
    printf("netif set status CB\r\n");
    netif_set_status_callback(&wifi_netif, netif_status_callback);
    printf("netif set status CB OK\r\n");
    printf("netif setup\r\n");
    netif_set_up(&wifi_netif);
    printf("netif setup OK\r\n");
    printf("DHCP starting...\r\n");
    dhcp_start(&wifi_netif);
    printf("DHCP started\r\n");

    osThreadDef(wifi_input, wifi_netif_input_task,
                WIFI_NETIF_INPUT_TASK_PRIO, 0, WIFI_NETIF_INPUT_TASK_STACK);
    osThreadCreate(osThread(wifi_input), NULL);
    printf("Thread input created\r\n");

    osThreadDef(lwip_timeout, wifi_netif_timeout_task,
                WIFI_NETIF_TIMEOUT_TASK_PRIO, 0, WIFI_NETIF_TIMEOUT_TASK_STACK);
    osThreadCreate(osThread(lwip_timeout), NULL);
    printf("Thread lwip timeout created\r\n");

    return 0;
}