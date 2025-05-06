
/**
 * Hunter Adams (vha3@cornell.edu)
 *
 * This demonstration animates two balls bouncing about the screen.
 * Through a serial interface, the user can change the ball color.
 *
 * HARDWARE CONNECTIONS
 *  - GPIO 16 ---> VGA Hsync
 *  - GPIO 17 ---> VGA Vsync
 *  - GPIO 18 ---> 470 ohm resistor ---> VGA Green
 *  - GPIO 19 ---> 330 ohm resistor ---> VGA Green
 *  - GPIO 20 ---> 330 ohm resistor ---> VGA Blue
 *  - GPIO 21 ---> 330 ohm resistor ---> VGA Red
 *  - RP2040 GND ---> VGA GND
 *
 * RESOURCES USED
 *  - PIO state machines 0, 1, and 2 on PIO instance 0
 *  - DMA channels (2, by claim mechanism)
 *  - 153.6 kBytes of RAM (for pixel color data)
 *
 */

// Include the VGA grahics library
#include "vga_driver/vga16_graphics.h"
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
#include "hardware/pll.h"
#include "hardware/adc.h"

// Include protothreads
#include "pt_driver/pt_cornell_rp2040_v1_3.h"

// Include game headers
#include "game_state.h"
#include "array_collection_difficultylevel.h"
#include "input_handler.h"
#include "eeprom.h"

#define FRAME_RATE 33000
#define JOYSTICK_COOLDOWN_US 150000 // 500ms cooldown for joystick input
// ==================================================
// === game controller thread
// ==================================================
static PT_THREAD(protothread_serial(struct pt *pt))
{
  PT_BEGIN(pt);
  while (1)
  {
    switch (player1.currentState)
    {
    case START_MENU:
      static int lastJoystickTime = 0;
      if (time_us_32() - lastJoystickTime > JOYSTICK_COOLDOWN_US)
      {
        adc_select_input(ADC_CHAN0);
        int joystick_x = adc_read();
        adc_select_input(ADC_CHAN1);
        int joystick_y = adc_read();

        int index = joystickSelect(joystick_x, joystick_y);
        handle_start_menu_input(false, index);
        lastJoystickTime = time_us_32();
      }

      // Read button inputs
      if (gpio_get(BUTTON_PIN_P1_E) == 0)
      {
        // ghetto debouncing
        while (gpio_get(BUTTON_PIN_P1_E) == 0)
          ;
        handle_start_menu_input(true, -1);
      }
      // TODO: Add joystick reads here in the future
      break;
    case GAME_PLAYING:
      adc_select_input(ADC_CHAN0);
      int joystick_x = adc_read();

      adc_select_input(ADC_CHAN1);
      int joystick_y = adc_read();

      // User Selection
      int index = joystickSelect(joystick_x, joystick_y);

      if (gpio_get(BUTTON_PIN_P1_S) == 0)
      {
        // ghetto debouncing
        while (gpio_get(BUTTON_PIN_P1_S) == 0)
          ;
        // printf("skip level");
        skipLevel(&player1);
      }
      else if (gpio_get(BUTTON_PIN_P1_E) == 0)
      {
        // ghetto debouncing
        while (gpio_get(BUTTON_PIN_P1_E) == 0)
          ;
        if (index != -1)
        {
          // Handle card selection
          handle_card_select(&player1, true, index);
        }
        else
        {
          // Handle card selection
          handle_card_select(&player1, true, -1);
        }
      }
      else if (index != -1)
      {
        handle_card_select(&player1, false, index);
      }
      else if (gpio_get(BUTTON_PIN_P1_R) == 0)
      {
        // ghetto debouncing
        while (gpio_get(BUTTON_PIN_P1_R) == 0)
          ;
        resetLevel(&player1);
      }
      break;
    case GAME_OVER:
      // Read button inputs
      if (gpio_get(BUTTON_PIN_P1_E) == 0 || gpio_get(BUTTON_PIN_P1_R) == 0)
      {
        // ghetto debouncing
        while (gpio_get(BUTTON_PIN_P1_E) == 0 || gpio_get(BUTTON_PIN_P1_R) == 0)
          ;

        transitionToState(&player1, START_MENU);
      }
      break;
    }
    PT_YIELD(pt);
  } // END WHILE(1)
  PT_END(pt);
}

// Animation on core 0
static PT_THREAD(protothread_anim(struct pt *pt))
{
  // Mark beginning of thread
  PT_BEGIN(pt);

  static int begin_time;
  static int spare_time;
  while (1)
  {
    begin_time = time_us_32();
    executeStep(&player1);

    spare_time = FRAME_RATE - (time_us_32() - begin_time);
    PT_YIELD_usec(spare_time);
  } // END WHILE(1)
  PT_END(pt);
} // animation thread

