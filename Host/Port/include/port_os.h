#ifndef __PORT_OS_H
#define __PORT_OS_H

#include "platform_wrapper.h"
#include "board_config.h"

extern serial_ll_handle_t *serial_ll_if_g;
void control_path_rx_indication(void);

#endif