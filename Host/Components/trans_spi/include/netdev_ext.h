#ifndef __NETDEV_EXT_H
#define __NETDEV_EXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "queue.h"
#include "netdev_if.h"

void netdev_rx_queue_init(void);
QueueHandle_t netdev_get_rx_queue(void);

#ifdef __cplusplus
}
#endif

#endif