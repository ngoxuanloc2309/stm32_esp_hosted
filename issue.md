Received INIT event from ESP
capabilities: 0xe8
Features supported:
ESP board type: 13
Transport active
init_hosted_control_lib ret=0
process_rx: waiting...
SPI_TRANS EXIT: heap_before=68408, heap_after=49064, diff=19344
HDR: ifEXTI: pin=2
_type=2 len=22
process_rx: waiting...
SPI_TRANS EXIT: heap_before=49064, heap_after=49032, diff=32
HDR: ifEXTI: pin=2
_type=6 len=0
HDR: ifEXTI: pin=2
_type=6 len=0
serial_drv_write: in_count=23
serial_drv_write: ret=0
get_tx_buffer: malloc 1600 bytes, heap=48600
get_tx_buffer: sendbuf=0x20012e40
get_tx_buffer: building header
get_tx_buffer: memcpy len=23
get_tx_buffer: checksum
TX header: if_type=2 if_num=0 len=23
get_tx_buffer: done
get_tx_buffer: freeing priv=0x2EXTI: pin=1
000de48 func=0x8003c99
get_tx_buffer: returning sendbuf=0x20012e40
HDR: if_type=EXTI: pin=2
6 len=0
HDR: if_typeEXTI: pin=2
=2 len=66
process_rx: waiting...
SPI_TRANS EXIT: heap_before=47024, heap_after=46944, diff=80
HDR: ifEXTI: pin=2
_type=2 len=287
process_rx: waiting...
SPI_TRANS EXIT: heap_before=46944, heap_after=46648, diff=296
HDR: if_tyEXTI: pin=2
pe=2 len=43
process_rx: waiting...
SPI_TRANS EXIT: heap_before=46648, heap_after=46592, diff=56
MAC: fc:01:2c:f6:5a:ac
set_mac: s_mac addr=0x2000004c
set_mac: done fc:01:2c:f6:5a:ac
serial_drv_write: in_count=23
serial_drv_write: ret=0
EXTI: pin=1
get_tx_buffer: malloc 1600 bytes, heap=48600
get_tx_buffer: sendbuf=0x20012e40
get_tx_buffer: building header
get_tx_buffer: memcpy len=23
get_tx_buffer: checksum
TX header: if_type=2 if_num=0 len=23
get_tx_buffer: done
get_tx_buffer: freeing priv=0x2000de48 func=0x8003c99
get_tx_buffer: returning sendbuf=0x20012e40
HDR: if_type=EXTI: pin=2
6 len=0
HDR: if_typeEXTI: pin=2
=2 len=67
process_rx: waiting...
SPI_TRANS EXIT: heap_before=47024, heap_after=46944, diff=80
HDR: ifEXTI: pin=2
_type=2 len=287
process_rx: waiting...
SPI_TRANS EXIT: heap_before=46944, heap_after=46648, diff=296
HDR: iEXTI: pin=2
f_type=2 len=24
process_rx: waiting...
SPI_TRANS EXIT: heap_before=46648, heap_after=46600, diff=48
WiFi mode STA OK
lwip initializing...
wifi_netif_init: s_mac=fc:01:2c:f6:5a:ac
netif MAC: fc:01:2c:f6:5a:ac
netif up=1 link_up=1
dhcp_start(netif=0x2001eb84) w00
                                dhcp_start(): mallocing new DHCP client
                                                                       dhcp_start(): allocated dhcp
                                                                                                   dhcp_start(): starting DHCP configuration
                                    dhcp_discover()
                                                   transaction id xid(4bb5f646)
                                                                               dhcp_discover: making request
    dhcp_discover: sendto(DISCOVER, IP_ADDR_BROADCAST, LWIP_IANA_PORT_DHCP_SERVER)
                                                                                  ethernet_output: sending packet 0x2001f458
                    low_level_output: len=350
dhcp_discover: deleting()
                         dhcp_discover: SELECTING
                                                 dhcp_discover(): set request timeout 2000 msecs
                                                                                                DHCP started
