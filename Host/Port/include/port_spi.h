#ifndef __PORT_SPI_H
#define __PORT_SPI_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

#include "board_config.h"

int port_spi_init(void);
int port_spi_transfer(uint8_t *tx_buf, uint8_t *rx_buf, uint16_t len);

#endif