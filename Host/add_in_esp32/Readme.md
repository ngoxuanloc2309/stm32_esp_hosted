This folder is created to port mqtt_handle to esp32-hosted. This includes:
-add_on_stm32: mqtt_shared.h (sub-folder)
-mqtt_handle.h
-mqtt_handle.c

How to port?
In the location: Host/libs/esp-hosted/esp_hosted_fg/esp/esp_driver/network_adapter/main/peer_data_example.c

- include header: #include "mqtt_handler.h"

- Define more config status message:

#define MSG_ID_MQTT_START  	10 /*host -> CP: start MQTT*/
#define MSG_ID_MQTT_PUBLISH 11 /*host -> CP: publish MQTT*/
#define MSG_ID_MQTT_STOP	12 /*host -> CP: stop MQTT*/

- in lines: 71-85 add code:
	/*Add in*/
	case MSG_ID_MQTT_START:
		if (req->data && req->data_len >= sizeof(mqtt_start_cmd_t)) {
			mqtt_handler_start((const mqtt_start_cmd_t *)req->data);
		}
		break;
	case MSG_ID_MQTT_PUBLISH:
		if (req->data && req->data_len >= sizeof(mqtt_publish_cmd_t)) {
			mqtt_handler_publish((const mqtt_publish_cmd_t *)req->data);
		}
		break;
	case MSG_ID_MQTT_STOP:
		mqtt_handler_stop();
		break;
	/*Add in*/

- in the location: Host/libs/esp-hosted/esp_hosted_fg/esp/esp_driver/network_adapter/main/Cmakelists.txt
Delete all files then you need to paste this code:
====================CODE====================== 

set(common_dir "../../../../common")
set(mqtt_addon_dir "../../../../../../../../Host/add_in_esp32")
set(mqtt_shared_dir "../../../../../../../../Host/add_on_stm32")

set(COMPONENT_SRCS
    "slave_control.c"
    "${common_dir}/esp_hosted_config.pb-c.c"
    "${common_dir}/utils/esp_hosted_cli.c"
    "protocomm_pserial.c"
    "esp_hosted_coprocessor.c"
    "slave_bt.c"
    "mempool.c"
    "stats.c"
    "mempool_ll.c"
    "host_power_save.c"
    "nw_split_router.c"
)

if(CONFIG_ESP_HOSTED_COPROCESSOR_EXAMPLE_PEER_DATA)
    list(APPEND COMPONENT_SRCS peer_data_example.c)
    list(APPEND COMPONENT_SRCS "${mqtt_addon_dir}/mqtt_handler.c")
endif()

if(CONFIG_ESP_HOSTED_COPROCESSOR_EXAMPLE_MQTT)
    list(APPEND COMPONENT_SRCS example_mqtt_client.c)
endif()

if(CONFIG_ESP_HOSTED_COPROCESSOR_EXAMPLE_HTTP_CLIENT)
    list(APPEND COMPONENT_SRCS example_http_client.c)
endif()

set(COMPONENT_ADD_INCLUDEDIRS
    "."
    "${common_dir}/include"
    "${common_dir}/utils"
    "${mqtt_addon_dir}"
    "${mqtt_shared_dir}"
)

if(CONFIG_ESP_SDIO_HOST_INTERFACE)
    list(APPEND COMPONENT_SRCS sdio_slave_api.c)
else(CONFIG_ESP_SPI_HOST_INTERFACE)
    list(APPEND COMPONENT_SRCS spi_slave_api.c)
endif()

register_component()
target_compile_definitions(${COMPONENT_LIB} PRIVATE)

# Add directory of protocomm_priv.h to include paths
idf_component_get_property(protocomm_dir protocomm COMPONENT_DIR)
target_include_directories(${COMPONENT_LIB} PRIVATE "${protocomm_dir}/src/common")

====================CODE======================
