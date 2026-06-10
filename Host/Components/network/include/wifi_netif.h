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

#ifdef __cplusplus
}
#endif

#endif