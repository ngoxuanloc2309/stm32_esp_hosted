In the location : Host/libs/esp-hosted/esp_hosted_fg/host/control_lib/src/ctrl_core.c (+ line 15)::

1.  Change from: #define command_log(...)             printf(__VA_ARGS__); printf("\r");
    To: #define command_log(...)             do { printf(__VA_ARGS__); printf("\r"); } while(0)

In the location: Host/libs/esp-hosted/esp_hosted_fg/host/stm32/port/include/platform_wrapper.h (+ line 52)::

2.  Change from: int control_path_platform_init(void);
    To: int control_path_platform_init(struct serial_drv_handle_t *serial_drv_handle);

In the location: Host/libs/esp-hosted/esp_hosted_fg/host/virtual_serial_if/src/serial_if.c (line 193)

3. Clear: "mem_free(write_buf);"

4. slave_control.c:
if (net_link_up) {
    // esp_netif_up(slave_sta_netif);
    esp_netif_action_connected(slave_sta_netif, NULL, 0, NULL);
} else {
    // esp_netif_down(slave_sta_netif);
    esp_netif_action_disconnected(slave_sta_netif, NULL, 0, NULL);
}