/*sys_arch.c*/
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/stats.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#if !NO_SYS

static SemaphoreHandle_t s_lwip_sys_mutex = NULL;

/* --- sys_init --- */
void sys_init(void)
{
    s_lwip_sys_mutex = xSemaphoreCreateRecursiveMutex();
    LWIP_ASSERT("sys_init: mutex failed", s_lwip_sys_mutex != NULL);
}

/* --- Mailbox --- */
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
    *mbox = xQueueCreate((UBaseType_t)size, sizeof(void *));
    if (*mbox == NULL) return ERR_MEM;
    return ERR_OK;
}

void sys_mbox_free(sys_mbox_t *mbox)
{
    vQueueDelete(*mbox);
}

void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
    while (xQueueSendToBack(*mbox, &msg, portMAX_DELAY) != pdTRUE);
}

err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
    if (xQueueSendToBack(*mbox, &msg, 0) == pdTRUE) return ERR_OK;
    return ERR_MEM;
}

err_t sys_mbox_trypost_fromisr(sys_mbox_t *mbox, void *msg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (xQueueSendToBackFromISR(*mbox, &msg, &xHigherPriorityTaskWoken) == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        return ERR_OK;
    }
    return ERR_MEM;
}

u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
    void *dummy;
    if (msg == NULL) msg = &dummy;

    TickType_t start = xTaskGetTickCount();

    if (timeout == 0) {
        while (xQueueReceive(*mbox, msg, portMAX_DELAY) != pdTRUE);
        return (u32_t)(xTaskGetTickCount() - start);
    }

    if (xQueueReceive(*mbox, msg, pdMS_TO_TICKS(timeout)) == pdTRUE)
        return (u32_t)(xTaskGetTickCount() - start);

    *msg = NULL;
    return SYS_ARCH_TIMEOUT;
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    void *dummy;
    if (msg == NULL) msg = &dummy;
    if (xQueueReceive(*mbox, msg, 0) == pdTRUE) return 0;
    return SYS_MBOX_EMPTY;
}

int sys_mbox_valid(sys_mbox_t *mbox)
{
    return (*mbox != SYS_MBOX_NULL) ? 1 : 0;
}

void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
    *mbox = SYS_MBOX_NULL;
}

/* --- Semaphore --- */
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
    *sem = xSemaphoreCreateCounting(0xFFFF, (UBaseType_t)count);
    if (*sem == NULL) return ERR_MEM;
    return ERR_OK;
}

u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
    TickType_t start = xTaskGetTickCount();
    TickType_t ticks = (timeout == 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout);

    if (xSemaphoreTake(*sem, ticks) == pdTRUE)
        return (u32_t)(xTaskGetTickCount() - start);

    return SYS_ARCH_TIMEOUT;
}

void sys_sem_signal(sys_sem_t *sem)
{
    xSemaphoreGive(*sem);
}

void sys_sem_free(sys_sem_t *sem)
{
    vSemaphoreDelete(*sem);
}

int sys_sem_valid(sys_sem_t *sem)
{
    return (*sem != SYS_SEM_NULL) ? 1 : 0;
}

void sys_sem_set_invalid(sys_sem_t *sem)
{
    *sem = SYS_SEM_NULL;
}

/* --- Mutex --- */
err_t sys_mutex_new(sys_mutex_t *mutex)
{
    *mutex = xSemaphoreCreateRecursiveMutex();
    if (*mutex == NULL) return ERR_MEM;
    return ERR_OK;
}

void sys_mutex_lock(sys_mutex_t *mutex)
{
    xSemaphoreTakeRecursive(*mutex, portMAX_DELAY);
}

void sys_mutex_unlock(sys_mutex_t *mutex)
{
    xSemaphoreGiveRecursive(*mutex);
}

void sys_mutex_free(sys_mutex_t *mutex)
{
    vSemaphoreDelete(*mutex);
}

/* --- Thread --- */
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread,
                             void *arg, int stacksize, int prio)
{
    TaskHandle_t h = NULL;
    xTaskCreate(thread, name, (uint16_t)(stacksize / sizeof(StackType_t)),
                arg, (UBaseType_t)prio, &h);
    return h;
}

/* --- Critical section --- */
sys_prot_t sys_arch_protect(void)
{
    xSemaphoreTakeRecursive(s_lwip_sys_mutex, portMAX_DELAY);
    return (sys_prot_t)1;
}

void sys_arch_unprotect(sys_prot_t pval)
{
    (void)pval;
    xSemaphoreGiveRecursive(s_lwip_sys_mutex);
}

/* --- Ticks --- */
u32_t sys_now(void)
{
    return (u32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

#endif /* !NO_SYS */