/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "driver/uart.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "led.h"
#include "usart.h"
#include "wifi.h"


#define UDP_SERVER_PORT 3333
#define UDP_CLIENT_PORT 3332
#define UDP_CLIENT_IP "192.168.4.2" // UDP 服务端 IP 地址（根据你的需求修改）

static const char *TAG = "example";

static uint8_t led_state;                //声明一个LED灯状态的变量

void uart_rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";

    //设置日志等级为ESP_LOG_INFO
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);

    /*  为接收的数据分配一个缓冲区，大小为 RX_BUF_SIZE + 1 字节。
        多分配的 1 字节用于在接收到的数据末尾添加字符串终止符（\0），以便于打印。  */
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE + 1);

     if (data == NULL) {
        ESP_LOGE(RX_TASK_TAG, "Unable to allocate memory for UART buffer");
        vTaskDelete(NULL); // 结束任务
        return;
    }

     // 初始化 UDP 套接字
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // 创建 UDP 套接字
    if (sock < 0) {
        ESP_LOGE(RX_TASK_TAG, "Unable to create socket: errno %d", errno);
        free(data); // 释放内存
        vTaskDelete(NULL); // 结束任务
        return;
    }
    
    struct sockaddr_in dest_addr1; // 使用 IPv4 地址结构
    dest_addr1.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr1.sin_family = AF_INET;
    dest_addr1.sin_port = htons(UDP_SERVER_PORT);
    // 绑定套接字到指定的地址和端口
    int err = bind(sock, (struct sockaddr *)&dest_addr1, sizeof(dest_addr1));
    if (err < 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
    }
    ESP_LOGI(TAG, "Socket bound, port %d", UDP_SERVER_PORT);


    // 设置目标 UDP 客户端的 IP 地址和端口
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(UDP_CLIENT_PORT); // 设置 UDP 客户端端口
    dest_addr.sin_addr.s_addr = inet_addr(UDP_CLIENT_IP);  // 设置客户端 IP 地址

    while (1) {
        //从 UART 端口接收数据
        int size = uart_read_bytes(UART_NUM_0, data, RX_BUF_SIZE, 20 / portTICK_PERIOD_MS);

        //检查是否有数据被读取
        if (size > 0) {

            //将接收的数据缓冲区末尾设置为字符串终止符
            data[size] = 0;

            // //使用日志函数 ESP_LOGI 打印接收到的字节数和数据内容。
            // ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", size, data);

            // //调用 ESP_LOG_BUFFER_HEXDUMP 函数以十六进制形式打印接收的数据内容，便于调试原始数据。
            // ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, size, ESP_LOG_INFO);

            // 通过 UDP 发送数据到客户端
            int err = sendto(sock, data, size, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            if (err < 0) {
                ESP_LOGE(RX_TASK_TAG, "Error occurred during sending: errno %d", errno);
            } else {
                led_state = !led_state;
                led_set_level(led_state);
                ESP_LOGI(RX_TASK_TAG, "Sent %d bytes to %s:%d", size, UDP_CLIENT_IP, UDP_SERVER_PORT);
            }
        }
    }
    //任务结束时释放缓冲区内存（不过这里实际上不会执行，因为任务处于无限循环中）
    // 释放缓冲区并关闭套接字
    free(data);
    // shutdown(sock, 0);
    close(sock);
    vTaskDelete(NULL); // 结束任务
}

static void udp_server_task(void *pvParameters)
{
    char rx_buffer[128];
    char addr_str[128];
    int addr_family = (int)pvParameters;// 用于决定协议族，通常是 AF_INET (IPv4)
    int ip_protocol = IPPROTO_IP;// 使用 IPv4 协议
    struct sockaddr_in dest_addr; // 使用 IPv4 地址结构

    while (1) {
        // 设置目的地址（IPv4）
        dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(UDP_SERVER_PORT);

        // 创建套接字
        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created");

        // 设置套接字接收超时时间为10秒
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

        // 绑定套接字到指定的地址和端口
        int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TAG, "Socket bound, port %d", UDP_SERVER_PORT);

        // 用于存储源地址
        struct sockaddr_storage source_addr;
        socklen_t socklen = sizeof(source_addr);

        while (1) {
            ESP_LOGI(TAG, "Waiting for data");

            //计算接收数据长度
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            // 如果接收失败
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            else {
                // 获取发送方的IP地址
                inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);

                // 将接收到的数据进行空字符终止处理，并将其当作字符串来处理
                rx_buffer[len] = 0; 
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
                ESP_LOGI(TAG, "%s", rx_buffer);
                
                // 将接收到的数据通过 UART 发送给另一个单片机
                uart_write_bytes(UART_NUM_0, rx_buffer, len); // 发送数据到 UART

                // 将接收到的数据发送回原发送者
                int err = sendto(sock, rx_buffer, len, 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
                if (err < 0) {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    break;
                }
            }
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    led_init();
    uart_init();
    wifi_init_softap();

    xTaskCreate(uart_rx_task, "uart_rx_task", 1024 * 2, NULL, 5, NULL);//configMAX_PRIORITIES - 1
    xTaskCreate(udp_server_task, "udp_server", 4096, (void*)AF_INET, 5, NULL);
}
