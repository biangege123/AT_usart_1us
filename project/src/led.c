#include "led.h"
#include "button.h"
#include "wk_system.h"



gpio_type *led_gpio_port[LED_NUM] = {LED2_GPIO,LED3_GPIO,LED4_GPIO};
uint16_t led_gpio_pin[LED_NUM] = {LED2_PIN,LED3_PIN,LED4_PIN};
crm_periph_clock_type led_gpio_crm_clk[LED_NUM] = {LED2_GPIO_CRM_CLK,LED3_GPIO_CRM_CLK,LED4_GPIO_CRM_CLK};




void at32_board_init(void)
{
    //delay_init();

    at32_led_init(LED2);
    at32_led_init(LED3);
    at32_led_init(LED4);
    at32_led_off(LED2);
    at32_led_off(LED3);
    at32_led_off(LED4);

    at32_button_init();

}




void at32_led_init(led_type led)
{
    gpio_init_type gpio_init_struct;

    crm_periph_clock_enable(led_gpio_crm_clk[led],TRUE);
    gpio_default_para_init(&gpio_init_struct);

    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
    gpio_init_struct.gpio_pins = led_gpio_pin[led];
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init(led_gpio_port[led],&gpio_init_struct);

}





void at32_led_on(led_type led)
{
    if(led > (LED_NUM - 1))
        return;
    if(led_gpio_pin[led])
        led_gpio_port[led]->clr = led_gpio_pin[led];

}

void at32_led_off(led_type led)
{
    if(led > (LED_NUM - 1))
        return;
    if(led_gpio_pin[led])
        led_gpio_port[led]->scr = led_gpio_pin[led];

}

void at32_led_toggle(led_type led)
{
    if(led > (LED_NUM - 1))
        return;
    if(led_gpio_pin[led])
        led_gpio_port[led]->odt ^= led_gpio_pin[led];

}
