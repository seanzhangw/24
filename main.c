
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
#include "array_sol.h"
#include "input_handler.h"

#define FRAME_RATE 33000

// ==================================================
// === users serial input thread
// ==================================================
static PT_THREAD(protothread_serial(struct pt *pt))
{
  PT_BEGIN(pt);
  while (1)
  {
    // 0 = 1st card, 1 = operation, 2 = 2nd card
    int current_stage = 0;
    int selected_index1 = -1;
    int selected_op = -1;
    int selected_index2 = -1;

    switch (player1.currentState)
    {
    case START_MENU:
      // Read button inputs

      if (gpio_get(BUTTON_PIN_P1_E) == 0 || gpio_get(BUTTON_PIN_P1_R) == 0)
      {
        transitionToState(&player1, GAME_PLAYING);
        transitionToState(&player2, GAME_PLAYING);
      }
      break;

    case GAME_PLAYING:
      // Read ADC Inputs
      adc_select_input(ADC_CHAN0);
      int joystick_x = adc_read();

      adc_select_input(ADC_CHAN1);
      int joystick_y = adc_read();

      // User Selection
      int index = joystickSelect(joystick_x, joystick_y);

      if (index != -1 && gpio_get(BUTTON_PIN_P1_E) == 0)
      {
        if (current_stage == 0)
        {
          selected_index1 = index;
          current_stage = 1;
        }
        else if (current_stage == 1)
        {
          selected_op = index;
          current_stage = 2;
        }
        else if (current_stage == 2)
        {
          selected_index2 = index;
        }

        // Perform operation
        int num1 = player1.nums[selected_index1];
        int num2 = player1.nums[selected_index2];
        char op = operations[selected_op];

        if (num1 != -1 && num2 != -1)
        {
          int result = 0;
          switch (op)
          {
          case '+':
            result = num1 + num2;
            break;
          case '-':
            result = num1 - num2;
            break;
          case '*':
            result = num1 * num2;
            break;
          case '/':
            result = num1 / num2;
            break;
          }

          // Update cards
          player1.nums[selected_index1] = -1;
          player1.nums[selected_index2] = result;
          current_stage = 0; // Reset for next round
        }
        // Reset buttons
        if (gpio_get(BUTTON_PIN_P1_R) == 0)
        {
          transitionToState(&player1, GAME_PLAYING);
          break;
        }
        if (gpio_get(BUTTON_PIN_P2_R) == 0)
        {
          transitionToState(&player2, GAME_PLAYING);
          break;
        }

        // If operation all finished, check and transition state
        int active_cards = 0;
        int value = -1;
        for (int i = 0; i < 4; i++)
        {
          if (player1.nums[i] != -1)
          {
            active_cards++;
            value = player1.nums[i];
          }
        }
        if (active_cards == 1 && value == 24)
        {
          transitionToState(&player1, GAME_OVER);
          break;
        }

      case GAME_OVER:
        if (gpio_get(BUTTON_PIN_P1_E) == 0)
        {
          transitionToState(&player1, START_MENU);
        }
        break;
      }

      PT_YIELD_usec(1000); // Yield to other threads
    } // END WHILE(1)
    PT_END(pt);
  }
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

  // initialize solutions
  sol_init();

  // initialize controllers
  initController();

  // start core 1
  multicore_reset_core1();
  multicore_launch_core1(&core1_main);

  // add threads
  pt_add_thread(protothread_serial);
  pt_add_thread(protothread_anim);

  transitionToState(&player1, START_MENU);

  // start scheduler
  pt_schedule_start;
}
