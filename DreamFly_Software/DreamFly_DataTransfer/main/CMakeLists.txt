idf_component_register(SRCS "usart.c" "wifi.c" "led.c" "app_main.c"
                       PRIV_REQUIRES spi_flash
                       REQUIRES driver
                                esp_wifi
                                nvs_flash
                       INCLUDE_DIRS "")
