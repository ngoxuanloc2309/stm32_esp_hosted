/*port_serial_drv.c*/
#include <stdlib.h>
#include <string.h>
#include "platform_wrapper.h"
#include "serial_ll_if.h"
#include "port_os.h"
#include "serial_if.h"

struct serial_drv_handle_t {
    serial_ll_handle_t *ll_handle;
};

struct serial_drv_handle_t* serial_drv_open(const char* transport)
{
    (void)transport;

    struct serial_drv_handle_t *h = hosted_malloc(sizeof(struct serial_drv_handle_t));
    if (!h) return NULL;

    if (!serial_ll_if_g) {
        serial_ll_if_g = serial_ll_init(control_path_rx_indication);
        if (!serial_ll_if_g) {
            hosted_free(h);
            return NULL;
        }
        if (serial_ll_if_g->fops->open(serial_ll_if_g) != 0) {
            hosted_free(h);
            return NULL;
        }
    }

    h->ll_handle = serial_ll_if_g;
    return h;
}

int serial_drv_write(struct serial_drv_handle_t *handle,
                     uint8_t *buf, int in_count, int *out_count)
{
    printf("serial_drv_write: in_count=%d\r\n", in_count);
    if (!handle || !handle->ll_handle) {
        printf("serial_drv_write: NULL handle!\r\n");
        return -1;
    }
    int ret = handle->ll_handle->fops->write(handle->ll_handle, buf, (uint16_t)in_count);
    printf("serial_drv_write: ret=%d\r\n", ret);
    if (ret == 0 && out_count)
        *out_count = in_count;
    return ret;
}

// uint8_t* serial_drv_read(struct serial_drv_handle_t *handle,
//                          uint32_t *out_nbyte)
// {
//     if (!handle || !handle->ll_handle) return NULL;

//     printf("serial_drv_read: waiting for data...\r\n");
//     uint16_t rlen = 0;
//     uint8_t *buf = handle->ll_handle->fops->read(handle->ll_handle, &rlen);
//     printf("serial_drv_read: fops->read returned rlen=%d\r\n", rlen);

//     if (!buf || !rlen) {
//         if (out_nbyte) *out_nbyte = 0;
//         return NULL;
//     }

//     uint32_t proto_len = 0;
//     if (parse_tlv(buf, &proto_len) != 0) {
//         printf("serial_drv_read: TLV parse failed\r\n");
//         if (out_nbyte) *out_nbyte = 0;
//         return NULL;
//     }

//     char *ep_name = CTRL_EP_NAME_RESP;
//     uint32_t offset = 1 + 2 + strlen(ep_name) + 1 + 2;

//     printf("serial_drv_read: proto_len=%lu offset=%lu\r\n", proto_len, offset);

//     // Alloc new buffer để caller có thể free an toàn
//     uint8_t *proto_buf = hosted_malloc(proto_len);
//     if (!proto_buf) {
//         if (out_nbyte) *out_nbyte = 0;
//         return NULL;
//     }
//     memcpy(proto_buf, buf + offset, proto_len);

//     if (out_nbyte) *out_nbyte = proto_len;
//     return proto_buf;
// }

uint8_t* serial_drv_read(struct serial_drv_handle_t *handle,
                         uint32_t *out_nbyte)
{
    if (!handle || !handle->ll_handle) return NULL;
    uint16_t rlen = 0;
    uint8_t *buf = handle->ll_handle->fops->read(handle->ll_handle, &rlen);
    if (!buf || !rlen) {
        if (out_nbyte) *out_nbyte = 0;
        return NULL;
    }

    uint32_t proto_len = 0;
    if (parse_tlv(buf, &proto_len) != 0) {
        if (out_nbyte) *out_nbyte = 0;
        hosted_free(buf);  
        return NULL;
    }

    char *ep_name = CTRL_EP_NAME_RESP;
    uint32_t offset = 1 + 2 + strlen(ep_name) + 1 + 2;

    uint8_t *proto_buf = hosted_malloc(proto_len);
    if (!proto_buf) {
        hosted_free(buf);
        if (out_nbyte) *out_nbyte = 0;
        return NULL;
    }
    memcpy(proto_buf, buf + offset, proto_len);
    hosted_free(buf);  

    if (out_nbyte) *out_nbyte = proto_len;
    return proto_buf;
}

int serial_drv_close(struct serial_drv_handle_t **handle)
{
    if (!handle || !(*handle)) return -1;
    hosted_free(*handle);
    *handle = NULL;
    return 0;
}