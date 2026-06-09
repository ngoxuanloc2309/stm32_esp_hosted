#include "cmsis_os.h"
#include "transport_drv.h"
#include "ctrl_api.h"
#include "platform_wrapper.h"
#include "trace.h"
#include "app.h"
#include "app_config.h"

static osThreadId app_task_id = 0;
static volatile uint8_t transport_active = 0;

static void app_task(void const *arg);
static void transport_event_handler(uint8_t event);

static void transport_event_handler(uint8_t event)
{
    switch (event) {
        case TRANSPORT_ACTIVE:
            printf("Transport active\r\n");
            transport_active = 1;
            printf("Calling init_hosted_control_lib...\r\n");
            int ret = init_hosted_control_lib();
            printf("init_hosted_control_lib ret=%d\r\n", ret);
            break;
        default:
            break;
    }
}

static void app_task(void const *arg)
{
    ctrl_cmd_t *req  = NULL;
    ctrl_cmd_t *resp = NULL;

    while (!transport_active)
        osDelay(100);

    osDelay(2000);

    printf("Sending wifi_get_mac request...\r\n");

    req = (ctrl_cmd_t *)hosted_calloc(1, sizeof(ctrl_cmd_t));
    if (!req) {
        printf("Failed to alloc ctrl_cmd_t\r\n");
        goto cleanup;
    }

    req->msg_type        = CTRL_REQ;
    req->msg_id          = CTRL_REQ_GET_MAC_ADDR;
    req->u.wifi_mac.mode = WIFI_MODE_STA;
    req->cmd_timeout_sec = DEFAULT_CTRL_RESP_TIMEOUT;

    printf("Calling wifi_get_mac...\r\n");
    resp = wifi_get_mac(req);
    printf("wifi_get_mac returned: resp=%p\r\n", resp);

    if (!resp || resp->resp_event_status != SUCCESS) {
        printf("Failed, status=%d\r\n", resp ? resp->resp_event_status : -1);
        goto cleanup;
    }

    printf("ESP32-C6 STA MAC: %s\r\n", resp->u.wifi_mac.mac);

cleanup:
    CLEANUP_CTRL_MSG(resp);
    if (req) hosted_free(req);
    vTaskSuspend(NULL);
}

void app_main(void)
{
    printf("Starting STM32 app...\r\n");
    printf("Initializing transport...\r\n");
    transport_init(transport_event_handler);
    printf("Inited transport\r\n");
    osThreadDef(app_thread, app_task, osPriorityNormal, 0, APP_TASK_STACK_SIZE);
    app_task_id = osThreadCreate(osThread(app_thread), NULL);
    assert(app_task_id);
}