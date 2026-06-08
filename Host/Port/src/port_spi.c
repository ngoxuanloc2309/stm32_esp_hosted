#include "port_spi.h"
#include "spi.h"

int port_spi_init(void)
{
    HAL_GPIO_WritePin(USR_SPI_CS_GPIO_Port, USR_SPI_CS_Pin, GPIO_PIN_SET);
    return 0;
}

int port_spi_transfer(uint8_t *tx_buf, uint8_t *rx_buf, uint16_t len)
{
    HAL_StatusTypeDef ret;
    HAL_GPIO_WritePin(USR_SPI_CS_GPIO_Port, USR_SPI_CS_Pin, GPIO_PIN_RESET);
    ret = HAL_SPI_TransmitReceive(USER_SPI, tx_buf, rx_buf, len, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(USR_SPI_CS_GPIO_Port, USR_SPI_CS_Pin, GPIO_PIN_SET);
    return (ret == HAL_OK) ? 0 : -1;
}