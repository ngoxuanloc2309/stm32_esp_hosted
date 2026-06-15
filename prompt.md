Chào bạn, tôi đang làm dự án esp32-hosted với phần cứng gồm stm32f407vgt6 discovery, esp32c6 super mini. Hiện tôi đang lấy stm32 làm host, hiện tại cần kết nối wifi connect mqtt thôi. Nếu tôi chỉ chạy trên stm32 còn esp32 chạy lwip mạng các thứ thì đã xong rồi, nhưng bây giờ tôi muốn stm32 chạy lwip, stm32 tự có ip riêng như 1 device mạng, còn esp32 chỉ đóng vai trò là 1 card wifi thôi. Đây là link repo: https://github.com/espressif/esp-hosted.git ;;; Tôi đang dùng folder là esp_hosted_fg. Dự án của tôi đây: 

----Đây là cấu trúc dự án-----

PS E:\prj_canhan\remote\stm32_esp_hosted> ls


    Directory: E:\prj_canhan\remote\stm32_esp_hosted


Mode                 LastWriteTime         Length Name                                                                                                                                                             
----                 -------------         ------ ----                                                                                                                                                             
d-----          6/8/2026   9:07 PM                .vscode                                                                                                                                                          
d-----         6/14/2026   8:42 PM                build                                                                                                                                                            
d-----          6/8/2026   9:07 PM                Core                                                                                                                                                             
d-----          6/8/2026   9:07 PM                Drivers                                                                                                                                                          
d-----         6/14/2026   3:16 PM                Host                                                                                                                                                             
d-----          6/8/2026   9:07 PM                Middlewares                                                                                                                                                      
-a----          6/8/2026   9:07 PM              5 .gitignore                                                                                                                                                       
-a----          6/8/2026   9:07 PM            236 .gitmodules                                                                                                                                                      
-a----          6/8/2026   9:07 PM          13961 .mxproject                                                                                                                                                       
-a----          6/8/2026  11:09 PM            861 Change.md                                                                                                                                                        
-a----         6/14/2026   3:16 PM           6955 Makefile                                                                                                                                                         
-a----         6/10/2026  10:25 PM            687 Readme.md                                                                                                                                                        
-a----          6/8/2026   9:07 PM          23366 startup_stm32f407xx.s                                                                                                                                            
-a----          6/8/2026   9:07 PM           7300 STM32F407XX_FLASH.ld                                                                                                                                             
-a----         6/10/2026  10:25 PM           6334 test_spi.ioc                                                                                                                                                     


PS E:\prj_canhan\remote\stm32_esp_hosted> cd .\Host\
PS E:\prj_canhan\remote\stm32_esp_hosted\Host> ls


    Directory: E:\prj_canhan\remote\stm32_esp_hosted\Host


Mode                 LastWriteTime         Length Name                                                                                                                                                             
----                 -------------         ------ ----                                                                                                                                                             
d-----         6/10/2026  10:25 PM                add_in_esp32                                                                                                                                                     
d-----         6/10/2026  10:25 PM                add_on_stm32                                                                                                                                                     
d-----         6/14/2026   3:16 PM                App                                                                                                                                                              
d-----         6/14/2026   3:16 PM                Components                                                                                                                                                       
d-----          6/8/2026   9:07 PM                libs                                                                                                                                                             
d-----          6/8/2026   9:07 PM                Port                                                                                                                                                             
d-----          6/8/2026   9:07 PM                Services                                                                                                                                                         
-a----         6/14/2026   3:16 PM           5092 host.mk                                                                                                                                                          


PS E:\prj_canhan\remote\stm32_esp_hosted\Host> cd App            
PS E:\prj_canhan\remote\stm32_esp_hosted\Host\App> tree /F
Folder PATH listing
Volume serial number is 0000002A 02B5:5D80
E:.
│   app.c
│   app.h
│   app_config.h
│   board_config.h
│   
└───user
    ├───user_ble
    │       user_ble.c
    │       user_ble.h
    │       
    └───user_mqtt
            user_mqtt.c
            user_mqtt.h
            
PS E:\prj_canhan\remote\stm32_esp_hosted\Host\App> cd ..\Components\
PS E:\prj_canhan\remote\stm32_esp_hosted\Host\Components> tree /F
Folder PATH listing
Volume serial number is 0000007E 02B5:5D80
E:.
├───network
│   ├───include
│   │       lwipopts.h
│   │       wifi_netif.h
│   │       
│   └───src
│           wifi_netif.c
│           
└───trans_spi
    ├───include
    │       netdev_ext.h
    │       spi_drv.h
    │       transport_drv.h
    │       
    └───src
            ctrl_core_wrapper.c
            netdev_if.c
            spi_drv.c
            transport_drv.c
            
