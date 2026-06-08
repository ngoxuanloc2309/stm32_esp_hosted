In the location : Host/libs/esp-hosted/esp_hosted_fg/host/control_lib/src/ctrl_core.c (+ line 15)::

Change from: #define command_log(...)             printf(__VA_ARGS__); printf("\r");
To: #define command_log(...)             do { printf(__VA_ARGS__); printf("\r"); } while(0)