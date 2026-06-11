#ifndef __LWIPOPTS_H
#define __LWIPOPTS_H

/* USER DEFINE */

/* ---- FreeRTOS integration ---- */
#define NO_SYS                      0
#define LWIP_FREERTOS_THREAD_STACKSIZE 512
#define LWIP_FREERTOS_THREAD_PRIO   4

/* ---- Memory ---- */
#define MEM_LIBC_MALLOC             1
#define MEMP_MEM_MALLOC             1
#define MEM_ALIGNMENT               4

/* ---- pbuf ---- */
#define PBUF_POOL_SIZE              8
#define PBUF_POOL_BUFSIZE           1600

/* ---- ARP ---- */
#define LWIP_ARP                    1
#define ARP_TABLE_SIZE              4
#define ARP_QUEUEING                1

/* ---- IP ---- */
#define LWIP_IPV4                   1
#define LWIP_IPV6                   0
#define IP_REASSEMBLY               0
#define IP_FRAG                     0

/* ---- ICMP ---- */
#define LWIP_ICMP                   1

/* ---- DHCP ---- */
#define LWIP_DHCP                   1
#define DHCP_DOES_ARP_CHECK         0

/* ---- DNS ---- */
#define LWIP_DNS                    1
#define DNS_TABLE_SIZE              4
#define DNS_MAX_NAME_LENGTH         64

/* ---- TCP ---- */
#define LWIP_TCP                    1
#define TCP_MSS                     1460
#define TCP_WND                     (4 * TCP_MSS)
#define TCP_SND_BUF                 (4 * TCP_MSS)
#define TCP_SND_QUEUELEN            8
#define MEMP_NUM_TCP_SEG            16
#define MEMP_NUM_TCP_PCB            4
#define MEMP_NUM_TCP_PCB_LISTEN     2

/* ---- UDP ---- */
#define LWIP_UDP                    1
#define MEMP_NUM_UDP_PCB            4

/* ---- Socket / Netconn ---- */
#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0

/* ---- Callback-style raw API ---- */
#define LWIP_CALLBACK_API           1

/* ---- MQTT ---- */
#define LWIP_ALTCP                  0
#define MQTT_OUTPUT_RINGBUF_SIZE    512
#define MQTT_VAR_HEADER_BUFFER_LEN  128
#define MQTT_REQ_MAX_IN_FLIGHT      4
#define MQTT_CONNECT_TIMOUT         10

/* ---- Netif ---- */
#define LWIP_NETIF_API              0
#define LWIP_NETIF_STATUS_CALLBACK  1
#define LWIP_NETIF_LINK_CALLBACK    1
#define LWIP_NETIF_HOSTNAME         1

/* ---- Stats (disable to save RAM) ---- */
#define LWIP_STATS                  0

/* ---- Checksum ---- */
#define CHECKSUM_BY_HARDWARE        0

/* ---- Debug (comment out in production) ---- */
/* #define LWIP_DEBUG */
/* #define MQTT_DEBUG       LWIP_DBG_ON */
/* #define DHCP_DEBUG       LWIP_DBG_ON */
/* #define TCP_DEBUG        LWIP_DBG_ON */

#endif