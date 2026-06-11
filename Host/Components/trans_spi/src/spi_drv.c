/*spi_drv.c*/
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
#include "lwip/pbuf.h"

#define TO_SLAVE_QUEUE_SIZE         10
#define FROM_SLAVE_QUEUE_SIZE       10
#define TRANSACTION_TASK_STACK_SIZE 4096
#define PROCESS_RX_TASK_STACK_SIZE  4096
#define MAX_PAYLOAD_SIZE            (MAX_SPI_BUFFER_SIZE - sizeof(struct esp_payload_header))

static uint8_t first_trans = 1;
static struct esp_private *esp_priv[MAX_NETWORK_INTERFACES];
static struct netdev_ops esp_net_ops = {
    .netdev_open  = esp_netdev_open,
    .netdev_close = esp_netdev_close,
    .netdev_xmit  = esp_netdev_xmit,
};

static SemaphoreHandle_t spi_sem;
static SemaphoreHandle_t mutex_spi_trans;
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
    spi_drv_evt_handler_fp = transport_evt_handler_fp;

    if (init_netdev()) {
        printf("netdev failed to init\r\n");
        assert(0);
    }

    spi_sem = xSemaphoreCreateBinary();
    assert(spi_sem);

    mutex_spi_trans = xSemaphoreCreateMutex();
    assert(mutex_spi_trans);

    to_slave_queue = xQueueCreate(TO_SLAVE_QUEUE_SIZE, sizeof(interface_buffer_handle_t));
    assert(to_slave_queue);

    from_slave_queue = xQueueCreate(FROM_SLAVE_QUEUE_SIZE, sizeof(interface_buffer_handle_t));
    assert(from_slave_queue);

    osThreadDef(transaction_thread, transaction_task,
            osPriorityNormal, 0, TRANSACTION_TASK_STACK_SIZE);
    transaction_task_id = osThreadCreate(osThread(transaction_thread), NULL);
    assert(transaction_task_id);

    osThreadDef(rx_thread, process_rx_task,
            osPriorityAboveNormal, 0, PROCESS_RX_TASK_STACK_SIZE);
    process_rx_task_id = osThreadCreate(osThread(rx_thread), NULL);
    assert(process_rx_task_id);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_DATA_READY_PIN || GPIO_Pin == GPIO_HANDSHAKE_PIN) {
        if (spi_sem != NULL) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xSemaphoreGiveFromISR(spi_sem, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

static stm_ret_t spi_transaction_v2(uint8_t *txbuff)
{
    uint32_t heap_before = xPortGetFreeHeapSize();
    printf("spi_tx_v2 ENTER\r\n");
    printf("SPI_TX: heap=%d\r\n", heap_before);
    uint8_t *rxbuff = NULL;
    uint8_t txbuff_allocated = 0;
    interface_buffer_handle_t buf_handle = {0};
    struct esp_payload_header *payload_header;
    uint16_t len, offset;
    HAL_StatusTypeDef retval = HAL_ERROR;
    uint16_t rx_checksum = 0, checksum = 0;

    printf("SPI_TX: heap=%d\r\n", xPortGetFreeHeapSize());
    rxbuff = (uint8_t *)malloc(MAX_SPI_BUFFER_SIZE);
    assert(rxbuff);
    printf("rxbuff=%p\r\n", rxbuff);
    if (!rxbuff) {
        printf("rxbuff MALLOC FAILED\r\n");
        return STM_FAIL;
    }
    memset(rxbuff, 0, MAX_SPI_BUFFER_SIZE);

    if (!txbuff) {
        txbuff = (uint8_t *)malloc(MAX_SPI_BUFFER_SIZE);
        printf("txbuff=%p\r\n", txbuff);
        if (!txbuff) {
            printf("txbuff MALLOC FAILED\r\n");
            free(rxbuff);
            return STM_FAIL;
        }
        memset(txbuff, 0, MAX_SPI_BUFFER_SIZE);
        txbuff_allocated = 1;
    }
    if (first_trans)
        osDelay(10);

    HAL_GPIO_WritePin(USR_SPI_CS_GPIO_Port, USR_SPI_CS_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    printf("CS LOW - starting transfer\r\n");
    retval = HAL_SPI_TransmitReceive(USER_SPI, txbuff, rxbuff,
            MAX_SPI_BUFFER_SIZE, HAL_MAX_DELAY);
    printf("transfer done\r\n");
    while (USER_SPI->State == HAL_SPI_STATE_BUSY);
    HAL_GPIO_WritePin(USR_SPI_CS_GPIO_Port, USR_SPI_CS_Pin, GPIO_PIN_SET);

    if (txbuff_allocated) { free(txbuff); txbuff = NULL; }

    if (retval != HAL_OK) {
        printf("SPI error: %d\r\n", retval);
        free(rxbuff);
        return STM_FAIL;
    }

    payload_header = (struct esp_payload_header *)rxbuff;
    len    = le16toh(payload_header->len);
    offset = le16toh(payload_header->offset);

    printf("HDR: if_type=%d len=%d\r\n", payload_header->if_type, len);

    if (!len || len > MAX_PAYLOAD_SIZE || offset != sizeof(struct esp_payload_header)) {
        free(rxbuff);
        return STM_OK;
    }

    rx_checksum = le16toh(payload_header->checksum);
    payload_header->checksum = 0;
    checksum = compute_checksum(rxbuff, len + offset);

    if (checksum != rx_checksum) {
        printf("checksum mismatch rx=%04X calc=%04X\r\n", rx_checksum, checksum);
        free(rxbuff);
        return STM_FAIL;
    }

    buf_handle.priv_buffer_handle = rxbuff;
    buf_handle.free_buf_handle    = free;
    buf_handle.payload_len        = len;
    buf_handle.if_type            = payload_header->if_type;
    buf_handle.if_num             = payload_header->if_num;
    buf_handle.payload            = rxbuff + offset;
    buf_handle.seq_num            = le16toh(payload_header->seq_num);
    buf_handle.flag               = payload_header->flags;

    if (pdTRUE != xQueueSend(from_slave_queue, &buf_handle, portMAX_DELAY)) {
        printf("from_slave_queue full\r\n");
        free(rxbuff);
        return STM_FAIL;
    }

    printf("queued to from_slave OK\r\n");

    uint32_t heap_after = xPortGetFreeHeapSize();
    printf("SPI_TRANS EXIT: heap_before=%d, heap_after=%d, diff=%d\r\n", 
           heap_before, heap_after, (int)heap_before - (int)heap_after);
    return STM_OK;
}

static void check_and_execute_spi_transaction(void)
{
    uint8_t *txbuff = NULL;
    uint8_t is_valid_tx_buf = 0;

    txbuff = get_tx_buffer(&is_valid_tx_buf);

    if (!is_valid_tx_buf) {
        GPIO_PinState hs = HAL_GPIO_ReadPin(GPIO_HANDSHAKE_PORT, GPIO_HANDSHAKE_PIN);
        GPIO_PinState dr = HAL_GPIO_ReadPin(GPIO_DATA_READY_PORT, GPIO_DATA_READY_PIN);
        if (hs != GPIO_PIN_SET || dr != GPIO_PIN_SET) {
            if (txbuff) free(txbuff);
            return;
        }
        // GPIO pins ready but no TX data - still need to handle RX
        // Don't free txbuff yet, will be freed at end
    }

    if (is_valid_tx_buf) {
        uint32_t tick = HAL_GetTick();
        while (HAL_GPIO_ReadPin(GPIO_HANDSHAKE_PORT, GPIO_HANDSHAKE_PIN) != GPIO_PIN_SET) {
            if (HAL_GetTick() - tick > 2000) {
                printf("check_exec: HS timeout, drop\r\n");
                if (txbuff) free(txbuff);
                return;
            }
            osDelay(1);
        }
        printf("check_exec: HS ready, calling SPI\r\n");
    }

    // printf("check_exec: taking mutex\r\n");
    xSemaphoreTake(mutex_spi_trans, portMAX_DELAY);
    // printf("check_exec: got mutex\r\n");
    spi_transaction_v2(txbuff);
    xSemaphoreGive(mutex_spi_trans);
    // printf("check_exec: gave mutex\r\n");
    if (txbuff) { free(txbuff); txbuff = NULL; }
}

stm_ret_t send_to_slave(uint8_t iface_type, uint8_t iface_num,
        uint8_t *wbuffer, uint16_t wlen)
{
    interface_buffer_handle_t buf_handle = {0};

    if (!wbuffer || !wlen || wlen > MAX_PAYLOAD_SIZE) {
        printf("send_to_slave: invalid args\r\n");
        if (wbuffer) free(wbuffer);
        return STM_FAIL;
    }

    buf_handle.if_type            = iface_type;
    buf_handle.if_num             = iface_num;
    buf_handle.payload_len        = wlen;
    buf_handle.payload            = wbuffer;
    buf_handle.priv_buffer_handle = wbuffer;
    buf_handle.free_buf_handle    = free;

    if (pdTRUE != xQueueSend(to_slave_queue, &buf_handle, portMAX_DELAY)) {
        printf("send_to_slave: queue full\r\n");
        free(wbuffer);
        return STM_FAIL;
    }

    // printf("send_to_slave: queued if_type=%d len=%d waiting=%d\r\n", 
    // iface_type, wlen, (int)uxQueueMessagesWaiting(to_slave_queue));

    xSemaphoreGive(spi_sem);
    return STM_OK;
}

static void transaction_task(void const *pvParameters)
{
    printf("transaction_task started\r\n");
    for (;;) {
        // printf("TT: waiting\r\n");
        xSemaphoreTake(spi_sem, pdMS_TO_TICKS(100));
        // printf("TT: woke\r\n");
        check_and_execute_spi_transaction();

        uint32_t tick = HAL_GetTick();
        while (HAL_GetTick() - tick < 2000) {
            GPIO_PinState hs = HAL_GPIO_ReadPin(GPIO_HANDSHAKE_PORT, GPIO_HANDSHAKE_PIN);
            GPIO_PinState dr = HAL_GPIO_ReadPin(GPIO_DATA_READY_PORT, GPIO_DATA_READY_PIN);
            if (hs == GPIO_PIN_SET && dr == GPIO_PIN_SET) {
                printf("TT: polling response HS=%d DR=%d\r\n", hs, dr);
                check_and_execute_spi_transaction();
                break;
            }
            osDelay(1);
        }
        // printf("TT: poll done elapsed=%lu\r\n", HAL_GetTick() - tick);
    }
}

static void process_rx_task(void const *pvParameters)
{
    printf("process_rx_task started\r\n");
    interface_buffer_handle_t buf_handle = {0};
    struct pbuf *buffer = NULL;
    struct esp_priv_event *event = NULL;
    struct esp_private *priv = NULL;

    while (1) {
        printf("process_rx: waiting...\r\n");
        if (pdTRUE != xQueueReceive(from_slave_queue, &buf_handle, portMAX_DELAY))
            continue;

        // printf("RX: if_type=%d len=%d\r\n", buf_handle.if_type, buf_handle.payload_len);

        if (buf_handle.if_type == ESP_SERIAL_IF) {
            serial_rx_handler(&buf_handle);

        } else if (buf_handle.if_type == ESP_STA_IF || buf_handle.if_type == ESP_AP_IF) {
            priv = get_priv(buf_handle.if_type, buf_handle.if_num);
            if (priv) {
                buffer = (struct pbuf *)malloc(sizeof(struct pbuf));
                assert(buffer);
                buffer->len     = buf_handle.payload_len;
                buffer->payload = malloc(buf_handle.payload_len);
                assert(buffer->payload);
                memcpy(buffer->payload, buf_handle.payload, buf_handle.payload_len);
                netdev_rx(priv->netdev, buffer);
                // Free the pbuf struct (payload might be managed by netdev)
                // free(buffer);
                // buffer = NULL;
            }

        } else if (buf_handle.if_type == ESP_PRIV_IF) {
            first_trans = 0;
            free(buffer);
            buffer = (struct pbuf *)malloc(sizeof(struct pbuf));
            assert(buffer);
            buffer->len     = buf_handle.payload_len;
            buffer->payload = malloc(buf_handle.payload_len);
            assert(buffer->payload);
            memcpy(buffer->payload, buf_handle.payload, buf_handle.payload_len);
            process_priv_communication(buffer);

            event = (struct esp_priv_event *)buf_handle.payload;
            if (event->event_type == ESP_PRIV_EVENT_INIT) {
                if (spi_drv_evt_handler_fp)
                    spi_drv_evt_handler_fp(TRANSPORT_ACTIVE);
            }
            // Free the pbuf struct and payload after processing
            // if (buffer) {
            //     if (buffer->payload)
            //         free(buffer->payload);
            //     free(buffer);
            //     buffer = NULL;
            // }
        }

        if (buf_handle.free_buf_handle)
            buf_handle.free_buf_handle(buf_handle.priv_buffer_handle);
    }
}

static uint8_t *get_tx_buffer(uint8_t *is_valid_tx_buf)
{
    // printf("get_tx_buffer: queue=%d\r\n", (int)uxQueueMessagesWaiting(to_slave_queue));
    struct esp_payload_header *payload_header;
    uint8_t *sendbuf = NULL;
    uint8_t *payload = NULL;
    uint16_t len = 0;
    interface_buffer_handle_t buf_handle = {0};

    *is_valid_tx_buf = 0;

    if (pdTRUE != xQueueReceive(to_slave_queue, &buf_handle, 0))
        return NULL;

    // printf("get_tx_buffer: got if_type=%d len=%d payload=%p\r\n", 
    // buf_handle.if_type, buf_handle.payload_len, buf_handle.payload);

    len = buf_handle.payload_len;
    if (!len) goto done;

    printf("get_tx_buffer: malloc %d bytes, heap=%d\r\n", MAX_SPI_BUFFER_SIZE, xPortGetFreeHeapSize());
    sendbuf = (uint8_t *)malloc(MAX_SPI_BUFFER_SIZE);
    printf("get_tx_buffer: sendbuf=%p\r\n", sendbuf);
    if (!sendbuf) { printf("get_tx_buffer: malloc failed\r\n"); goto done; }

    printf("get_tx_buffer: memset\r\n");
    memset(sendbuf, 0, MAX_SPI_BUFFER_SIZE);
    printf("get_tx_buffer: building header\r\n");
    payload_header = (struct esp_payload_header *)sendbuf;
    payload = sendbuf + sizeof(struct esp_payload_header);
    payload_header->len      = htole16(len);
    payload_header->offset   = htole16(sizeof(struct esp_payload_header));
    payload_header->if_type  = buf_handle.if_type;
    payload_header->if_num   = buf_handle.if_num;
    printf("get_tx_buffer: memcpy len=%d\r\n", len);
    memcpy(payload, buf_handle.payload, min(len, MAX_PAYLOAD_SIZE));
    printf("get_tx_buffer: checksum\r\n");
    payload_header->checksum = htole16(compute_checksum(sendbuf,
            sizeof(struct esp_payload_header) + len));
    *is_valid_tx_buf = 1;
    printf("get_tx_buffer: done\r\n");

done:
    printf("get_tx_buffer: freeing priv=%p func=%p\r\n", 
        buf_handle.priv_buffer_handle, buf_handle.free_buf_handle);
    if (buf_handle.free_buf_handle)
        buf_handle.free_buf_handle(buf_handle.priv_buffer_handle);
    printf("get_tx_buffer: returning sendbuf=%p\r\n", sendbuf);
    return sendbuf;
}