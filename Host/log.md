TT: poll timeout HS=0 DR=0
EXTI: pin=1
EXTI: pin=2
TT: polling response HS=1 DR=1 elapsed=947
Check and execute spi transaction first
spi_tx_v2 ENTER
SPI_TX: heap=68408
SPI_TX: heap=68408
rxbuff=0x2000dc68
txbuff=0x2000e2b0
CS LOW - starting transfer
transfer done
RX raw 32byte:  04 00 15 00 0C 00 CF 01 00 00 00 00 00 13 02 01 0D 00 01 E8 03 01 00 04 08 46 47 00 01 00 00 00HDR: if_EXTI: pin=2
type=4 len=21
Received INIT event from ESP
capabilities: 0xe8
Features supported:
ESP board type: 13
Transport active
init_hosted_control_lib ret=0
process_rx: waiting...
queued to from_slave OK
SPI_TRANS EXIT: heap_before=68408, heap_after=49064, diff=19344
spi_tx_v2 ENTER
SPI_TX: heap=49064
SPI_TX: heap=49064
rxbuff=0x2000dc68
txbuff=0x20012e40
CS LOW - starting transfer
transfer done
RX raw 32byte:  02 00 16 00 0C 00 50 05 01 00 00 00 01 08 00 63 74 72 6C 45 76 6E 74 02 08 00 08 03 10 AD 02 EAHDR: iEXTI: pin=2
f_type=2 len=22
process_rx: waiting...
queued to from_slave OK
SPI_TRANS EXIT: heap_before=49064, heap_after=49032, diff=32
spi_tx_v2 ENTER
SPI_TX: heap=49032
SPI_TX: heap=49032
rxbuff=0x2000dc68
txbuff=0x20012e80
CS LOW - starting transfer
transfer done
RX raw 32byte:  F6 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00HDR:EXTI: pin=2
RX raw 32byte:  F6 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00HDR: if_type=6 len=0
spi_tx_v2 ENTER
SPI_TX: heap=49032
SPI_TX: heap=49032
rxbuff=0x2000dc68
txbuff=0x20012e80
CS LOW - starting transfer
transfer done
RX raw 32byte:  F6 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00HDR: EXTI: pin=2
if_type=6 len=0
TT: poll timeout HS=1 DR=1
serial_drv_write: in_count=23
serial_drv_write: ret=0
EXTI: pin=1
TT: polling response HS=1 DR=1 elapsed=1826
Check and execute spi transaction first
get_tx_buffer: malloc 1600 bytes, heap=48720
get_tx_buffer: sendbuf=0x20012e40
get_tx_buffer: memset
get_tx_buffer: building header
get_tx_buffer: memcpy len=23
get_tx_buffer: checksum
TX header: if_type=2 if_num=0 len=23
get_tx_buffer: done
get_tx_buffer: freeing priv=0x2000ddd0 func=0x8003c99
get_tx_buffer: returning sendbuf=0x20012e40
check_exec: HS ready, calling SPI
spi_tx_v2 ENTER
SPI_TX: heap=47144
SPI_TX: heap=47144
rxbuff=0x20013488
CS LOW - starting transfer
transfer done
RX raw 32byte:  F6 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00HDR: if_type=EXTI: pin=2
6 len=0
spi_tx_v2 ENTER
SPI_TX: heap=47144
SPI_TX: heap=47144
rxbuff=0x20013488
txbuff=0x20013ad0
CS LOW - starting transfer
transfer done
RX raw 32byte:  02 00 3F 00 0C 00 BF 15 02 00 00 00 01 08 00 63 74 72 6C 45 76 6E 74 02 31 00 08 03 10 AF 02 FAHDR: if_tyEXTI: pin=2
pe=2 len=63
process_rx: waiting...
queued to from_slave OK
SPI_TRANS EXIT: heap_before=47144, heap_after=47072, diff=72
spi_tx_v2 ENTER
SPI_TX: heap=47072
SPI_TX: heap=47072
rxbuff=0x20013488
txbuff=0x20013ad0
CS LOW - starting transfer
transfer done
RX raw 32byte:  02 00 1F 01 0C 00 70 07 03 00 00 00 01 08 00 63 74 72 6C 45 76 6E 74 02 11 01 08 03 10 B4 02 A2HDR: EXTI: pin=2
if_type=2 len=287
process_rx: waiting...
queued to from_slave OK
SPI_TRANS EXIT: heap_before=47072, heap_after=46776, diff=296
spi_tx_v2 ENTER
SPI_TX: heap=46776
SPI_TX: heap=46776
rxbuff=0x20013488
txbuff=0x20013ad0
CS LOW - starting transfer
transfer done
RX raw 32byte:  02 00 2B 00 0C 00 8C 0A 04 00 00 00 01 08 00 63 74 72 6C 52 65 73 70 02 1D 00 08 02 10 C9 01 18HDR:EXTI: pin=2
 if_type=2 len=43
