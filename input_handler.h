#pragma once
// Include standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
// Include Pico libraries
#include "pico/stdlib.h"
#include "pico/divider.h"
#include "pico/multicore.h"
// Include hardware libraries
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/adc.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "hardware/i2c.h"

// I2C for ADS1115
#define I2C_PORT i2c1
#define SDA_PIN 6
#define SCL_PIN 7
#define ADS1115_ADDR 0x48
#define ADS1115_REG_CONVERSION 0x00
#define ADS1115_REG_CONFIG     0x01

#define ADC_CHAN0 0
#define ADC_PIN0 26

#define ADC_CHAN1 1
#define ADC_PIN1 27

#define ADC_CHAN2 2
#define ADC_PIN2 28

#define BUTTON_PIN_P1_R 2
#define BUTTON_PIN_P2_R 3
#define BUTTON_PIN_P1_E 4
#define BUTTON_PIN_P2_E 5
#define BUTTON_PIN_P1_S 8
#define BUTTON_PIN_P2_S 9

// ADC reading values
#define DEADZONE 1000 // reduce noise
#define DEADZONEads 1150
#define CENTER 2048
#define CENTERads 1500

void initController();

int joystickSelect(int, int);

int joystickSelect_ads(int, int);

int16_t ads1115_read_single_channel(uint8_t);