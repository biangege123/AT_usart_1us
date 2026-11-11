#ifndef __BUTTON_H
#define __BUTTON_H


#ifdef __cplusplus
extern "C"{
#endif

#include "stdio.h"
#include "at32f403a_407.h"



typedef enum
{
    USER_BUTTON = 0,
    S1_BUTTON = 1,
    S2_BUTTON = 2,
    S3_BUTTON = 3,
    S4_BUTTON = 4,
    NO_BUTTON = 5
}button_type;

#define USER_BUTTON_PIN     GPIO_PINS_0
#define USER_BUTTON_PORT    GPIOA
#define USER_BUTTON_CRM_CLK CRM_GPIOA_PERIPH_CLOCK



#define S1_BUTTON_PIN      GPIO_PINS_2
#define S1_BUTTON_PORT     GPIOE
#define S1_BUTTON_CRM_CLK  CRM_GPIOE_PERIPH_CLOCK

#define S2_BUTTON_PIN      GPIO_PINS_3
#define S2_BUTTON_PORT     GPIOE
#define S2_BUTTON_CRM_CLK  CRM_GPIOE_PERIPH_CLOCK

#define S3_BUTTON_PIN      GPIO_PINS_4
#define S3_BUTTON_PORT     GPIOE
#define S3_BUTTON_CRM_CLK  CRM_GPIOE_PERIPH_CLOCK

#define S4_BUTTON_PIN      GPIO_PINS_5
#define S4_BUTTON_PORT     GPIOE
#define S4_BUTTON_CRM_CLK  CRM_GPIOE_PERIPH_CLOCK





void at32_button_init(void);        //此按钮一定要GND,VCC
button_type at32_button_user_press(void);
button_type at32_button_s1_press(void);
button_type at32_button_s2_press(void);
button_type at32_button_s3_press(void);
button_type at32_button_s4_press(void);

#ifdef __cplusplus    
}
#endif






#endif