process_rx: waiting...
queued to from_slave OK
SPI_TRANS EXIT: heap_before=46776, heap_after=46720, diff=56
MAC: fc:01:2c:f6:5a:ac
TT: poll timeoue: ret=0
t=2get_tx_buffer: malloc 1600 bytes, heap=48720
get_tx_buffer: sendbuf=0x20012e40
get_tx_buffer: memset
get_tx_buffer: building header
get_tx_buffer: memcpy len=23
get_tx_buffer: checksum
TX header: if_type=2 if_num=0 len=23
get_tx_buffer: done
get_tx_buffer: freeing priv=0x2000ddd0 func=0x8003c99
get_tx_buffer: returning sendbuf=0x20012e40
check_exec: HS ready, calling SPI
spi_tx_v2 ENTER
SPI_TX: heap=47144
SPI_TX: heap=47144
rxbuff=0x20013488
CS LOW - starting transfer
transfer done
RX raw 32byte:  F6 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00HDR: if_typeEXTI: pin=2
EXTI: pin=1
=6 len=0
spi_tx_v2 ENTER
SPI_TX: heap=47144
SPI_TX: heap=47144
rxbuff=0x20013488
txbuff=0x20013ad0
CS LOW - starting transfer
transfer done
RX raw 32byte:  F6 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00HDR: if_typeEXTI: pin=2
=6 len=0
spi_tx_v2 ENTER
SPI_TX: heap=47144
SPI_TX: heap=47144
rxbuff=0x20013488
txbuff=0x20013ad0
CS LOW - starting transfer
transfer done
RX raw 32byte:  02 00 18 00 0C 00 7E 05 05 00 00 00 01 08 00 63 74 72 6C 52 65 73 70 02 0A 00 08 02 10 CC 01 18HDR: EXTI: pin=2
if_type=2 len=24
process_rx: waiting...
queued to from_slave OK
SPI_TRANS EXIT: heap_before=47144, heap_after=47112, diff=32
spi_tx_v2 ENTER
SPI_TX: heap=47112
SPI_TX: heap=47112
rxbuff=0x20013488
txbuff=0x20013ad0
CS LOW - starting transfer
transfer done
RX raw 32byte:  F6 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00HDR: EXTI: pin=2
if_type=6 len=0
WiFi mode STA OK
serial_drv_write: in_count=44
serial_drv_write: ret=0
EXTI: pin=1
TT: polling response HS=1 DR=1 elapsed=1341
Check and execute spi transaction first
get_tx_buffer: malloc 1600 bytes, heap=48696
get_tx_buffer: sendbuf=0x20012e40
get_tx_buffer: memset
get_tx_buffer: building header
get_tx_buffer: memcpy len=44
get_tx_buffer: checksum
TX header: if_type=2 if_num=0 len=44
get_tx_buffer: done
get_tx_buffer: freeing priv=0x2000ddf8 func=0x8003c99
get_tx_buffer: returning sendbuf=0x20012e40
check_exec: HS ready, calling SPI
spi_tx_v2 ENTER
SPI_TX: heap=47144
SPI_TX: heap=47144
rxbuff=0x20013488
CS LOW - starting transfer
transfer done
RX raw 32byte:  F6 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00HDR: if_type=EXTI: pin=2
6 len=0
spi_tx_v2 ENTER
SPI_TX: heap=47144
SPI_TX: heap=47144
rxbuff=0x20013488
txbuff=0x20013ad0
CS LOW - starting transfer
transfer done
RX raw 32byte:  02 00 3F 00 0C 00 C3 15 06 00 00 00 01 08 00 63 74 72 6C 45 76 6E 74 02 31 00 08 03 10 AF 02 FAHDR: if_type=EXTI: pin=2
2 len=63
process_rx: waiting...
queued to from_slave OK
SPI_TRANS EXIT: heap_before=47144, heap_after=47072, diff=72
spi_tx_v2 ENTER
SPI_TX: heap=47072
SPI_TX: heap=47072
rxbuff=0x20013488
txbuff=0x20013ad0
CS LOW - starting transfer
transfer done
RX raw 32byte:  02 00 1F 01 0C 00 74 07 07 00 00 00 01 08 00 63 74 72 6C 45 76 6E 74 02 11 01 08 03 10 B4 02 A2HDR: iEXTI: pin=2
f_type=2 len=287
process_rx: waiting...
queued to from_slave OK
SPI_TRANS EXIT: heap_before=47072, heap_after=46776, diff=296
spi_tx_v2 ENTER
SPI_TX: heap=46776
SPI_TX: heap=46776
rxbuff=0x20013488
txbuff=0x20013ad0
CS LOW - starting transfer
transfer done
RX raw 32byte:  02 00 2B 00 0C 00 D0 0A 08 00 00 00 01 08 00 63 74 72 6C 52 65 73 70 02 1D 00 08 02 10 CF 01 18HDR: iEXTI: pin=2
f_type=2 len=43
process_rx: waiting...
queued to from_slave OK
SPI_TRANS EXIT: heap_before=46776, heap_after=46720, diff=56
Connecting to AP: Spotdog123
Waitinll timeout HS=1 DR=1
ent...
TT: poll timeout HS=1 DR=0
EXTI: pin=1
spi_tx_v2 ENTER
SPI_TX: heap=48752
SPI_TX: heap=48752
rxbuff=0x20012e40
txbuff=0x20013488
CS LOW - starting transfer
transfer done
RX raw 32byte:  F6 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00HDR: if_type=EXTI: pin=2
6 len=0
spi_tx_v2 ENTER
SPI_TX: heap=48752
SPI_TX: heap=48752
rxbuff=0x20012e40
txbuff=0x20013488
CS LOW - starting transfer
transfer done
RX raw 32byte:  02 00 3F 00 0C 00 C6 15 09 00 00 00 01 08 00 63 74 72 6C 45 76 6E 74 02 31 00 08 03 10 AF 02 FAHDR: EXTI: pin=2
if_type=2 len=63
process_rx: waiting...
queued to from_slave OK
SPI_TRANS EXIT: heap_before=48752, heap_after=48680, diff=72
spi_tx_v2 ENTER
SPI_TX: heap=48680
SPI_TX: heap=48680
rxbuff=0x20012e40
txbuff=0x20013488
CS LOW - starting transfer
transfer done
RX raw 32byte:  02 00 1F 01 0C 00 77 07 0A 00 00 00 01 08 00 63 74 72 6C 45 76 6E 74 02 11 01 08 03 10 B4 02 A2HDR: EXTI: pin=2
if_type=2 len=287
process_rx: waiting...
queued to from_slave OK
SPI_TRANS EXIT: heap_before=48680, heap_after=48384, diff=296
spi_tx_v2 ENTER
SPI_TX: heap=48384
SPI_TX: heap=48384
rxbuff=0x20012e40
txbuff=0x20013488
CS LOW - starting transfer
transfer done
RX raw 32byte:  F6 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00HDR:EXTI: pin=2
 if_type=6 len=0
