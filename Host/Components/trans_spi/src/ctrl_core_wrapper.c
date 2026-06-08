#include "cmsis_os.h"
#include "common.h"

// redefine before including ctrl_core.c
#define command_log(...) do { printf(__VA_ARGS__); printf("\r"); } while(0)

#include "../../../../../../Host/libs/esp-hosted/esp_hosted_fg/host/control_lib/src/ctrl_core.c"