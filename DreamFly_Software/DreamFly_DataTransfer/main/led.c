#include "driver/gpio.h"

#include "led.h"

#define LED_PIN_NUM GPIO_NUM_2 //选择板子上led灯的gpio
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<LED_PIN_NUM)    //配置GPIO_OUT位寄存器

void led_init(void){
    //zero-initialize the config structure.
    gpio_config_t io_conf = {
    //disable interrupt
        .intr_type = GPIO_INTR_DISABLE,
    //set as output mode
        .mode = GPIO_MODE_OUTPUT,
    //bit mask of the pins that you want to set,e.g.GPIO18/19
        .pin_bit_mask = GPIO_OUTPUT_PIN_SEL,
    //disable pull-down mode
        .pull_down_en = 0,
    //disable pull-up mode
        .pull_up_en = 1,
    };
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

void led_set_level(uint32_t led_level){
    gpio_set_level(LED_PIN_NUM, led_level);
}
