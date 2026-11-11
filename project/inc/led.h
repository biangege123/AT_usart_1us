#ifndef __LED_H
#define __LED_H

#ifdef __cplusplus
extern "C"{
#endif


#include "at32f403a_407.h"


typedef enum
{
    LED2 = 0,
    LED3 = 1,
    LED4 = 2
}led_type;


#define LED_NUM                 3

#define LED2_PIN                GPIO_PINS_13
#define LED2_GPIO               GPIOD
#define LED2_GPIO_CRM_CLK       CRM_GPIOD_PERIPH_CLOCK

#define LED3_PIN                GPIO_PINS_14
#define LED3_GPIO               GPIOD
#define LED3_GPIO_CRM_CLK       CRM_GPIOD_PERIPH_CLOCK

#define LED4_PIN                GPIO_PINS_15
#define LED4_GPIO               GPIOD
#define LED4_GPIO_CRM_CLK       CRM_GPIOD_PERIPH_CLOCK

void at32_board_init(void);
void at32_led_init(led_type led);
void at32_led_on(led_type led);
void at32_led_off(led_type led);
void at32_led_toggle(led_type led);


#ifdef __cplusplus    
}
#endif


#endif