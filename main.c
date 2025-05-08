
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
#include "hardware/spi.h"

// Include protothreads
#include "pt_driver/pt_cornell_rp2040_v1_3.h"

// Include game headers
#include "game_state.h"
#include "array_collection_difficultylevel.h"
#include "input_handler.h"

#include "background.h"
// #include "bingo.h"
// #include "buzzer.h"
#include "click.h"
#include "deal_cards.h" // should move into main.c
// #include "final_victory.h"
#include "flip_cards.h"

// const int num_of_audio_files = 2;
#define MAX_AUDIO_FILES 2
#include "eeprom.h"

#define FRAME_RATE 33000

// SPI pin definitions
#define PIN_MISO 12
#define PIN_CS 13
#define PIN_SCK 14
#define PIN_MOSI 15
#define SPI_PORT spi1

// DAC command configuration: A-channel, 1x gain, active mode
#define DAC_config_chan_A 0b0011000000000000

int data_chan = 666;
int data_chan2 = 777;
int data_chan3 = 888;
int data_chan4 = 999;
const unsigned short *DAC_data_background = NULL;
const unsigned short *DAC_data_click = NULL;
const unsigned short *DAC_data_deal = NULL;
const unsigned short *DAC_data_flip = NULL;

// ==================================================
// === game controller thread
// ==================================================
static PT_THREAD(protothread_serial(struct pt *pt))
{
  PT_BEGIN(pt);
  while (1)
  {
    // printf("player state: %d\n\r", player1.currentState);
    switch (player1.currentState)
    {
    case START_MENU:
      static JoystickDir lastJoystickDir = NEUTRAL;
      adc_select_input(ADC_CHAN0);
      int joystick_x = adc_read();
      adc_select_input(ADC_CHAN1);
      int joystick_y = adc_read();

      sleep_us(400); // Small delay to stabilize joystick readings
      JoystickDir index = joystickSelect(joystick_x, joystick_y);
      if (index != NEUTRAL && lastJoystickDir == NEUTRAL)
      {
        handle_start_menu_input(false, index);
      }
      lastJoystickDir = index;

      // Read button inputs
      if (gpio_get(BUTTON_PIN_P1_E) == 0)
      {
        // ghetto debouncing
        while (gpio_get(BUTTON_PIN_P1_E) == 0)
          ;
        handle_start_menu_input(true, -NEUTRAL);
      }
      // TODO: Add joystick reads here in the future
      break;
    case oneMin:
      if (gpio_get(BUTTON_PIN_P1_E) == 0)
      {
        while (gpio_get(BUTTON_PIN_P1_E) == 0)
          ;
        transitionToState(&player1, START_MENU);
        transitionToState(&player2, START_MENU);
      }
      break;
    case twoMin:
      if (gpio_get(BUTTON_PIN_P1_E) == 0)
      {
        while (gpio_get(BUTTON_PIN_P1_E) == 0)
          ;
        transitionToState(&player1, START_MENU);
        transitionToState(&player2, START_MENU);
      }
      break;
    case threeMin:
      if (gpio_get(BUTTON_PIN_P1_E) == 0)
      {
        while (gpio_get(BUTTON_PIN_P1_E) == 0)
          ;
        transitionToState(&player1, START_MENU);
        transitionToState(&player2, START_MENU);
      }
      break;
    case GAME_PLAYING:
      adc_select_input(ADC_CHAN0);
      joystick_x = adc_read();

      adc_select_input(ADC_CHAN1);
      joystick_y = adc_read();

      // User Selection
      index = joystickSelect(joystick_x, joystick_y);

      if (gpio_get(BUTTON_PIN_P1_S) == 0)
      {
        // ghetto debouncing
        while (gpio_get(BUTTON_PIN_P1_S) == 0)
          ;
        // printf("skip level");
        skipLevel(&player1, startMenuState.settings.difficultyLevel);
      }
      else if (gpio_get(BUTTON_PIN_P1_E) == 0)
      {
        // ghetto debouncing
        while (gpio_get(BUTTON_PIN_P1_E) == 0)
          ;
        if (index != NEUTRAL)
        {
          // Handle card selection
          handle_card_select(&player1, true, index);
        }
        else
        {
          // Handle card selection
          handle_card_select(&player1, true, NEUTRAL);
        }
      }
      else if (index != NEUTRAL)
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
      // printf("start menu time: %d min\n", startMenuState.settings.mins);
      if (gpio_get(BUTTON_PIN_P1_E) == 0 || gpio_get(BUTTON_PIN_P1_R) == 0)
      {
        // ghetto debouncing
        while (gpio_get(BUTTON_PIN_P1_E) == 0 || gpio_get(BUTTON_PIN_P1_R) == 0)
          ;

        transitionToState(&player1, START_MENU);
        // dma_channel_abort(data_chan3);
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
      static JoystickDir lastJoystickDir = NEUTRAL;
      int joystick_x = ads1115_read_single_channel(7);
      int joystick_y = ads1115_read_single_channel(4);

      sleep_us(400); // Small delay to stabilize joystick readings
      JoystickDir index = joystickSelect(joystick_x, joystick_y);
      if (index != NEUTRAL && lastJoystickDir == NEUTRAL)
      {
        handle_start_menu_input(false, index);
      }
      lastJoystickDir = index;

      // Read button inputs
      if (gpio_get(BUTTON_PIN_P2_E) == 0)
      {
        // ghetto debouncing
        while (gpio_get(BUTTON_PIN_P2_E) == 0)
          ;
        handle_start_menu_input(true, NEUTRAL);
      }
      // TODO: Add joystick reads here in the future
      break;

    case oneMin:
      if (gpio_get(BUTTON_PIN_P2_E) == 0)
      {
        while (gpio_get(BUTTON_PIN_P2_E) == 0)
          ;
        transitionToState(&player1, START_MENU);
        transitionToState(&player2, START_MENU);
      }
      break;
    case twoMin:
      if (gpio_get(BUTTON_PIN_P2_E) == 0)
      {
        while (gpio_get(BUTTON_PIN_P2_E) == 0)
          ;
        transitionToState(&player1, START_MENU);
        transitionToState(&player2, START_MENU);
      }
      break;
    case threeMin:
      if (gpio_get(BUTTON_PIN_P2_E) == 0)
      {
        while (gpio_get(BUTTON_PIN_P2_E) == 0)
          ;
        transitionToState(&player1, START_MENU);
        transitionToState(&player2, START_MENU);
      }
      break;
      break;
    case GAME_PLAYING:
      joystick_x = ads1115_read_single_channel(7);
      joystick_y = ads1115_read_single_channel(4);

      // User Selection
      index = joystickSelect_ads(joystick_x, joystick_y);
      // f("index: %d\n", index);

      if (gpio_get(BUTTON_PIN_P2_S) == 0)
      {
        // ghetto debouncing
        while (gpio_get(BUTTON_PIN_P2_S) == 0)
          ;
        skipLevel(&player2, startMenuState.settings.difficultyLevel);
      }
      else if (gpio_get(BUTTON_PIN_P2_E) == 0)
      {
        // ghetto debouncing
        while (gpio_get(BUTTON_PIN_P2_E) == 0)
          ;
        if (index != NEUTRAL)
        {
          // Handle card selection
          handle_card_select(&player2, true, index);
        }
        else
        {
          // Handle card selection
          handle_card_select(&player2, true, NEUTRAL);
        }
      }
      else if (index != NEUTRAL)
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
      // printf("start menu time: %d min\n", startMenuState.settings.mins);
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
  // initialize stdio
  stdio_init_all();

  // initialize VGA
  initVGA();

  // SPI setup (do once)
  spi_init(SPI_PORT, 20000000);
  spi_set_format(SPI_PORT, 16, 0, 0, 0);
  gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
  gpio_set_function(PIN_CS, GPIO_FUNC_SPI);
  gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
  gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

  // Prepare wrapped audio, changed into loop? X
  // DAC_data_background = background_audio; //
  DAC_data_click = click_audio;     // 11378
  DAC_data_deal = deal_cards_audio; // 7674
  DAC_data_flip = flip_cards_audio; // 6880

  // DAC_data_background = (unsigned short *)malloc(background_audio_len * sizeof(unsigned short));
  // if (!DAC_data_background) {
  //     printf("DAC_data3 malloc failed!\n");
  // }

  // for (uint32_t i = 0; i < background_audio_len; ++i) {
  //     DAC_data_background[i] = DAC_config_chan_A | (background_audio[i] & 0x0fff);
  // }

  // configure the first DMA channel for audio playback: deal_cards
  data_chan = dma_claim_unused_channel(true);
  printf("DMA channel %d claimed for data_chan\n", data_chan);
  dma_channel_config c = dma_channel_get_default_config(data_chan);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
  channel_config_set_read_increment(&c, true);
  channel_config_set_write_increment(&c, false);
  dma_timer_set_fraction(0, 0x0006, 0xffff); // ~11.025 kHz
  channel_config_set_dreq(&c, 0x3b);         // Timer 0 pacing

  dma_channel_configure(
      data_chan, &c,
      &spi_get_hw(SPI_PORT)->dr,
      DAC_data_deal,        // DAC_data
      deal_cards_audio_len, // sample_count
      false                 // DO NOT Start immediately
  );

  // configure the second DMA channel for audio playback: click
  data_chan2 = dma_claim_unused_channel(true);
  printf("DMA channel %d claimed for data_chan2\n", data_chan2);
  dma_channel_config c2 = dma_channel_get_default_config(data_chan2);
  channel_config_set_transfer_data_size(&c2, DMA_SIZE_16);
  channel_config_set_read_increment(&c2, true);
  channel_config_set_write_increment(&c2, false);
  dma_timer_set_fraction(0, 0x0006, 0xffff); // ~11.025 kHz
  channel_config_set_dreq(&c2, 0x3b);        // Timer 0 pacing

  dma_channel_configure(
      data_chan2, &c2,
      &spi_get_hw(SPI_PORT)->dr,
      DAC_data_click,  // DAC_data
      click_audio_len, // sample_count
      false            // DO NOT Start immediately
  );

  // configure the third DMA channel for audio playback: background
  data_chan3 = dma_claim_unused_channel(true);
  printf("DMA channel %d claimed for data_chan3\n", data_chan3);
  dma_channel_config c3 = dma_channel_get_default_config(data_chan3);
  channel_config_set_transfer_data_size(&c3, DMA_SIZE_16);
  channel_config_set_read_increment(&c3, true);
  channel_config_set_write_increment(&c3, false);
  dma_timer_set_fraction(0, 0x0006, 0xffff); // ~11.025 kHz
  channel_config_set_dreq(&c3, 0x3b);        // Timer 0 pacing

  dma_channel_configure(
      data_chan3, &c3,
      &spi_get_hw(SPI_PORT)->dr,
      DAC_data_background,  // DAC_data
      background_audio_len, // sample_count
      false                 // DO NOT Start immediately
  );

  // configure the fourth DMA channel for audio playback: flip_cards
  data_chan4 = dma_claim_unused_channel(true);
  printf("DMA channel %d claimed for data_chan4\n", data_chan4);
  dma_channel_config c4 = dma_channel_get_default_config(data_chan4);
  channel_config_set_transfer_data_size(&c4, DMA_SIZE_16);
  channel_config_set_read_increment(&c4, true);
  channel_config_set_write_increment(&c4, false);
  dma_timer_set_fraction(0, 0x0006, 0xffff); // ~11.025 kHz
  channel_config_set_dreq(&c4, 0x3b);        // Timer 0 pacing

  dma_channel_configure(
      data_chan4, &c4,
      &spi_get_hw(SPI_PORT)->dr,
      DAC_data_flip,        // DAC_data
      flip_cards_audio_len, // sample_count
      false                 // DO NOT Start immediately
  );

  // initialize solutions
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

  menuLock = spin_lock_instance(MENU_LOCK_ID);
  paramLock = spin_lock_instance(PARAM_LOCK_ID);

  transitionToState(&player1, START_MENU);

  // start scheduler
  pt_schedule_start;
}