lwip netif reak: q=0x2000dc68
HCP)
lwIP netif initialized
serial_drv_write: in_count=44
serial_drv_write: ret=0
etharp_timer
            etharp_timer
                        dhcp_fine_tmr(): request timeout
                                                        dhcp_timeout()
                                                                      dhcp_timeout(): restarting discovery
  dhcp_discover()
                 transaction id xid(4bb5f646)
                                             dhcp_discover: making request
                                                                          dhcp_discover: sendto(DISCOVER, IP_ADDR_BROADCAST, LWIP_IANA_PORT_DHCP_SERVER)
                                                ethernet_output: sending packet 0x2001f3b0
                                                                                          low_level_output: len=350
dhcp_discover: deleting()
                         dhcp_discover: SELECTING
                                                 dhcp_discover(): set request timeout 4000 msecs
                                                                                                get_tx_buffer: malloc 1600 bytes, heap=41200
get_tx_buffer: sendbuf=0x200147b8
get_tx_buffer: building header
get_tx_buffer: memcpy len=350
get_tx_buffer: checksum
TX header: if_type=0 if_num=0 len=350
get_tx_buffer: done
get_tx_buffer: freeing priv=0x2000dfa8 func=0x8003c99
get_tx_buffer: returning sendbuf=0x200147b8
HDR: if_type=EXTI: pin=2
6 len=0
EXTI: pin=1
HDR: if_typEXTI: pin=2
HDR: if_type=6 len=0
HDR: iEXTI: pin=2
f_type=2 len=61
process_rx: waiting...
SPI_TRANS EXIT: heap_before=39952, heap_after=39880, diff=72
HDR:EXTI: pin=2
 if_type=0 len=352
STA/AP packet len=352
netdev_rx: q=0x2000dc68 len=352
process_rx: waiting...
netif_input:IT: heap_before=398ethernet_input: dest:0hx:0hx:0hx:0hx:0hx:0hx, src:0hx:0hx:0hx:0hx:0hx:0hx, type:fc
         dhcp_recv(pbuf = 0x2001f3b0) from DHCP server 10.43.137.137 port 67
                                                                            pbuf->len = 310
                                                                                           pbuf->tot_len = 310
      transaction id mismatch reply_msg->xid(24fcaaae)!=dhcp->xid(4bb5f646)
                                                                           netif_input ret=0
STA connected event from ESP32
EXTI: pin=1
get_tx_buffer: malloc 1600 bytes, heap=41560
get_tx_buffer: sendbuf=0x200147b8
get_tx_buffer: building header
get_tx_buffer: memcpy len=44
get_tx_buffer: checksum
TX header: if_type=2 if_num=0 len=44
get_tx_buffer: done
get_tx_buffer: freeing priv=0x2000e1c8 func=0x8003c99
get_tx_buffer: returning sendbuf=0x200147b8
HDR: if_type=EXTI: pin=2
6 len=0
HDR: if_tEXTI: pin=2
HDR: if_type=0 len=352
STA/AP packet len=352
netdev_rx: q=0x2000dc68 len=352
process_rx: waiting...
net_TRANS EXIT: heap_before=400ethernet_input: dest:0hx:0hx:0hx:0hx:0hx:0hx, src:0hx:0hx:0hx:0hx:0hx:0hx, type:fc
         dhcp_recv(pbuf = 0x2001f3b0) from DHCP server 10.43.137.137 port 67
                                                                            pbuf->len = 310
                                                                                           pbuf->tot_len = 310
      transaction id mismatch reply_msg->xid(24fcaaae)!=dhcp->xid(4bb5f646)
                                                                           netif_input ret=0
etharp_timer
            HDR:EXTI: pin=2
 if_type=2 len=43
process_rx: waiting...
SPI_TRANS EXIT: heap_before=39624, heap_after=39952, diff=-328
HDR:EXTI: pin=2
 if_type=2 len=287
