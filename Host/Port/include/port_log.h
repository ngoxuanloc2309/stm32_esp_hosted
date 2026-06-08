#ifndef __PORT_LOG_H
#define __PORT_LOG_H

#include <stdint.h>
#include "board_config.h"

void port_log_init(void);
void port_log_write(const char *msg);
void print_hex_dump(uint8_t *buff, uint16_t rx_len, char *human_str);

#endif