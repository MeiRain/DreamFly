#include <string.h>

#include "driver/gpio.h"
#include "driver/uart.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "usart.h"

const int RX_BUF_SIZE = 1024;

#define TXD_PIN_NUM (GPIO_NUM_20)
#define RXD_PIN_NUM (GPIO_NUM_19)

void uart_init(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 460800,//波特率
        .data_bits = UART_DATA_8_BITS,//传输位
        .parity = UART_PARITY_DISABLE,//奇偶控制
        .stop_bits = UART_STOP_BITS_1,//停止位
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,//硬件流控模式
        .source_clk = UART_SCLK_DEFAULT,//通信模式
    };
    uart_param_config(UART_NUM_0, &uart_config);

    //指定配置 Tx、Rx、RTS 和 CTS 信号的 GPIO 管脚编号
    uart_set_pin(UART_NUM_0, TXD_PIN_NUM, RXD_PIN_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    /*  安装驱动程序并指定以下参数：
            1. UART 控制器编号
            2. Tx 环形缓冲区的大小
            3. Rx 环形缓冲区的大小
            4. 指向事件队列句柄的指针
            5. 事件队列大小
            6. 分配中断的标志            */
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_0, RX_BUF_SIZE * 2, 0, 0, NULL, 0);

}

int sendData(const char* logName, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_0, data, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}

void uart_tx_data(void *arg)
{
    static const char *TX_TASK_TAG = "TX_TASK";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    while (1) {
        sendData(TX_TASK_TAG, "Hello world");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