process_rx: waiting...
SPI_TRANS EXIT: heap_before=39952, heap_after=39656, diff=296
Connecting to AP: Spotdog123
0get_tx_buffer: building header
get_tx_buffer: memcpy len=350
get_tx_buffer: checksum
TX header: if_type=0 if_num=0 len=350
get_tx_buffer: done
get_tx_buffer: freeing priv=0x20014650 func=0x8003c99
get_tx_buffer: returning sendbuf=0x200147b8
HDR: if_tyEXTI: pin=2
pe=2 len=61
process_rx: waiting...
SPI_TRANS EXIT: heap_before=40072, heap_after=40000, diff=72
HDR: if_EXTI: pin=2
type=6 len=0
EXTI: pin=1
HDR: EXTI: pin=2
if_type=6 len=0
HDR: ifEXTI: pin=2
_type=0 len=352
STA/AP packet len=352
netdev_rx: q=0x2000dc68 len=352
process_rx: waiting...
netif_input:IT: heap_before=400ethernet_input: dest:0hx:0hx:0hx:0hx:0hx:0hx, src:0hx:0hx:0hx:0hx:0hx:0hx, type:fc
         dhcp_recv(pbuf = 0x2001f3b0) from DHCP server 10.43.137.137 port 67
                                                                            pbuf->len = 310
                                                                                           pbuf->tot_len = 310
      skipping option 28 in options
                                   skipping option 43 in options
                                                                searching DHCP_OPTION_MESSAGE_TYPE
                                                                                                  DHCP_OFFER received in DHCP_STATE_SELECTING state
                                           dhcp_handle_offer(netif=0x2001eb84) w00
                                                                                  dhcp_handle_offer(): server 0x89892b0a
                dhcp_handle_offer(): offer for 0x25892b0a
                                                         dhcp_select(netif=0x2001eb84) w00
                                                                                          transaction id xid(4bb5f646)
              ethernet_output: sending packet 0x2001f5d8
                                                        low_level_output: len=350
dhcp_select: REQUESTING
                       dhcp_select(): set request timeout 2000 msecs
                                                                    get_tx_bufnetifsendbuf=0x2001get_tx_buffer: building header
get_tx_buffer: memcpy len=350
get_tx_buffer: checksum
TX header: if_type=0 if_num=0 len=350
get_tx_buffer: done
get_tx_buffer: freeing priv=0x20014650 func=0x8003c99
get_tx_buffer: returning sendbuf=0x200147b8
HDR: if_type=EXTI: pin=2
6 len=0
HDR:EXTI: pin=2
 if_type=0 len=352
STA/AP packet len=352
netdev_rx: q=0x2000dc68 len=352
process_rx: waiting...
netif_input:IT: heap_before=400ethernet_input: dest:0hx:0hx:0hx:0hx:0hx:0hx, src:0hx:0hx:0hx:0hx:0hx:0hx, type:fc
         dhcp_recv(pbuf = 0x2001f3b0) from DHCP server 10.43.137.137 port 67
                                                                            pbuf->len = 310
                                                                                           pbuf->tot_len = 310
      transaction id mismatch reply_msg->xid(24fcaaae)!=dhcp->xid(4bb5f646)
                                                                           netif_input ret=0
etharp_timer
            HDR:EXTI: pin=2
 if_type=0 len=352
STA/AP packet len=352
netdev_rx: q=0x2000dc68 len=352
process_rx: waiting...
netif_input:IT: heap_before=396ethernet_input: dest:0hx:0hx:0hx:0hx:0hx:0hx, src:0hx:0hx:0hx:0hx:0hx:0hx, type:fc
         dhcp_recv(pbuf = 0x2001f3b0) from DHCP server 10.43.137.137 port 67
                                                                            pbuf->len = 310
                                                                                           pbuf->tot_len = 310
      skipping option 28 in options
                                   skipping option 43 in options
                                                                searching DHCP_OPTION_MESSAGE_TYPE
                                                                                                  DHCP_ACK received
           dhcp_check(netif=0x2001eb84) w0
                                          netif_input ret=0
HDR:EXTI: pin=2
 if_type=6 len=0
STA connected event from ESP32
etharp_raw: sending raw ARP packet.
                                   ethernet_output: sending packet 0x2001f3b0
                                                                             low_level_output: len=42
etharp_timer
            EXTI: pin=1
