#include "cmsis_os.h"
#include "string.h"
#include "spi.h"
#include "gpio.h"
#include "trace.h"
#include "spi_drv.h"
#include "adapter.h"
#include "serial_drv.h"
#include "netdev_if.h"
#include "stats.h"
#include "board_config.h"

#define TO_SLAVE_QUEUE_SIZE               10
#define FROM_SLAVE_QUEUE_SIZE             10
#define TRANSACTION_TASK_STACK_SIZE       4096
#define PROCESS_RX_TASK_STACK_SIZE        4096
#define MAX_PAYLOAD_SIZE                  (MAX_SPI_BUFFER_SIZE - sizeof(struct esp_payload_header))

static struct esp_private *esp_priv[MAX_NETWORK_INTERFACES];

static struct netdev_ops esp_net_ops = {
    .netdev_open  = esp_netdev_open,
    .netdev_close = esp_netdev_close,
    .netdev_xmit  = esp_netdev_xmit,
};

static osSemaphoreId osSemaphore;
static osMutexId mutex_spi_trans;
static osThreadId process_rx_task_id = 0;
static osThreadId transaction_task_id = 0;

static QueueHandle_t to_slave_queue   = NULL;
static QueueHandle_t from_slave_queue = NULL;

static void (*spi_drv_evt_handler_fp)(uint8_t);

static void transaction_task(void const *pvParameters);
static void process_rx_task(void const *pvParameters);
static uint8_t *get_tx_buffer(uint8_t *is_valid_tx_buf);
static void deinit_netdev(void);

static struct esp_private *get_priv(uint8_t if_type, uint8_t if_num)
{
    for (int i = 0; i < MAX_NETWORK_INTERFACES; i++) {
        if (esp_priv[i] &&
            esp_priv[i]->if_type == if_type &&
            esp_priv[i]->if_num  == if_num)
            return esp_priv[i];
    }
    return NULL;
}

static int init_netdev(void)
{
    void *ndev = NULL;
    struct esp_private *priv = NULL;
    char *if_name = STA_INTERFACE;
    uint8_t if_type = ESP_STA_IF;

    for (int i = 0; i < MAX_NETWORK_INTERFACES; i++) {
        ndev = netdev_alloc(sizeof(struct esp_private), if_name);
        if (!ndev) { deinit_netdev(); return STM_FAIL; }

        priv = (struct esp_private *)netdev_get_priv(ndev);
        if (!priv) { deinit_netdev(); return STM_FAIL; }

        priv->netdev  = ndev;
        priv->if_type = if_type;
        priv->if_num  = 0;

        if (netdev_register(ndev, &esp_net_ops)) {
            deinit_netdev();
            return STM_FAIL;
        }

        if_name = SOFTAP_INTERFACE;
        if_type = ESP_AP_IF;
        esp_priv[i] = priv;
    }
    return STM_OK;
}

static void deinit_netdev(void)
{
    for (int i = 0; i < MAX_NETWORK_INTERFACES; i++) {
        if (esp_priv[i]) {
            if (esp_priv[i]->netdev) {
                netdev_unregister(esp_priv[i]->netdev);
                netdev_free(esp_priv[i]->netdev);
            }
            esp_priv[i] = NULL;
        }
    }
}

void transport_init(void (*transport_evt_handler_fp)(uint8_t))
{
    stm_ret_t retval = STM_OK;

    spi_drv_evt_handler_fp = transport_evt_handler_fp;
    osSemaphoreDef(SEM);

    retval = init_netdev();
    if (retval) {
        printf("netdev failed to init\n\r");
        assert(retval == STM_OK);
    }

    osSemaphore = osSemaphoreCreate(osSemaphore(SEM), 1);
    assert(osSemaphore);

    mutex_spi_trans = xSemaphoreCreateMutex();
    assert(mutex_spi_trans);

    to_slave_queue = xQueueCreate(TO_SLAVE_QUEUE_SIZE,
            sizeof(interface_buffer_handle_t));
    assert(to_slave_queue);

    from_slave_queue = xQueueCreate(FROM_SLAVE_QUEUE_SIZE,
            sizeof(interface_buffer_handle_t));
    assert(from_slave_queue);

    osThreadDef(transaction_thread, transaction_task,
            osPriorityAboveNormal, 0, TRANSACTION_TASK_STACK_SIZE);
    transaction_task_id = osThreadCreate(osThread(transaction_thread), NULL);
    assert(transaction_task_id);

    osThreadDef(rx_thread, process_rx_task,
            osPriorityAboveNormal, 0, PROCESS_RX_TASK_STACK_SIZE);
    process_rx_task_id = osThreadCreate(osThread(rx_thread), NULL);
    assert(process_rx_task_id);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_DATA_READY_PIN ||
        GPIO_Pin == GPIO_HANDSHAKE_PIN) {
        if (osSemaphore != NULL)
            osSemaphoreRelease(osSemaphore);
    }
}

