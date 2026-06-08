In the location : Host/libs/esp-hosted/esp_hosted_fg/host/control_lib/src/ctrl_core.c (+ line 15)::

1.  Change from: #define command_log(...)             printf(__VA_ARGS__); printf("\r");
    To: #define command_log(...)             do { printf(__VA_ARGS__); printf("\r"); } while(0)

In the location: Host/libs/esp-hosted/esp_hosted_fg/host/stm32/port/include/platform_wrapper.h (+ line 52)::

2.  Change from: int control_path_platform_init(void);
    To: int control_path_platform_init(struct serial_drv_handle_t *serial_drv_handle);