get_tx_buffer: malloc 1600 bytes, heap=41920
get_tx_buffer: sendbuf=0x20014650
get_tx_buffer: building header
get_tx_buffer: memcpy len=42
get_tx_buffer: checksum
TX header: if_type=0 if_num=0 len=42
get_tx_buffer: done
get_tx_buffer: freeing priv=0x2000dfa8 func=0x8003c99
get_tx_buffer: returning sendbuf=0x20014650
etharp_raw: sending raw ARP packet.
                                   ethernet_output: sending packet 0x2001f3b0
                                                                             low_level_output: len=42
HDR: if_type=EXTI: pin=2
6 len=0
etharp_timer
            HDR:EXTI: pin=2
 if_type=0 len=352
STA/AP packet len=352
netdev_rx: q=0x2000dc68 len=352
process_rx: waiting...
netif_input:IT: heap_before=403ethernet_input: dest:0hx:0hx:0hx:0hx:0hx:0hx, src:0hx:0hx:0hx:0hx:0hx:0hx, type:fc
         dhcp_recv(pbuf = 0x2001f3b0) from DHCP server 10.43.137.137 port 67
                                                                            pbuf->len = 310
                                                                                           pbuf->tot_len = 310
      transaction id mismatch reply_msg->xid(24fcaaae)!=dhcp->xid(4bb5f646)
                                                                           netif_input ret=0
HDR:EXTI: pin=2
 if_type=6 len=0
HDR:EXTI: pin=2
 if_type=6 len=0
get_tx_buffer: malloc 1600 bytes, heap=41920
get_tx_buffer: sendbuf=0x20014650
get_tx_buffer: building header
get_tx_buffer: memcpy len=42
get_tx_buffer: checksum
TX header: if_type=0 if_num=0 len=42
get_tx_buffer: done
get_tx_buffer: freeing priv=0x2000dfa8 func=0x8003c99
get_tx_buffer: returning sendbuf=0x20014650
HDR: if_tyEXTI: pin=2
pe=6 len=0
HDR: EXTI: pin=2
if_type=6 len=0
HDR: EXTI: pin=2
if_type=6 len=0
HDR:EXTI: pin=2
 if_type=6 len=0
etharp_timer
            etharp_raw: sending raw ARP packet.
                                               ethernet_output: sending packet 0x2001f3b0
                                                                                         low_level_output: len=42
etharp_timer
            get_tx_buffer: malloc 1600 bytes, heap=41920
get_tx_buffer: sendbuf=0x20014650
get_tx_buffer: building header
get_tx_buffer: memcpy len=42
get_tx_buffer: checksum
TX header: if_type=0 if_num=0 len=42
get_tx_buffer: done
get_tx_buffer: freeing priv=0x2000dfa8 func=0x8003c99
get_tx_buffer: returning sendbuf=0x20014650
HDR: if_typeEXTI: pin=2
=6 len=0
etharp_timer
            HDR: EXTI: pin=2
if_type=6 len=0
HDR:EXTI: pin=2
 if_type=6 len=0
HDR:tEXTI: pin=2
if_type=6 len=0
etharp_raw: sending raw ARP packet.
                                   ethernet_output: sending packet 0x2001f3b0
                                                                             low_level_output: len=42
EXTI: pin=1
get_tx_buffer: malloc 1600 bytes, heap=41920
get_tx_buffer: sendbuf=0x20014650
get_tx_buffer: building header
get_tx_buffer: memcpy len=42
get_tx_buffer: checksum
TX header: if_type=0 if_num=0 len=42
get_tx_buffer: done
get_tx_buffer: freeing priv=0x2000dfa8 func=0x8003c99
get_tx_buffer: returning sendbuf=0x20014650
HDR: if_type=EXTI: pin=2
6 len=0
etharp_timer
            HDR:EXTI: pin=2
 if_type=0 len=352
