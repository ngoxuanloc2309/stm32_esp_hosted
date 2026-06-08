# -------------------------------------------------------
# Host layer build configuration
# -------------------------------------------------------

HOST_DIR = Host

# -------------------------------------------------------
# ESP-Hosted-FG library paths
# -------------------------------------------------------
HOST_ESP_DIR = $(HOST_DIR)/libs/esp-hosted/esp_hosted_fg/host

HOST_ESP_FILES = \
$(HOST_ESP_DIR)/stm32/common/common.c \
$(HOST_ESP_DIR)/stm32/common/util.c \
$(HOST_ESP_DIR)/stm32/common/stats.c \
$(HOST_ESP_DIR)/control_lib/src/ctrl_api.c \
$(HOST_ESP_DIR)/control_lib/src/ctrl_core.c \
$(HOST_ESP_DIR)/stm32/driver/transport/transport_drv.c \
$(HOST_ESP_DIR)/stm32/virtual_serial_if/src/serial_ll_if.c \
$(HOST_ESP_DIR)/stm32/netif/src/netdev_if.c

HOST_ESP_INCLUDES = \
-I$(HOST_ESP_DIR)/stm32/common \
-I$(HOST_ESP_DIR)/stm32/port/include \
-I$(HOST_ESP_DIR)/stm32/driver/transport \
-I$(HOST_ESP_DIR)/stm32/driver/netif \
-I$(HOST_ESP_DIR)/stm32/virtual_serial_if/include \
-I$(HOST_ESP_DIR)/control_lib/include \
-I$(HOST_ESP_DIR)/control_lib/src/include \
-I$(HOST_DIR)/libs/esp-hosted/esp_hosted_fg/esp/esp_driver/network_adapter/main

# -------------------------------------------------------
# Protobuf
# -------------------------------------------------------
HOST_PROTO_DIR = $(HOST_DIR)/libs/esp-hosted/esp_hosted_fg/host/linux/port/src

HOST_PROTO_FILES = \
$(HOST_ESP_DIR)/../common/protobuf-c/protobuf-c.c \
$(HOST_ESP_DIR)/../common/esp_hosted_config.pb-c.c

HOST_PROTO_INCLUDES = \
-I$(HOST_ESP_DIR)/../common \
-I$(HOST_ESP_DIR)/../common/protobuf-c

# -------------------------------------------------------
# Port
# -------------------------------------------------------
PORT_FILES = \
$(HOST_DIR)/Port/src/port_os.c \
$(HOST_DIR)/Port/src/port_spi.c \
$(HOST_DIR)/Port/src/port_gpio.c \
$(HOST_DIR)/Port/src/port_log.c

PORT_INCLUDES = \
-I$(HOST_DIR)/Port/include

# -------------------------------------------------------
# Components
# -------------------------------------------------------
COMPONENTS_FILES = \
$(HOST_DIR)/Components/trans_spi/src/spi_drv.c \
$(HOST_DIR)/Components/trans_spi/src/transport_drv.c

COMPONENTS_INCLUDES = \
-I$(HOST_DIR)/Components/trans_spi/include

# -------------------------------------------------------
# Services
# -------------------------------------------------------
SERVICES_FILES =

SERVICES_INCLUDES =

# -------------------------------------------------------
# App
# -------------------------------------------------------
APP_FILES = \
$(HOST_DIR)/App/app.c

APP_INCLUDES = \
-I$(HOST_DIR)/App

# -------------------------------------------------------
# Aggregate
# -------------------------------------------------------
HOST_SOURCES = \
$(HOST_ESP_FILES) \
$(HOST_PROTO_FILES) \
$(PORT_FILES) \
$(COMPONENTS_FILES) \
$(SERVICES_FILES) \
$(APP_FILES)

HOST_INCLUDES = \
$(HOST_ESP_INCLUDES) \
$(HOST_PROTO_INCLUDES) \
$(PORT_INCLUDES) \
$(COMPONENTS_INCLUDES) \
$(SERVICES_INCLUDES) \
$(APP_INCLUDES)