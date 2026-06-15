Thấy vấn đề rồi. Trong log STM32:
netif_input: got pbuf len=352
Có xuất hiện (dù bị cắt), nhưng không bao giờ thấy tcpip_input ret= sau đó. Nghĩa là print bị overwrite hoàn toàn bởi SPI task.
Nhưng quan trọng hơn, nhìn kỹ dòng này từ ESP32 stats:
Lwip: in[8] slave_out[0] host_out[8] both_out[0]
host_out[8] - ESP32 đã forward 8 packet lên host. Nhưng slave_out[0] - không có packet nào đi ra từ slave lwIP. Điều này confirm DHCP offer đang được forward đúng.
Vấn đề thực sự tôi nghi: STM32 nhận DHCP offer nhưng không gửi DHCP Request lại. Nhìn STM32 log, low_level_output: len=350 chỉ xuất hiện đều đặn - đây là DHCP Discover được retry, không có DHCP Request.
DHCP flow đúng phải là:
Discover → Offer → Request → ACK
STM32 chỉ loop Discover mãi, không chuyển sang Request. Nghĩa là lwIP nhận được Offer nhưng không xử lý được - DHCP state machine không advance.
Root cause thực sự: tcpip_input bị gọi từ wifi_netif_input_task nhưng lwIP DHCP timer không chạy vì wifi_netif_timeout_task đang bị comment out, và tcpip_init thread thì tự handle timeout - nhưng chỉ khi có activity.
Thực ra với tcpip_init(), timeout được handle bởi tcpip thread nội bộ qua LWIP_TCPIP_CORE_LOCKING. Không cần timeout task riêng.
Vấn đề khác: tcpip_input() được gọi từ task ngoài - đây là đúng. Nhưng ethernet_input là input function của netif, và với tcpip_init, bạn phải dùng netif->input = tcpip_input thay vì ethernet_input.
Nhìn lại wifi_netif_low_init - bạn không set netif->input. Còn trong wifi_netif_init:
cnetif_add(&wifi_netif, ..., wifi_netif_low_init, ethernet_input);
Tham số cuối ethernet_input được set làm netif->input. Sau đó trong wifi_netif_input_task bạn gọi tcpip_input(p, &wifi_netif) - đây bypass netif->input và đưa thẳng vào tcpip thread, tcpip thread sẽ gọi netif->input(p, netif) tức là ethernet_input. Về lý thuyết đúng.
Nhưng thực tế vấn đề là: khi dùng tcpip_init, netif_add phải được gọi với input function là tcpip_input, không phải ethernet_input:
c// SAI với tcpip_init:
netif_add(&wifi_netif, ..., wifi_netif_low_init, ethernet_input);

// ĐÚNG với tcpip_init:
netif_add(&wifi_netif, ..., wifi_netif_low_init, tcpip_input);
Và trong wifi_netif_input_task thay vì gọi tcpip_input thủ công, gọi netif->input:
cif (wifi_netif.input(p, &wifi_netif) != ERR_OK)
    pbuf_free(p);
Sửa wifi_netif_init lại:
cif (!netif_add(&wifi_netif, &ipaddr, &netmask, &gw,
               NULL, wifi_netif_low_init, tcpip_input)) {  // đổi ethernet_input -> tcpip_input
Và trong wifi_netif_input_task:
cerr_t err = wifi_netif.input(p, &wifi_netif);  // thay tcpip_input(p, &wifi_netif)
printf("netif_input ret=%d\r\n", err);
if (err != ERR_OK)
    pbuf_free(p);
Thử sửa và chạy lại xem sao.