STA/AP packet len=352
netdev_rx: q=0x2000dc68 len=352
process_rx: waiting...
netif_input:IT: heap_before=403ethernet_input: dest:0hx:0hx:0hx:0hx:0hx:0hx, src:0hx:0hx:0hx:0hx:0hx:0hx, type:fc
         dhcp_recv(pbuf = 0x2001f3b0) from DHCP server 10.43.137.137 port 67
                                                                            pbuf->len = 310
                                                                                           pbuf->tot_len = 310
      transaction id mismatch reply_msg->xid(24fcaaae)!=dhcp->xid(4bb5f646)
                                                                           netif_input ret=0
HDR:EXTI: pin=2
 if_type=6 len=0
HDR:EXTI: pin=2
 if_type=6 len=0
etharp_timer
            etharp_raw: sending raw ARP packet.
                                               ethernet_output: sending packet 0x2001f3b0
                                                                                         low_level_output: len=42
dhcp_bind(netif=0x2001eb84) w00
                               dhcp_bind(): t0 renewal timer 3599 secs
                                                                      dhcp_bind(): set request timeout 3599000 msecs
            dhcp_bind(): t1 renewal timer 1799 secs
                                                   dhcp_bind(): set request timeout 1799000 msecs
                                                                                                 dhcp_bind(): t2 rebind timer 3149 secs
                               dhcp_bind(): set request timeout 3149000 msecs
                                                                             dhcp_bind(): IP: 0x25892b0a SN: 0x00ffffff GW: 0x89892b0a
                              DHCP got IP: 10.43.137.37
GW:  10.43.137.137
NM:  255.255.255.0
Network up!
Connecting to MQTT broker broker.hivemq.com:1883...
etharp_find_entry: found empty entry 0
                                      etharp_find_entry: selecting empty entry 0
                                                                                etharp_request: sending ARP request.
            etharp_raw: sending raw ARP packet.
                                               ethernet_output: sending packet 0x2001f760
                                                                                         low_level_output: len=42
etharp_query: queued packet 0x2001f6d0 on ARP entry 0
                                                     etharp_timer
                                                                 etharp_request: sending ARP request.
                                                                                                     etharp_raw: sending raw ARP packet.
                                ethernet_output: sending packet 0x2001f750
                                                                          low_level_output: len=42
etharp_find_entry: found matching entry 0
                                         etharp_query: queued packet 0x2001f750 on ARP entry 0
                                                                                              get_tx_buffer: malloc 1600 bytes, heap=37920
get_tx_buffer: sendbuf=0x20015658
get_tx_buffer: building header
get_tx_buffer: memcpy len=42
get_tx_buffer: checksum
TX header: if_type=0 if_num=0 len=42
get_tx_buffer: done
get_tx_buffer: freeing priv=0x2000dfa8 func=0x8003c99
get_tx_buffer: returning sendbuf=0x20015658
HDR: if_type=EXTI: pin=2
6 len=0
HDR:nEXTI: pin=2
if_type=6 len=0
HDR: ifEXTI: pin=2
_type=6 len=0
HDR:EXTI: pin=2
 if_type=6 len=0
etharp_timer
            etharp_request: sending ARP request.
                                                etharp_raw: could not allocate pbuf for ARP request.
                                                                                                    EXTI: pin=1
get_tx_buffer: malloc 1600 bytes, heap=37976
get_tx_buffer: sendbuf=0x20015658
get_tx_buffer: building header
get_tx_buffer: memcpy len=42
get_tx_buffer: checksum
TX header: if_type=0 if_num=0 len=42
get_tx_buffer: done
get_tx_buffer: freeing priv=0x2000dce0 func=0x8003c99
get_tx_buffer: returning sendbuf=0x20015658
etharp_timer
            etharp_request: sending ARP request.
                                                etharp_raw: could not allocate pbuf for ARP request.
                                                                                                    HDR: if_type=EXTI: pin=2
6 len=0
HDR: EXTI: pin=2
HDR: if_type=0 len=352
STA/AP packet len=352
netdev_rx: q=0x2000dc68 len=352
process_rx: waiting...
nPI_TRANS EXIT: heap_before=364HDREXTI: pin=2
: if_type=0 len=42
STA/AP packet len=42
netdev_rx: q=0x2000dc68 len=42