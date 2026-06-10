/*netdev_if.c*/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "netdev_ext.h"

#define NETDEV_RX_QUEUE_SIZE 10

struct netdev {
    char name[16];
    void *priv;
    struct netdev_ops *ops;
};

static QueueHandle_t s_rx_queue = NULL;

void netdev_rx_queue_init(void)
{
    s_rx_queue = xQueueCreate(NETDEV_RX_QUEUE_SIZE, sizeof(struct pbuf *));
}

QueueHandle_t netdev_get_rx_queue(void)
{
    return s_rx_queue;
}

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
    if (!s_rx_queue) {
        free(net_buf->payload);
        free(net_buf);
        return -1;
    }
    if (xQueueSend(s_rx_queue, &net_buf, 0) != pdTRUE) {
        free(net_buf->payload);
        free(net_buf);
        return -1;
    }
    return 0;
}