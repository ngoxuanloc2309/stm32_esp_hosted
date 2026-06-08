#ifndef __PORT_LOG_H
#define __PORT_LOG_H

#include <stdint.h>
#include "board_config.h"

void port_log_init(void);
void port_log_write(const char *msg);

#endif