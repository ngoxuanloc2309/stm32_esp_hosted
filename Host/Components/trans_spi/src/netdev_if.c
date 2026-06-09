/*netdev_if.c*/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "netdev_if.h"

struct netdev {
    char name[16];
    void *priv;
    struct netdev_ops *ops;
};

netdev_handle_t netdev_alloc(uint32_t sizeof_priv, char *name)
{
    struct netdev *dev = calloc(1, sizeof(struct netdev) + sizeof_priv);
    if (!dev) return NULL;
    strncpy(dev->name, name, sizeof(dev->name) - 1);
    dev->priv = (uint8_t *)dev + sizeof(struct netdev);
    return dev;
}

void netdev_free(netdev_handle_t netdev)
{
    if (netdev) free(netdev);
}

void *netdev_get_priv(netdev_handle_t netdev)
{
    if (!netdev) return NULL;
    return ((struct netdev *)netdev)->priv;
}

int netdev_register(netdev_handle_t netdev, struct netdev_ops *ops)
{
    if (!netdev || !ops) return -1;
    ((struct netdev *)netdev)->ops = ops;
    return 0;
}

int netdev_unregister(netdev_handle_t netdev)
{
    if (!netdev) return -1;
    ((struct netdev *)netdev)->ops = NULL;
    return 0;
}

int netdev_rx(netdev_handle_t netdev, struct pbuf *net_buf)
{
    if (!netdev || !net_buf) return -1;
    /* placeholder: forward to network stack when LwIP is added */
    if (net_buf->payload) free(net_buf->payload);
    free(net_buf);
    return 0;
}