static stm_ret_t spi_transaction_v2(uint8_t *txbuff)
{
    uint8_t *rxbuff = NULL;
    interface_buffer_handle_t buf_handle = {0};
    struct esp_payload_header *payload_header;
    uint16_t len, offset;
    HAL_StatusTypeDef retval = HAL_ERROR;
    uint16_t rx_checksum = 0, checksum = 0;

    rxbuff = (uint8_t *)malloc(MAX_SPI_BUFFER_SIZE);
    assert(rxbuff);
    memset(rxbuff, 0, MAX_SPI_BUFFER_SIZE);

    if (!txbuff) {
        txbuff = (uint8_t *)malloc(MAX_SPI_BUFFER_SIZE);
        assert(txbuff);
        memset(txbuff, 0, MAX_SPI_BUFFER_SIZE);
    }

    HAL_GPIO_WritePin(USR_SPI_CS_GPIO_Port, USR_SPI_CS_Pin, GPIO_PIN_RESET);
    retval = HAL_SPI_TransmitReceive(USER_SPI, txbuff, rxbuff,
            MAX_SPI_BUFFER_SIZE, HAL_MAX_DELAY);
    while (USER_SPI->State == HAL_SPI_STATE_BUSY);
    HAL_GPIO_WritePin(USR_SPI_CS_GPIO_Port, USR_SPI_CS_Pin, GPIO_PIN_SET);

    switch (retval) {
        case HAL_OK:
            payload_header = (struct esp_payload_header *)rxbuff;
            len    = le16toh(payload_header->len);
            offset = le16toh(payload_header->offset);

            if (!len || len > MAX_PAYLOAD_SIZE ||
                offset != sizeof(struct esp_payload_header)) {
                if (rxbuff) { free(rxbuff); rxbuff = NULL; }
                osDelay(0);
            } else {
                rx_checksum = le16toh(payload_header->checksum);
                payload_header->checksum = 0;
                checksum = compute_checksum(rxbuff, len + offset);

                if (checksum == rx_checksum) {
                    buf_handle.priv_buffer_handle = rxbuff;
                    buf_handle.free_buf_handle    = free;
                    buf_handle.payload_len        = len;
                    buf_handle.if_type            = payload_header->if_type;
                    buf_handle.if_num             = payload_header->if_num;
                    buf_handle.payload            = rxbuff + offset;
                    buf_handle.seq_num            = le16toh(payload_header->seq_num);
                    buf_handle.flag               = payload_header->flags;

                    if (pdTRUE != xQueueSend(from_slave_queue,
                            &buf_handle, portMAX_DELAY)) {
                        printf("Failed to send buffer\n\r");
                        goto done;
                    }
                } else {
                    if (rxbuff) { free(rxbuff); rxbuff = NULL; }
                }
            }

            if (txbuff) { free(txbuff); txbuff = NULL; }
            break;

        case HAL_TIMEOUT:
            printf("timeout in SPI transaction\n\r");
            goto done;

        case HAL_ERROR:
            printf("error in SPI transaction\n\r");
            goto done;

        default:
            printf("unknown error in SPI transaction\n\r");
            goto done;
    }
    return STM_OK;

done:
    if (txbuff) { free(txbuff); txbuff = NULL; }
    if (rxbuff) { free(rxbuff); rxbuff = NULL; }
    return STM_FAIL;
}

static void check_and_execute_spi_transaction(void)
{
    uint8_t *txbuff = NULL;
    uint8_t is_valid_tx_buf = 0;
    GPIO_PinState gpio_handshake    = GPIO_PIN_RESET;
    GPIO_PinState gpio_rx_data_ready = GPIO_PIN_RESET;

    gpio_handshake = HAL_GPIO_ReadPin(GPIO_HANDSHAKE_PORT, GPIO_HANDSHAKE_PIN);
    gpio_rx_data_ready = HAL_GPIO_ReadPin(GPIO_DATA_READY_PORT, GPIO_DATA_READY_PIN);

    if (gpio_handshake == GPIO_PIN_SET) {
        txbuff = get_tx_buffer(&is_valid_tx_buf);
        if (gpio_rx_data_ready == GPIO_PIN_SET || is_valid_tx_buf) {
            xSemaphoreTake(mutex_spi_trans, portMAX_DELAY);
            spi_transaction_v2(txbuff);
            xSemaphoreGive(mutex_spi_trans);
        }
    }
}

stm_ret_t send_to_slave(uint8_t iface_type, uint8_t iface_num,
        uint8_t *wbuffer, uint16_t wlen)
{
    interface_buffer_handle_t buf_handle = {0};

    if (!wbuffer || !wlen || wlen > MAX_PAYLOAD_SIZE) {
        printf("write fail: buff(%p) OR len(%u)>max(%u)\n\r",
                wbuffer, wlen, MAX_PAYLOAD_SIZE);
        if (wbuffer) { free(wbuffer); wbuffer = NULL; }
        return STM_FAIL;
    }

    buf_handle.if_type           = iface_type;
    buf_handle.if_num            = iface_num;
    buf_handle.payload_len       = wlen;
    buf_handle.payload           = wbuffer;
    buf_handle.priv_buffer_handle = wbuffer;
    buf_handle.free_buf_handle   = free;

    if (pdTRUE != xQueueSend(to_slave_queue, &buf_handle, portMAX_DELAY)) {
        printf("Failed to send to to_slave_queue\n\r");
        if (wbuffer) { free(wbuffer); wbuffer = NULL; }
        return STM_FAIL;
    }

    check_and_execute_spi_transaction();
    return STM_OK;
}

