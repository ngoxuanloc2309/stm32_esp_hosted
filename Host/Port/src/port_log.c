/*port_log.c*/
#include <stdio.h>
#include <string.h>
#include "port_log.h"
#include "usart.h"

void port_log_write(const char *msg)
{
    HAL_UART_Transmit(UART_LOG, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

int __io_putchar(int ch)
{
    HAL_UART_Transmit(UART_LOG, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}