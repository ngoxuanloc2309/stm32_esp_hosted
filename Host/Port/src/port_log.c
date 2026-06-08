#include <stdio.h>
#include <string.h>
#include "port_log.h"
#include "usart.h"

void port_log_write(const char *msg)
{
    HAL_UART_Transmit(UART_LOG, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void print_hex_dump(uint8_t *buff, uint16_t rx_len, char *human_str)
{
    char hex[4];
    if (human_str)
        port_log_write(human_str);
    port_log_write(": ");
    for (uint16_t i = 0; i < rx_len; i++) {
        snprintf(hex, sizeof(hex), "%02X ", buff[i]);
        port_log_write(hex);
    }
    port_log_write("\r\n");
}