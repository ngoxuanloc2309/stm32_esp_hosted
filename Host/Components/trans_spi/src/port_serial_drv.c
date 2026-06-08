#include <stdlib.h>
#include <string.h>
#include "platform_wrapper.h"
#include "serial_ll_if.h"

/* Concrete definition of the opaque handle */
struct serial_drv_handle_t {
    serial_ll_handle_t *ll_handle;
};

struct serial_drv_handle_t* serial_drv_open(const char* transport)
{
    (void)transport; /* STM32: transport string not used */

    struct serial_drv_handle_t *h = hosted_malloc(sizeof(struct serial_drv_handle_t));
    if (!h) return NULL;

    h->ll_handle = serial_ll_init(NULL);
    if (!h->ll_handle) {
        hosted_free(h);
        return NULL;
    }

    if (h->ll_handle->fops->open(h->ll_handle) != 0) {
        hosted_free(h);
        return NULL;
    }

    return h;
}

int serial_drv_write(struct serial_drv_handle_t *handle,
                     uint8_t *buf, int in_count, int *out_count)
{
    if (!handle || !handle->ll_handle) return -1;

    int ret = handle->ll_handle->fops->write(handle->ll_handle, buf, (uint16_t)in_count);
    if (ret == 0 && out_count)
        *out_count = in_count;
    return ret;
}

uint8_t* serial_drv_read(struct serial_drv_handle_t *handle,
                         uint32_t *out_nbyte)
{
    if (!handle || !handle->ll_handle) return NULL;

    uint16_t rlen = 0;
    uint8_t *buf = handle->ll_handle->fops->read(handle->ll_handle, &rlen);
    if (out_nbyte) *out_nbyte = rlen;
    return buf;
}

int serial_drv_close(struct serial_drv_handle_t **handle)
{
    if (!handle || !(*handle)) return -1;

    if ((*handle)->ll_handle)
        (*handle)->ll_handle->fops->close((*handle)->ll_handle);

    hosted_free(*handle);
    *handle = NULL;
    return 0;
}