// ==================================================
// === game controller thread
// ==================================================
static PT_THREAD(protothread_serial1(struct pt *pt))
{
  PT_BEGIN(pt);

  while (1)
  {
    switch (player2.currentState)
    {
    case START_MENU:
      static int lastJoystickTime = 0;
      if (time_us_32() - lastJoystickTime > JOYSTICK_COOLDOWN_US)
      {
        int joystick_x = ads1115_read_single_channel(7);
        int joystick_y = ads1115_read_single_channel(4);

        int index = joystickSelect(joystick_x, joystick_y);
        handle_start_menu_input(false, index);
        lastJoystickTime = time_us_32();
      }

      // Read button inputs
      if (gpio_get(BUTTON_PIN_P2_E) == 0)
      {
        // ghetto debouncing
        while (gpio_get(BUTTON_PIN_P2_E) == 0)
          ;
        handle_start_menu_input(true, -1);
      }
      // TODO: Add joystick reads here in the future
      break;
    case GAME_PLAYING:
      int joystick_x = ads1115_read_single_channel(7);
      int joystick_y = ads1115_read_single_channel(4);

      // User Selection
      int index = joystickSelect_ads(joystick_x, joystick_y);
      // f("index: %d\n", index);

      if (gpio_get(BUTTON_PIN_P2_S) == 0)
      {
        // ghetto debouncing
        while (gpio_get(BUTTON_PIN_P2_S) == 0)
          ;
        skipLevel(&player2);
      }
      else if (gpio_get(BUTTON_PIN_P2_E) == 0)
      {
        // ghetto debouncing
        while (gpio_get(BUTTON_PIN_P2_E) == 0)
          ;
        if (index != -1)
        {
          // Handle card selection
          handle_card_select(&player2, true, index);
        }
        else
        {
          // Handle card selection
          handle_card_select(&player2, true, -1);
        }
      }
      else if (index != -1)
      {
        handle_card_select(&player2, false, index);
      }
      else if (gpio_get(BUTTON_PIN_P2_R) == 0)
      {
        // ghetto debouncing
        while (gpio_get(BUTTON_PIN_P2_R) == 0)
          ;
        resetLevel(&player2);
      }
      break;
    case GAME_OVER:
      // Read button inputs
      if (gpio_get(BUTTON_PIN_P2_E) == 0 || gpio_get(BUTTON_PIN_P2_R) == 0)
      {
        // ghetto debouncing
        while (gpio_get(BUTTON_PIN_P2_E) == 0 || gpio_get(BUTTON_PIN_P2_R) == 0)
          ;

        transitionToState(&player2, START_MENU);
      }
      break;
    }
    PT_YIELD(pt);
  } // END WHILE(1)
  PT_END(pt);
}
// Animation on core 1
static PT_THREAD(protothread_anim1(struct pt *pt))
{
  // Mark beginning of thread
  PT_BEGIN(pt);

  // Variables for maintaining frame rate
  static int begin_time;
  static int spare_time;

  // Spawn a boid
  // spawnBoid(&boid1_x, &boid1_y, &boid1_vx, &boid1_vy, 1);

  while (1)
  {
    // Measure time at start of thread
    begin_time = time_us_32();
    executeStep(&player2);
    // delay in accordance with frame rate
    spare_time = FRAME_RATE - (time_us_32() - begin_time);
    // yield for necessary amount of time
    PT_YIELD_usec(spare_time);
    // NEVER exit while
  } // END WHILE(1)
  PT_END(pt);
} // animation thread

// ========================================
// === core 1 main -- started in main below
// ========================================
void core1_main()
{
  // Add animation thread
  pt_add_thread(protothread_anim1);
  pt_add_thread(protothread_serial1);

  // Start the scheduler
  pt_schedule_start;
}

// ========================================
// === main
// ========================================
// USE ONLY C-sdk library
int main()
{
  // initialize stio
  stdio_init_all();

  // initialize VGA
  initVGA();

  // // initialize solutions
  sol_init();

  init_eeprom();

  // initialize controllers
  initController();
  // start core 1
  multicore_reset_core1();
  multicore_launch_core1(&core1_main);

  // add threads
  pt_add_thread(protothread_serial);
  pt_add_thread(protothread_anim);

  menuLock = spin_lock_instance(SPINLOCK_ID);
  transitionToState(&player1, START_MENU);
  // start scheduler
  pt_schedule_start;
}
