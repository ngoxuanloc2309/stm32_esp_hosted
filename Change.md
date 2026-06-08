1. host.mk

Xóa path sai stm32/virtual_serial_if/include, giữ virtual_serial_if/include
Thêm vào HOST_ESP_FILES: esp_queue.c, serial_if.c
Xóa control.c khỏi HOST_ESP_FILES
Xóa include path stm32/app/control khỏi HOST_ESP_INCLUDES
Thêm PORT_FILES: port_serial_drv.c
Thêm HOST_CFLAGS: -DBSSID_LENGTH=18

2. platform_wrapper.h (trong libs)

Đổi signature control_path_platform_init(void) → control_path_platform_init(struct serial_drv_handle_t *serial_drv_handle)

3. port_os.c

Đổi signature control_path_platform_init match với header trên

4. Host/App/app.c

Xóa #include "control.h"
Xóa control_event_handler
Thay control_path_init(control_event_handler) → control_path_platform_init(NULL)

5. Tạo mới Host/Port/src/port_serial_drv.c

Implement serial_drv_open/write/read/close wrap xuống serial_ll_if