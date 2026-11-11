#include "button.h"
#include "wk_system.h"

#define BUTTON_DEBOUNCE_MS 10U

static uint8_t user_button_pressed = 1U;
static uint8_t s1_button_pressed = 1U;
static uint8_t s2_button_pressed = 1U;
static uint8_t s3_button_pressed = 1U;
static uint8_t s4_button_pressed = 1U;

static void button_gpio_init(gpio_type *port, uint16_t pin)
{
    gpio_init_type gpio_init_struct;

    gpio_default_para_init(&gpio_init_struct);
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pins = pin;
    gpio_init_struct.gpio_pull = GPIO_PULL_DOWN;
    gpio_init(port, &gpio_init_struct);
}

static button_type button_press_check(gpio_type *port, uint16_t pin,
                                      uint8_t *pressed, button_type button)
{
    if ((*pressed != 0U) &&
        (gpio_input_data_bit_read(port, pin) != RESET))
    {
        *pressed = 0U;
        wk_delay_ms(BUTTON_DEBOUNCE_MS);
        if (gpio_input_data_bit_read(port, pin) != RESET)
        {
            return button;
        }
    }
    else if (gpio_input_data_bit_read(port, pin) == RESET)
    {
        *pressed = 1U;
    }

    return NO_BUTTON;
}

void at32_button_init(void)
{
    crm_periph_clock_enable(USER_BUTTON_CRM_CLK, TRUE);
    crm_periph_clock_enable(S1_BUTTON_CRM_CLK, TRUE);

    button_gpio_init(USER_BUTTON_PORT, USER_BUTTON_PIN);
    button_gpio_init(S1_BUTTON_PORT, S1_BUTTON_PIN);
    button_gpio_init(S2_BUTTON_PORT, S2_BUTTON_PIN);
    button_gpio_init(S3_BUTTON_PORT, S3_BUTTON_PIN);
    button_gpio_init(S4_BUTTON_PORT, S4_BUTTON_PIN);
}

uint8_t at32_button_user_state(void)
{
    return gpio_input_data_bit_read(USER_BUTTON_PORT, USER_BUTTON_PIN);
}

uint8_t at32_button_s1_state(void)
{
    return gpio_input_data_bit_read(S1_BUTTON_PORT, S1_BUTTON_PIN);
}

uint8_t at32_button_s2_state(void)
{
    return gpio_input_data_bit_read(S2_BUTTON_PORT, S2_BUTTON_PIN);
}

uint8_t at32_button_s3_state(void)
{
    return gpio_input_data_bit_read(S3_BUTTON_PORT, S3_BUTTON_PIN);
}

uint8_t at32_button_s4_state(void)
{
    return gpio_input_data_bit_read(S4_BUTTON_PORT, S4_BUTTON_PIN);
}

button_type at32_button_user_press(void)
{
    return button_press_check(USER_BUTTON_PORT, USER_BUTTON_PIN,
                              &user_button_pressed, USER_BUTTON);
}

button_type at32_button_s1_press(void)
{
    return button_press_check(S1_BUTTON_PORT, S1_BUTTON_PIN,
                              &s1_button_pressed, S1_BUTTON);
}

button_type at32_button_s2_press(void)
{
    return button_press_check(S2_BUTTON_PORT, S2_BUTTON_PIN,
                              &s2_button_pressed, S2_BUTTON);
}

button_type at32_button_s3_press(void)
{
    return button_press_check(S3_BUTTON_PORT, S3_BUTTON_PIN,
                              &s3_button_pressed, S3_BUTTON);
}

button_type at32_button_s4_press(void)
{
    return button_press_check(S4_BUTTON_PORT, S4_BUTTON_PIN,
                              &s4_button_pressed, S4_BUTTON);
}
