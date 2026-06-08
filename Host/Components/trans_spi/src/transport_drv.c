#include "transport_drv.h"
#include "stats.h"

static char chip_type = ESP_PRIV_FIRMWARE_CHIP_UNRECOGNIZED;

int esp_netdev_open(netdev_handle_t netdev)  { return STM_OK; }
int esp_netdev_close(netdev_handle_t netdev) { return STM_OK; }

int esp_netdev_xmit(netdev_handle_t netdev, struct pbuf *net_buf)
{
    struct esp_private *priv = NULL;
    if (!netdev || !net_buf) return STM_FAIL;
    priv = (struct esp_private *)netdev_get_priv(netdev);
    if (!priv) return STM_FAIL;
    int ret = send_to_slave(priv->if_type, priv->if_num,
            net_buf->payload, net_buf->len);
    free(net_buf);
    return ret;
}

void process_capabilities(uint8_t cap)
{
#if DEBUG_TRANSPORT
    printf("capabilities: 0x%x\n\r", cap);
#endif
}

void process_priv_communication(struct pbuf *pbuf)
{
    if (!pbuf || !pbuf->payload) return;
    struct esp_priv_event *header = (struct esp_priv_event *)pbuf->payload;
    if (header->event_type == ESP_PRIV_EVENT_INIT)
        process_event(pbuf->payload, pbuf->len);
    hosted_free(pbuf);
}

void print_capabilities(uint32_t cap)
{
    printf("Features supported:\n\r");
    if (cap & ESP_WLAN_SDIO_SUPPORT)
        printf("\t * WLAN\n\r");
    if ((cap & ESP_BT_UART_SUPPORT) || (cap & ESP_BT_SDIO_SUPPORT)) {
        printf("\t * BT/BLE\n\r");
        if (cap & ESP_BLE_ONLY_SUPPORT)
            printf("\t   - BLE only\n\r");
        else if (cap & ESP_BR_EDR_ONLY_SUPPORT)
            printf("\t   - BR EDR only\n\r");
    }
}

void process_event(uint8_t *evt_buf, uint16_t len)
{
    if (!evt_buf || !len) return;
    struct esp_priv_event *event = (struct esp_priv_event *)evt_buf;
    if (event->event_type == ESP_PRIV_EVENT_INIT) {
        printf("Received INIT event from ESP\n\r");
        print_hex_dump(event->event_data, event->event_len, "process event");
        if (process_init_event(event->event_data, event->event_len))
            printf("failed to process init event\n\r");
    } else {
        printf("Drop unknown event\n\r");
    }
}

int process_init_event(uint8_t *evt_buf, uint8_t len)
{
    uint8_t len_left = len, tag_len;
    uint8_t *pos;
    if (!evt_buf) return STM_FAIL;
    pos = evt_buf;
    while (len_left) {
        tag_len = *(pos + 1);
        if (*pos == ESP_PRIV_CAPABILITY) {
            process_capabilities(*(pos + 2));
            print_capabilities(*(pos + 2));
        } else if (*pos == ESP_PRIV_FIRMWARE_CHIP_ID) {
            chip_type = *(pos + 2);
        }
        pos += (tag_len + 2);
        len_left -= (tag_len + 2);
    }

    if (chip_type != ESP_PRIV_FIRMWARE_CHIP_ESP32C6 &&
        chip_type != ESP_PRIV_FIRMWARE_CHIP_ESP32S2 &&
        chip_type != ESP_PRIV_FIRMWARE_CHIP_ESP32S3 &&
        chip_type != ESP_PRIV_FIRMWARE_CHIP_ESP32C2 &&
        chip_type != ESP_PRIV_FIRMWARE_CHIP_ESP32C3 &&
        chip_type != ESP_PRIV_FIRMWARE_CHIP_ESP32) {
        printf("Unrecognized ESP board type [%d]\n\r", chip_type);
        chip_type = ESP_PRIV_FIRMWARE_CHIP_UNRECOGNIZED;
        return STM_FAIL;
    }

    printf("ESP board type: %d\n\r", chip_type);
    return STM_OK;
}