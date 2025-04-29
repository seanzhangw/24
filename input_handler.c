#include "input_handler.h"
#include "game_state.h"

void initController()
{
    adc_init();
    gpio_init(ADC_PIN0);
    gpio_init(ADC_PIN1);

    // Button SETUP
    gpio_init(BUTTON_PIN_P1_R);
    gpio_init(BUTTON_PIN_P2_R);
    gpio_init(BUTTON_PIN_P1_E);
    gpio_init(BUTTON_PIN_P2_E);

    gpio_set_dir(BUTTON_PIN_P1_R, GPIO_IN);
    gpio_set_dir(BUTTON_PIN_P2_R, GPIO_IN);
    gpio_set_dir(BUTTON_PIN_P1_E, GPIO_IN);
    gpio_set_dir(BUTTON_PIN_P2_E, GPIO_IN);
}

int joystickSelect(int joystick_x, int joystick_y)
{
    int delta_x = joystick_x - CENTER;
    int delta_y = joystick_y - CENTER;

    if (abs(delta_x) > abs(delta_y))
    {
        if (delta_x < -DEADZONE)
            return 0; // LEFT -> Choice 0
        if (delta_x > DEADZONE)
            return 1; // RIGHT -> Choice 1
    }
    else
    {
        if (delta_y < -DEADZONE)
            return 2; // UP -> Choice 2
        if (delta_y > DEADZONE)
            return 3; // DOWN -> Choice 3
    }
    return -1; // No valid movement (still centered)
}