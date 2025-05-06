#include "input_handler.h"
#include "game_state.h"

void initController()
{
    adc_init();
    gpio_init(ADC_PIN0);
    gpio_init(ADC_PIN1);
    gpio_init(ADC_PIN2);

    i2c_init(I2C_PORT, 400 * 1000); // 100 kHz
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // Button SETUP
    gpio_init(BUTTON_PIN_P1_R);
    gpio_init(BUTTON_PIN_P2_R);
    gpio_init(BUTTON_PIN_P1_E);
    gpio_init(BUTTON_PIN_P2_E);
    gpio_init(BUTTON_PIN_P1_S);
    gpio_init(BUTTON_PIN_P2_S);

    gpio_set_dir(BUTTON_PIN_P1_R, GPIO_IN);
    gpio_set_dir(BUTTON_PIN_P2_R, GPIO_IN);
    gpio_set_dir(BUTTON_PIN_P1_E, GPIO_IN);
    gpio_set_dir(BUTTON_PIN_P2_E, GPIO_IN);
    gpio_set_dir(BUTTON_PIN_P1_S, GPIO_IN);
    gpio_set_dir(BUTTON_PIN_P2_S, GPIO_IN);
}

int joystickSelect(int joystick_x, int joystick_y)
{
    int delta_x = joystick_x - CENTER;
    int delta_y = joystick_y - CENTER;

    if (abs(delta_x) > abs(delta_y))
    {
        if (delta_x < -DEADZONE)
            return UP;
        if (delta_x > DEADZONE)
            return DOWN;
    }
    else
    {
        if (delta_y < -DEADZONE)
            return LEFT;
        if (delta_y > DEADZONE)
            return RIGHT;
    }
    return NEUTRAL; // No valid movement (still centered)
}

int joystickSelect_ads(int joystick_x, int joystick_y)
{
    int delta_x = joystick_x - CENTERads;
    int delta_y = joystick_y - CENTERads;

    if (abs(delta_x) > abs(delta_y))
    {
        if (delta_x < -DEADZONEads)
            return UP;
        if (delta_x > DEADZONEads)
            return DOWN;
    }
    else
    {
        if (delta_y < -DEADZONEads)
            return LEFT;
        if (delta_y > DEADZONEads)
            return RIGHT;
    }
    return NEUTRAL; // No valid movement (still centered)
}

int16_t ads1115_read_single_channel(uint8_t mux_bits)
{
    uint8_t config[3];
    config[0] = 0x01;                          // CONFIG register
    config[1] = 0x80 | (mux_bits << 4) | 0x02; // OS=1 (start), MUX, PGA=±2.048V, MODE=single-shot
    config[2] = 0xE3;                          // DR=860SPS, comparator off

    i2c_write_blocking(I2C_PORT, ADS1115_ADDR, config, 3, false);
    sleep_ms(2); // Wait for conversion to complete

    uint8_t reg = 0x00;
    i2c_write_blocking(I2C_PORT, ADS1115_ADDR, &reg, 1, false);
    uint8_t dummy[2];
    i2c_read_blocking(I2C_PORT, ADS1115_ADDR, dummy, 2, false);

    i2c_write_blocking(I2C_PORT, ADS1115_ADDR, config, 3, false);
    sleep_ms(2);

    i2c_write_blocking(I2C_PORT, ADS1115_ADDR, &reg, 1, false);
    uint8_t buf[2];
    i2c_read_blocking(I2C_PORT, ADS1115_ADDR, buf, 2, false);

    int16_t raw = (buf[0] << 8) | buf[1];
    return (raw * 4096) / 32768; // Scale to –4096..+4096
}