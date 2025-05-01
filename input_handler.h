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

#define ADC_CHAN0 0
#define ADC_PIN0 26

#define ADC_CHAN1 1
#define ADC_PIN1 27

#define BUTTON_PIN_P1_R 2
#define BUTTON_PIN_P2_R 3
#define BUTTON_PIN_P1_E 4
#define BUTTON_PIN_P2_E 5

// ADC reading values
#define DEADZONE 1000 // reduce noise
#define CENTER 2048

void initController();

int joystickSelect(int, int);