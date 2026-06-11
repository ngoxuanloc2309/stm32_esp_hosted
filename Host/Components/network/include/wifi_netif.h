#ifndef __WIFI_NETIF_H
#define __WIFI_NETIF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lwip/netif.h"
#include "lwip/ip_addr.h"

extern struct netif wifi_netif;

int  wifi_netif_init(void);
int  wifi_netif_is_up(void);
void wifi_netif_set_mac(uint8_t *mac);

/**
 * Set a static IP on the lwIP netif.
 * Called from dhcp_dns_event_cb in app.c when ESP32 reports its DHCP result.
 * All arguments are dotted-decimal ASCII strings (e.g. "10.43.137.37").
 */
void wifi_netif_set_static_ip(const char *ip, const char *nm, const char *gw);

#ifdef __cplusplus
}
#endif

#endif /* __WIFI_NETIF_H */