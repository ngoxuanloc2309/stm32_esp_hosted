#include "cmsis_os.h"
#include "transport_drv.h"
#include "ctrl_api.h"
#include "platform_wrapper.h"
#include "trace.h"
#include "app.h"
#include "app_config.h"
#include "control.h"

static osThreadId app_task_id = 0;
static volatile uint8_t transport_active = 0;

static void app_task(void const *arg);
static void control_event_handler(uint8_t event);
static void transport_event_handler(uint8_t event);

static void transport_event_handler(uint8_t event)
{
    switch (event) {
        case TRANSPORT_ACTIVE:
            printf("Transport active\n\r");
            transport_active = 1;
            control_path_init(control_event_handler);
            break;
        default:
            break;
    }
}

static void control_event_handler(uint8_t event)
{
    switch (event) {
        case STATION_CONNECTED:
            printf("Station connected\n\r");
            break;
        case STATION_DISCONNECTED:
            printf("Station disconnected\n\r");
            break;
        default:
            break;
    }
}

static void app_task(void const *arg)
{
    ctrl_cmd_t *req  = NULL;
    ctrl_cmd_t *resp = NULL;

    /* wait for transport to be active */
    while (!transport_active)
        osDelay(100);

    osDelay(500);

    /* get MAC address of STA interface */
    req = (ctrl_cmd_t *)hosted_calloc(1, sizeof(ctrl_cmd_t));
    if (!req) {
        printf("Failed to alloc ctrl_cmd_t\n\r");
        goto cleanup;
    }

    req->msg_type      = CTRL_REQ;
    req->msg_id        = CTRL_REQ_GET_MAC_ADDR;
    req->u.wifi_mac.mode = WIFI_MODE_STA;
    req->cmd_timeout_sec = DEFAULT_CTRL_RESP_TIMEOUT;

    resp = wifi_get_mac(req);

    if (!resp || resp->resp_event_status != SUCCESS) {
        printf("Failed to get MAC address\n\r");
        goto cleanup;
    }

    printf("ESP32-C6 STA MAC: %s\n\r", resp->u.wifi_mac.mac);

cleanup:
    CLEANUP_CTRL_MSG(resp);
    if (req) hosted_free(req);

    /* task done, suspend self */
    vTaskSuspend(NULL);
}

void app_main(void)
{
    transport_init(transport_event_handler);

    osThreadDef(app_thread, app_task, osPriorityNormal, 0, APP_TASK_STACK_SIZE);
    app_task_id = osThreadCreate(osThread(app_thread), NULL);
    assert(app_task_id);
}