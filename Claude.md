Chào bạn, tôi đang làm dự án stm32f407vgt6 discovery với esp32c6supermini. Tôi dùng github: "https://github.com/espressif/esp-hosted" ;; "https://github.com/espressif/esp-hosted.git" 

- Prj nằm trong thư mục test_spi

-Project hiện tại gồm các lớp:
    + App: Tầng cao nhất 
    + Components: Gồm các file raw api 
    + libs: Các repo github mà tôi clone về
    + Port: Các file mà tôi port từ lib ra để có thể sử dụng
    + Service: Là tầng giữa app - components

-Hiện tại code tôi viết đã giúp esp32 - stm32 giao tiếp với nhau được qua spi và nó đã lấy được MAC, đây là log: 
########LOG########
TT: polling response HS=1 DR=1
spi_tx_v2 ENTER
SPI_TX: heap=68408
SPI_TX: heap=68408
rxbuff=0x2000dc58
txbuff=0x2000e2a0
CS LOW - starting transfer
transfer done
HDR: if_type=4 len=21
Received INIT event from ESP
capabilities: 0xe8
Features supported:
ESP board type: 13
Transport active
Calling init_hosted_control_lib...
init_hosted_control_lib ret=0
process_rx: waiting...
queued to from_slave OK
spi_tx_v2 ENTER
SPI_TX: heap=49072
SPI_TX: heap=49072
rxbuff=0x2000dc58
txbuff=0x20012e28
CS LOW - starting transfer
transfer done
HDR: if_type=2 len=22
process_rx: waiting...
queued to from_slave OK
serial_drv_read: waiting for data...
serial_drv_read: fops->read returned rlen=22
serial_drv_read: proto_len=8 offset=14
DBG: calling process_ctrl_rx_msg
DBG: process_ctrl_rx_msg returned
serial_drv_read: waiting for data...
Sending wifi_get_mac request...
Calling wifi_get_mac...
serial_drv_write: in_count=23
serial_drv_write: ret=0
TT: polling response HS=1 DR=1
get_tx_buffer: malloc 1600 bytes, heap=48696
get_tx_buffer: sendbuf=0x20012e68
get_tx_buffer: memset
get_tx_buffer: building header
get_tx_buffer: memcpy len=23
get_tx_buffer: checksum
get_tx_buffer: done
get_tx_buffer: freeing priv=0x2000ddc0 func=0x8003bdd
get_tx_buffer: returning sendbuf=0x20012e68
check_exec: HS ready, calling SPI
spi_tx_v2 ENTER
SPI_TX: heap=47120
SPI_TX: heap=47120
rxbuff=0x200134b0
CS LOW - starting transfer
transfer done
HDR: if_type=6 len=0
spi_tx_v2 ENTER
SPI_TX: heap=48728
SPI_TX: heap=48728
rxbuff=0x20012e68
txbuff=0x200134b0
CS LOW - starting transfer
transfer done
HDR: if_type=2 len=63
process_rx: waiting...
queued to from_slave OK
TT: polling response HS=1 DR=1
spi_tx_v2 ENTER
SPI_TX: heap=48656
SPI_TX: heap=48656
rxbuff=0x20012e68
txbuff=0x200134b0
CS LOW - starting transfer
transfer done
HDR: if_type=2 len=43
process_rx: waiting...
queued to from_slave OK
serial_drv_read: fops->read returned rlen=63
serial_drv_read: proto_len=49 offset=14
DBG: calling process_ctrl_rx_msg
DBG: process_ctrl_rx_msg returned
serial_drv_read: waiting for data...
serial_drv_read: fops->read returned rlen=43
serial_drv_read: proto_len=29 offset=14
DBG: calling process_ctrl_rx_msg
wifi_get_mac returned: resp=0x2000de58
ESP32-C6 STA MAC: fc:01:2c:f6:5a:ac
DBG: process_ctrl_rx_msg returned
serial_drv_read: waiting for data...
TT: polling response HS=1 DR=1
spi_tx_v2 ENTER
SPI_TX: heap=48912
SPI_TX: heap=48912
rxbuff=0x20012e68
txbuff=0x200134b0
CS LOW - starting transfer
transfer done
HDR: if_type=6 len=0
spi_tx_v2 ENTER
SPI_TX: heap=48912
SPI_TX: heap=48912
rxbuff=0x20012e68
txbuff=0x200134b0
CS LOW - starting transfer
transfer done
HDR: if_type=2 len=63
process_rx: waiting...
queued to from_slave OK
serial_drv_read: fops->read returned rlen=63
serial_drv_read: proto_len=49 offset=14
DBG: calling process_ctrl_rx_msg
DBG: process_ctrl_rx_msg returned
serial_drv_read: waiting for data...
TT: polling response HS=1 DR=1
spi_tx_v2 ENTER
SPI_TX: heap=48840
SPI_TX: heap=48840
rxbuff=0x20012e68
txbuff=0x200134b0
CS LOW - starting transfer
transfer done
HDR: if_type=6 len=0
spi_tx_v2 ENTER
SPI_TX: heap=48840
SPI_TX: heap=48840
rxbuff=0x20012e68
txbuff=0x200134b0
CS LOW - starting transfer
transfer done
HDR: if_type=2 len=63
process_rx: waiting...
queued to from_slave OK
serial_drv_read: fops->read returned rlen=63
serial_drv_read: proto_len=49 offset=14
DBG: calling process_ctrl_rx_msg
DBG: process_ctrl_rx_msg returned
serial_drv_read: waiting for data...
TT: polling response HS=1 DR=1
spi_tx_v2 ENTER
SPI_TX: heap=48768
SPI_TX: heap=48768
rxbuff=0x20012e68
txbuff=0x200134b0
CS LOW - starting transfer
transfer done
HDR: if_type=6 len=0
spi_tx_v2 ENTER
SPI_TX: heap=48768
SPI_TX: heap=48768
rxbuff=0x20012e68
txbuff=0x200134b0
CS LOW - starting transfer
transfer done
HDR: if_type=2 len=63
process_rx: waiting...
queued to from_slave OK
serial_drv_read: fops->read returned rlen=63
serial_drv_read: proto_len=49 offset=14
DBG: calling process_ctrl_rx_msg
DBG: process_ctrl_rx_msg returned
serial_drv_read: waiting for data...
TT: polling response HS=1 DR=1
spi_tx_v2 ENTER
SPI_TX: heap=48696
SPI_TX: heap=48696
rxbuff=0x20012e68
txbuff=0x200134b0
CS LOW - starting transfer
transfer done
HDR: if_type=6 len=0
spi_tx_v2 ENTER
SPI_TX: heap=48696
SPI_TX: heap=48696
rxbuff=0x20012e68
txbuff=0x200134b0
CS LOW - starting transfer
transfer done
HDR: if_type=2 len=63
process_rx: waiting...
queued to from_slave OK
serial_drv_read: fops->read returned rlen=63
serial_drv_read: proto_len=49 offset=14
DBG: calling process_ctrl_rx_msg
DBG: process_ctrl_rx_msg returned
serial_drv_read: waiting for data...
########LOG########


* Thắc mắc và yêu cầu:
== Fix từng vấn đề trước, xong vấn đề này tôi chạy test lại nếu ok mới chuyển sang gạch đầu dòng 2
- Tuy nhiên tôi để ý kĩ thì hình như có vẻ nó cứ bị 72 bytes leak, mà tôi chưa rõ nguyên nhân (chưa fix được nhưng chúng ta sẽ để sau)

- Sau đó khi lấy được MAC rồi ko hiểu sao nó cứ tiếp tục poll để giao tiếp (DONE: Lý do mà copilot đưa ra hợp lý: Vấn đề 2: Continuous polling
Sau khi lấy MAC, hệ thống vẫn loop polling - đây là hành vi bình thường nếu bạn chưa gửi lệnh tiếp theo)

- Và tôi đang muốn stm32 host để esp32 connect đến wifi được config trong app_config.h (Xử lý vấn đề này)