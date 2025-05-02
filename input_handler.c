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


int joystickSelect_ads(int joystick_x, int joystick_y)
{
    int delta_x = joystick_x - CENTERads;
    int delta_y = joystick_y - CENTERads;

    if (abs(delta_x) > abs(delta_y))
    {
        if (delta_x < -DEADZONEads)
            return 0; // LEFT -> Choice 0
        if (delta_x > DEADZONEads)
            return 1; // RIGHT -> Choice 1
    }
    else
    {
        if (delta_y < -DEADZONEads)
            return 2; // UP -> Choice 2
        if (delta_y > DEADZONEads)
            return 3; // DOWN -> Choice 3
    }
    return -1; // No valid movement (still centered)
}

int16_t ads1115_read_single_channel(uint8_t mux_bits) {
    uint8_t config[3];
    config[0] = 0x01;  // CONFIG register
    config[1] = 0x80 | (mux_bits << 4) | 0x02;  // OS=1 (start), MUX, PGA=±2.048V, MODE=single-shot
    config[2] = 0xE3;  // DR=860SPS, comparator off

    if (i2c_write_blocking(I2C_PORT, ADS1115_ADDR, config, 3, false) < 0) {
        printf("Config write failed\n");
        return 0;
    }

    sleep_ms(2);  // Wait for conversion to complete

    uint8_t reg = 0x00;
    i2c_write_blocking(I2C_PORT, ADS1115_ADDR, &reg, 1, true);
    uint8_t buf[2];
    i2c_read_blocking(I2C_PORT, ADS1115_ADDR, buf, 2, false);

    int16_t raw = (buf[0] << 8) | buf[1];
    return (raw * 4096) / 32768;  // Scale to –4096..+4096
}


int16_t ads1115_read_ain0_scaled() {
    uint8_t config[3];
    config[0] = ADS1115_REG_CONFIG;
    config[1] = 0b11000010;
    config[2] = 0b11100101;

    if (i2c_write_blocking(I2C_PORT, ADS1115_ADDR, config, 3, false) < 0) {
        printf("Config write failed\n");
        return 0;
    }

    sleep_ms(2);

    uint8_t reg = ADS1115_REG_CONVERSION;
    if (i2c_write_blocking(I2C_PORT, ADS1115_ADDR, &reg, 1, true) < 0) {
        printf("Failed to set pointer to conversion reg\n");
        return 0;
    }

    uint8_t buf[2];
    if (i2c_read_blocking(I2C_PORT, ADS1115_ADDR, buf, 2, false) < 0) {
        printf("Read failed\n");
        return 0;
    }

    int16_t raw = (buf[0] << 8) | buf[1];  // MSB first
    int16_t scaled = (raw * 4096) / 32768; // Scale to –4096..+4096
    return scaled;
}