TT: poll timeout HS=1 DR=0
EXTI: pin=1
TT: polling response HS=1 DR=1 elapsed=276
Check and execute spi transaction first
spi_tx_v2 ENTER
SPI_TX: heap=48752
SPI_TX: heap=48752
rxbuff=0x20012e40
txbuff=0x20013488
CS LOW - starting transfer
transfer done
RX raw 32byte:  F6 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00HDR: if_typeEXTI: pin=2
=6 len=0
spi_tx_v2 ENTER
SPI_TX: heap=48752
SPI_TX: heap=48752
rxbuff=0x20012e40
txbuff=0x20013488
CS LOW - starting transfer
transfer done
RX raw 32byte:  02 00 3D 00 0C 00 16 0E 0B 00 00 00 01 08 00 63 74 72 6C 45 76 6E 74 02 2F 00 08 03 10 B1 02 8AHDR:EXTI: pin=2
 if_type=2 len=61
process_rx: waiting...
queued to from_slave OK
SPI_TRANS EXIT: heap_before=48752, heap_after=48680, diff=72
spi_tx_v2 ENTER
SPI_TX: heap=48680
SPI_TX: heap=48680
rxbuff=0x20012e40
txbuff=0x20013488
CS LOW - starting transfer
transfer done
RX raw 32byte:  00 00 60 01 0C 00 A3 27 00 00 00 00 FC 01 2C F6 5A AC 7A F7 3B 77 71 13 08 00 45 00 01 52 79 C3HDR:EXTI: pin=2
 if_type=0 len=352
STA/AP packet len=352