PS E:\prj_canhan\remote\stm32_esp_hosted\Host\Components> cd ..\Services\
PS E:\prj_canhan\remote\stm32_esp_hosted\Host\Services> tree /F
Folder PATH listing
Volume serial number is 00000014 02B5:5D80
E:.
├───ble
│       service_ble.c
│       service_ble.h
│       
└───mqtt
        service_mqtt.c
        service_mqtt.h
        
PS E:\prj_canhan\remote\stm32_esp_hosted\Host\Services> cd ..\Port\
PS E:\prj_canhan\remote\stm32_esp_hosted\Host\Port> tree /F
Folder PATH listing
Volume serial number is 00000034 02B5:5D80
E:.
├───include
│   │   ctrl_core_fix.h
│   │   netdev_if.h
│   │   port_gpio.h
│   │   port_log.h
│   │   port_os.h
│   │   port_spi.h
│   │   
│   ├───arch
│   │       sys_arch.h
│   │       
│   └───port_lwip
│           mem.h
│           platform_wrapper_fix.h
│           
└───src
        port_gpio.c
        port_log.c
        port_os.c
        port_serial_drv.c
        port_spi.c
        sys_arch.c
        
PS E:\prj_canhan\remote\stm32_esp_hosted\Host\Port> cd ..
PS E:\prj_canhan\remote\stm32_esp_hosted\Host> 

Đây là config bên esp32: 
(Top) → Example Configuration                                                                                                                                                                                       
                                                                                 Espressif IoT Development Framework Configuration                                                                                  
    Transport layer (SPI interface)  --->                                                                                                                                                                           
    Transport Priority Queue Configuration  --->                                                                                                                                                                    
    SPI Full-duplex Configuration  --->                                                                                                                                                                             
    ESP-Hosted Task config  --->                                                                                                                                                                                    
[*] Enable Mempool                                                                                                                                                                                                  
    Hosted Debugging  --->                                                                                                                                                                                          
[*] Allow Network Split using packet port number                                                                                                                                                                    
        Network split mode configuration  --->                                                                                                                                                                      
[ ] Allow host to enter deep sleep. Slave will wakeup host using GPIO                                                                                                                                               
[*] Start CLI at slave                                                                                                                                                                                              
    Wi-Fi control (Host manages Wi-Fi)  --->                                                                                                                                                                        
    Wi-Fi Default Example config  --->                                                                                                                                                                              
[ ] Additional higher layer examples to run                                                                                                                                                                         
[*] Peer data transfer echo example            

;;;;
(Top) → Example Configuration → Allow Network Split using packet port number → Network split mode configuration                                                                                                     
                                                                                 Espressif IoT Development Framework Configuration                                                                                  
    Slave side (local) LWIP port range (static)  --->                                                                                                                                                               
    Host side (remote) LWIP port range (static)  --->                                                                                                                                                               
[*] Extra port forwarding to host (static)                                                                                                                                                                          
        Host Static Port Forwarding  --->                                                                                                                                                                           
    Destination LWIP for unfiltered packet (Send packet to host LWIP)  --->    

;;;;
(Top) → Example Configuration → Allow Network Split using packet port number → Network split mode configuration → Destination LWIP for unfiltered packet                                                            
                                                                                 Espressif IoT Development Framework Configuration                                                                                  
( ) Send packet to slave LWIP                                                                                                                                                                                       
(X) Send packet to host LWIP                                                                                                                                                                                        
( ) Send packet to both LWIPs 
;;;
(Top) → Example Configuration → Allow Network Split using packet port number → Network split mode configuration → Extra port forwarding to host (static) → Host Static Port Forwarding                              
                                                                                 Espressif IoT Development Framework Configuration                                                                                  
(22,8554) TCP source ports to forward to host (comma separated)                                                                                                                                                     
(22,80,443,8080,8554,1883) TCP destination ports to forward to host (comma separated)                                                                                                                               
(53,67,68) UDP source ports to allowed from host (comma separated)                                                                                                                                                  
(53,67,68,123) UDP destination ports to forward to host (comma separated)            

Bạn chỉ cần trả lời bằng text, ko cần trả lời bằng ảnh và trả lời lan man để tiết kiệm token. Đồng thời chúng ta hiện tại sẽ ở mode trò chuyện trao đổi, khi nào cần code tôi sẽ bảo mode code (ở mode code bạn sẽ code nhưng ko được comment tiếng Việt, và các kí tự đặc biệt, chỉ comment tiếng Anh và comment ít). Bạn đã rõ chưa để tôi gửi các file code tầng App, Components, port để bạn hiểu thêm.