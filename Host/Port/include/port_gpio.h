#ifndef __PORT_GPIO_H
#define __PORT_GPIO_H

#include "stm32f4xx_hal.h"
#include "app_config.h"

#include "board_config.h"


void port_gpio_init(void);
uint8_t port_gpio_get_handshake(void);
uint8_t port_gpio_get_data_ready(void);

#endif