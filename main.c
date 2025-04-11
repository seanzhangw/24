
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
#include "hardware/clocks.h"
#include "hardware/pll.h"
// Include protothreads
#include "pt_driver/pt_cornell_rp2040_v1_3.h"

// Include game headers
#include "game_state.h"

// ==================================================
// === users serial input thread
// ==================================================
static PT_THREAD(protothread_serial(struct pt *pt))
{
  PT_BEGIN(pt);
  // stores user input
  static int user_input;
  // wait for 0.1 sec
  PT_YIELD_usec(1000000);
  // announce the threader version
  sprintf(pt_serial_out_buffer, "Protothreads RP2040 v1.0\n\r");
  // non-blocking write
  serial_write;
  while (1)
  {
    // print prompt
    switch (currentState)
    {
    case START_MENU:
      sprintf(pt_serial_out_buffer, "Input anything to start game: ");
      // non-blocking write
      serial_write;
      // spawn a thread to do the non-blocking serial read
      serial_read;
      // check if any input is received
      if (strlen(pt_serial_in_buffer) > 0)
      {
        transitionToState(GAME_PLAYING);
      }
      break;
    case GAME_PLAYING:
      sprintf(pt_serial_out_buffer, "Input two numbers and an operator (e.g., 0 + 1): ");
      serial_write;
      serial_read;

      int index_1 = 0;
      int index_2 = 0;
      // Parse user input for game logic
      char operation;
      if (sscanf(pt_serial_in_buffer, "%d %c %d", &index_1, &operation, &index_2) == 3)
      {
        int result;
        if (nums[index_1] == -1 || nums[index_2] == -1)
        {
          sprintf(pt_serial_out_buffer, "Error: Invalid index\n\r");
          serial_write;
          break;
        }
        switch (operation)
        {
        case '+':
          result = nums[index_1] + nums[index_2];
          break;
        case '-':
          result = nums[index_1] - nums[index_2];
          break;
        case '*':
          result = nums[index_1] * nums[index_2];
          break;
        case '/':
          result = nums[index_1] / nums[index_2];
          break;
        default:
          sprintf(pt_serial_out_buffer, "Error: Invalid operation\n\r");
          serial_write;
          break;
        }
        // Draw the result underneath the index_2 number
        char buffer[10];
        nums[index_1] = -1;
        nums[index_2] = result; // Update the number at index_2 with the result
        // Display the result
        sprintf(pt_serial_out_buffer, "Result: %d\n\r", result);
        serial_write;
      }
      else if (strcmp(pt_serial_in_buffer, "exit") == 0)
      {
        transitionToState(GAME_OVER);
      }
      else if (strcmp(pt_serial_in_buffer, "reset") == 0)
      {
        // Reset the game state
        transitionToState(GAME_PLAYING);
      }
      else
      {
        sprintf(pt_serial_out_buffer, "Error: Invalid input format. Use 'num1 op num2'\n\r");
        serial_write;
      }
      break;
    case GAME_OVER:
      sprintf(pt_serial_out_buffer, "Input anything to restart game: ");
      // non-blocking write
      serial_write;
      // spawn a thread to do the non-blocking serial read
      serial_read;
      // check if any input is received
      if (strlen(pt_serial_in_buffer) > 0)
      {
        transitionToState(START_MENU);
      }
      break;
    }
    PT_YIELD_usec(1000); // Yield to other threads
  } // END WHILE(1)
  PT_END(pt);
} // timer thread

// Animation on core 0
static PT_THREAD(protothread_anim(struct pt *pt))
{
  // Mark beginning of thread
  PT_BEGIN(pt);

  while (1)
  {
    executeStep();
    PT_YIELD_usec(1000); // Yield to other threads

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
    // begin_time = time_us_32();
    // // erase boid
    // drawRect(fix2int15(boid1_x), fix2int15(boid1_y), 2, 2, BLACK);
    // // update boid's position and velocity
    // wallsAndEdges(&boid1_x, &boid1_y, &boid1_vx, &boid1_vy);
    // // draw the boid at its new position
    // drawRect(fix2int15(boid1_x), fix2int15(boid1_y), 2, 2, color);
    // // delay in accordance with frame rate
    // spare_time = FRAME_RATE - (time_us_32() - begin_time);
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
  // pt_add_thread(protothread_anim1);
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

  // start core 1
  multicore_reset_core1();
  multicore_launch_core1(&core1_main);

  // add threads
  pt_add_thread(protothread_serial);
  pt_add_thread(protothread_anim);

  transitionToState(START_MENU);

  // start scheduler
  pt_schedule_start;
}