static void stop_spi_transactions_for_msec(int x)
{
    hard_delay(x);
}

static void transaction_task(void const *pvParameters)
{
    printf("\n\rESP-Hosted for ESP32-C2/C3/C6/S2/S3\n\r");

    for (;;) {
        if (osSemaphore != NULL) {
            if (osSemaphoreWait(osSemaphore, osWaitForever) == osOK)
                check_and_execute_spi_transaction();
        }
    }
}

static void process_rx_task(void const *pvParameters)
{
    stm_ret_t ret = STM_OK;
    interface_buffer_handle_t buf_handle = {0};
    uint8_t *payload = NULL;
    struct pbuf *buffer = NULL;
    struct esp_priv_event *event = NULL;
    struct esp_private *priv = NULL;

    while (1) {
        ret = xQueueReceive(from_slave_queue, &buf_handle, portMAX_DELAY);
        if (ret != pdTRUE) continue;

        payload = buf_handle.payload;

        if (buf_handle.if_type == ESP_SERIAL_IF) {
            serial_rx_handler(&buf_handle);

        } else if (buf_handle.if_type == ESP_STA_IF ||
                   buf_handle.if_type == ESP_AP_IF) {
            priv = get_priv(buf_handle.if_type, buf_handle.if_num);
            if (priv) {
                buffer = (struct pbuf *)malloc(sizeof(struct pbuf));
                assert(buffer);
                buffer->len     = buf_handle.payload_len;
                buffer->payload = malloc(buf_handle.payload_len);
                assert(buffer->payload);
                memcpy(buffer->payload, buf_handle.payload, buf_handle.payload_len);
                netdev_rx(priv->netdev, buffer);
            }

        } else if (buf_handle.if_type == ESP_PRIV_IF) {
            buffer = (struct pbuf *)malloc(sizeof(struct pbuf));
            assert(buffer);
            buffer->len     = buf_handle.payload_len;
            buffer->payload = malloc(buf_handle.payload_len);
            assert(buffer->payload);
            memcpy(buffer->payload, buf_handle.payload, buf_handle.payload_len);
            process_priv_communication(buffer);

            event = (struct esp_priv_event *)payload;
            if (event->event_type == ESP_PRIV_EVENT_INIT) {
                stop_spi_transactions_for_msec(50000);
                if (spi_drv_evt_handler_fp)
                    spi_drv_evt_handler_fp(TRANSPORT_ACTIVE);
            }

        } else if (buf_handle.if_type == ESP_TEST_IF) {
#if TEST_RAW_TP
            update_test_raw_tp_rx_len(buf_handle.payload_len);
#endif
        } else {
            printf("unknown if_type %d\n\r", buf_handle.if_type);
        }

        if (buf_handle.free_buf_handle)
            buf_handle.free_buf_handle(buf_handle.priv_buffer_handle);
    }
}

static uint8_t *get_tx_buffer(uint8_t *is_valid_tx_buf)
{
    struct esp_payload_header *payload_header;
    uint8_t *sendbuf = NULL;
    uint8_t *payload = NULL;
    uint16_t len = 0;
    interface_buffer_handle_t buf_handle = {0};

    *is_valid_tx_buf = 0;

    if (pdTRUE == xQueueReceive(to_slave_queue, &buf_handle, 0))
        len = buf_handle.payload_len;

    if (len) {
        sendbuf = (uint8_t *)malloc(MAX_SPI_BUFFER_SIZE);
        if (!sendbuf) { printf("malloc failed\n\r"); goto done; }

        memset(sendbuf, 0, MAX_SPI_BUFFER_SIZE);
        *is_valid_tx_buf = 1;

        payload_header = (struct esp_payload_header *)sendbuf;
        payload = sendbuf + sizeof(struct esp_payload_header);
        payload_header->len      = htole16(len);
        payload_header->offset   = htole16(sizeof(struct esp_payload_header));
        payload_header->if_type  = buf_handle.if_type;
        payload_header->if_num   = buf_handle.if_num;
        memcpy(payload, buf_handle.payload, min(len, MAX_PAYLOAD_SIZE));
        payload_header->checksum = htole16(compute_checksum(sendbuf,
                sizeof(struct esp_payload_header) + len));
    }

done:
    if (buf_handle.free_buf_handle)
        buf_handle.free_buf_handle(buf_handle.priv_buffer_handle);
    return sendbuf;
}