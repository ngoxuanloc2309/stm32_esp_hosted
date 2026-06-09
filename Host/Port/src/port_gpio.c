/*port_gpio.c*/
#include "port_gpio.h"

void port_gpio_init(void)
{
    /* GPIO already initialized in MX_GPIO_Init */
}

uint8_t port_gpio_get_handshake(void)
{
    return HAL_GPIO_ReadPin(GPIO_HANDSHAKE_PORT, GPIO_HANDSHAKE_PIN);
}

uint8_t port_gpio_get_data_ready(void)
{
    return HAL_GPIO_ReadPin(GPIO_DATA_READY_PORT, GPIO_DATA_READY_PIN);
}