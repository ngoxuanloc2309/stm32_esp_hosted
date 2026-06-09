/*port_os.c*/
#include <stdlib.h>
#include <string.h>
#include "cmsis_os.h"
#include "platform_wrapper.h"
#include "serial_ll_if.h"
#include "port_log.h"

#define SEC_TO_MILLISEC(x) (1000*(x))

static osSemaphoreId readSemaphore;
serial_ll_handle_t *serial_ll_if_g;

struct timer_handle_t {
    osTimerId timer_id;
};

void control_path_rx_indication(void)
{
    if (readSemaphore)
        osSemaphoreRelease(readSemaphore);
}

/* --- Memory --- */
void *hosted_malloc(size_t size)      { return malloc(size); }
void  hosted_free(void *ptr)          { if (ptr) free(ptr); }

void *hosted_calloc(size_t blk_no, size_t size)
{
    void *ptr = malloc(blk_no * size);
    if (ptr) memset(ptr, 0, blk_no * size);
    return ptr;
}

void *hosted_realloc(void *mem, size_t newsize)
{
    void *p = NULL;
    if (newsize == 0) { mem_free(mem); return NULL; }
    p = hosted_malloc(newsize);
    if (p && mem) { memcpy(p, mem, newsize); mem_free(mem); }
    return p;
}

/* --- Threads --- */

void *hosted_thread_create(void (*start_routine)(void const *), void *arg)
{
    if (!start_routine) return NULL;
    thread_handle_t *h = hosted_malloc(sizeof(thread_handle_t));
    if (!h) return NULL;

    BaseType_t ret = xTaskCreate(
        (TaskFunction_t)start_routine,
        "ctrl_task",
        CTRL_PATH_TASK_STACK_SIZE,
        arg,
        CTRL_PATH_TASK_PRIO,
        h
    );

    if (ret != pdPASS) {
        hosted_free(h);
        return NULL;
    }
    return h;
}

int hosted_thread_cancel(void *thread_handle)
{
    if (!thread_handle) return -1;
    thread_handle_t *h = (thread_handle_t *)thread_handle;
    int ret = osThreadTerminate(*h);
    mem_free(thread_handle);
    return ret;
}

/* --- Semaphores --- */
void *hosted_create_semaphore(int init_value)
{
    osSemaphoreDef(sem_template_ctrl);
    semaphore_handle_t *s = hosted_malloc(sizeof(semaphore_handle_t));
    if (!s) return NULL;
    *s = osSemaphoreCreate(osSemaphore(sem_template_ctrl), 1);
    if (!(*s)) { mem_free(s); return NULL; }
    return s;
}

int hosted_get_semaphore(void *semaphore_handle, int timeout)
{
    if (!semaphore_handle) return -1;
    semaphore_handle_t *s = (semaphore_handle_t *)semaphore_handle;
    if (!timeout)      return osSemaphoreWait(*s, 0);
    else if (timeout < 0) return osSemaphoreWait(*s, osWaitForever);
    else               return osSemaphoreWait(*s, SEC_TO_MILLISEC(timeout));
}

int hosted_post_semaphore(void *semaphore_handle)
{
    if (!semaphore_handle) return -1;
    return osSemaphoreRelease(*(semaphore_handle_t *)semaphore_handle);
}

int hosted_destroy_semaphore(void *semaphore_handle)
{
    if (!semaphore_handle) return -1;
    semaphore_handle_t *s = (semaphore_handle_t *)semaphore_handle;
    int ret = osSemaphoreDelete(*s);
    mem_free(semaphore_handle);
    return ret;
}

/* --- Timers --- */
void *hosted_timer_start(int duration, int type,
    void (*timeout_handler)(void const *), void *arg)
{
    osTimerDef(timerNew, timeout_handler);
    struct timer_handle_t *h = hosted_malloc(sizeof(struct timer_handle_t));
    if (!h) return NULL;
    os_timer_type ttype = (type == CTRL__TIMER_PERIODIC) ? osTimerPeriodic : osTimerOnce;
    h->timer_id = osTimerCreate(osTimer(timerNew), ttype, arg);
    if (!h->timer_id) { mem_free(h); return NULL; }
    if (osTimerStart(h->timer_id, SEC_TO_MILLISEC(duration)) != osOK) {
        osTimerDelete(h->timer_id);
        mem_free(h);
        return NULL;
    }
    return h;
}

int hosted_timer_stop(void *timer_handle)
{
    if (!timer_handle) return -1;
    struct timer_handle_t *h = (struct timer_handle_t *)timer_handle;
    osTimerStop(h->timer_id);
    osTimerDelete(h->timer_id);
    mem_free(timer_handle);
    return 0;
}

/* --- Sleep --- */
unsigned int sleep(unsigned int seconds)  { osDelay(seconds * 1000); return 0; }
unsigned int msleep(unsigned int mseconds) { osDelay(mseconds); return 0; }

/* --- Control Path Platform --- */

int control_path_platform_init(struct serial_drv_handle_t *serial_drv_handle)
{
    osSemaphoreDef(READSEM);
    readSemaphore = osSemaphoreCreate(osSemaphore(READSEM), 1);
    if (!readSemaphore) return -1;
    if (osSemaphoreWait(readSemaphore, portMAX_DELAY) != osOK) return -1;
    return 0;
}

int control_path_platform_deinit(void)
{
    if (!serial_ll_if_g) return -1;
    return serial_ll_if_g->fops->close(serial_ll_if_g);
}