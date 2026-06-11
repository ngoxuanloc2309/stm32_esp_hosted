#ifndef __SYS_ARCH_H__
#define __SYS_ARCH_H__

#include "lwip/opt.h"

#if (NO_SYS != 0)
#error "NO_SYS need to be set to 0 to use threaded API"
#endif

#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SYS_MBOX_NULL   ((QueueHandle_t)NULL)
#define SYS_SEM_NULL    ((SemaphoreHandle_t)NULL)

typedef SemaphoreHandle_t   sys_sem_t;
typedef SemaphoreHandle_t   sys_mutex_t;
typedef QueueHandle_t       sys_mbox_t;
typedef TaskHandle_t        sys_thread_t;
typedef int                 sys_prot_t;

#ifdef __cplusplus
}
#endif

#endif