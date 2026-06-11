#ifndef __NETDEV_IF_H
#define __NETDEV_IF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Forward declare only — actual definition comes from lwIP pbuf.h */
struct pbuf;

typedef struct netdev *netdev_handle_t;

struct netdev_ops {
    int (*netdev_open)  (netdev_handle_t netdev);
    int (*netdev_close) (netdev_handle_t netdev);
    int (*netdev_xmit)  (netdev_handle_t netdev, struct pbuf *net_buf);
};

netdev_handle_t netdev_alloc(uint32_t sizeof_priv, char *name);
void            netdev_free(netdev_handle_t netdev);
void           *netdev_get_priv(netdev_handle_t netdev);
int             netdev_register(netdev_handle_t netdev, struct netdev_ops *ops);
int             netdev_unregister(netdev_handle_t netdev);
int             netdev_rx(netdev_handle_t netdev, struct pbuf *net_buf);

#ifdef __cplusplus
}
#endif

#endif