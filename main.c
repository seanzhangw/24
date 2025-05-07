
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

// #include "background.h"
// #include "bingo.h" 
// #include "buzzer.h"
#include "deal_cards.h" // should move into main.c
// #include "final_victory.h"
#include "flip_cards.h"

// const int num_of_audio_files = 2;
#define MAX_AUDIO_FILES 2

#define FRAME_RATE 33000

// SPI pin definitions
#define PIN_MISO 12
#define PIN_CS   13
#define PIN_SCK  14
#define PIN_MOSI 15
#define SPI_PORT spi1

// DAC command configuration: A-channel, 1x gain, active mode
#define DAC_config_chan_A 0b0011000000000000

int data_chan = 666; 
unsigned short *DAC_data_background = NULL;
unsigned short *DAC_data_deal = NULL;
unsigned short *DAC_data_flip = NULL;

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
      // Read button inputs
      if (gpio_get(BUTTON_PIN_P1_E) == 0 || gpio_get(BUTTON_PIN_P1_R) == 0)
      {
        // ghetto debouncing
        while (gpio_get(BUTTON_PIN_P1_E) == 0 || gpio_get(BUTTON_PIN_P1_R) == 0)
          ;

        transitionToState(&player1, GAME_PLAYING); // trigger dma channel!!!
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

      if (gpio_get(BUTTON_PIN_P1_E) == 0)
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
  // pt_add_thread(protothread_serial1);

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

  // unsigned short *DAC_data_deal = NULL;
  // unsigned short *DAC_data_flip = NULL;
  // static const uint32_t sample_count_ = {deal_cards_audio_len, flip_cards_audio_len};

  // SPI setup (do once)
  spi_init(SPI_PORT, 20000000);
  spi_set_format(SPI_PORT, 16, 0, 0, 0);
  gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
  gpio_set_function(PIN_CS, GPIO_FUNC_SPI);
  gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
  gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

  // Prepare wrapped audio, changed into loop? x 
  // static unsigned short *DAC_data[MAX_AUDIO_FILES] = {NULL};
  DAC_data_deal = (unsigned short *)malloc(deal_cards_audio_len * sizeof(unsigned short));
  if (!DAC_data_deal) {
      printf("DAC_data1 malloc failed!\n");
  }

  for (uint32_t i = 0; i < deal_cards_audio_len; ++i) {
      DAC_data_deal[i] = DAC_config_chan_A | (deal_cards_audio[i] & 0x0fff);
  }

  DAC_data_flip = (unsigned short *)malloc(flip_cards_audio_len * sizeof(unsigned short));
  if (!DAC_data_flip) {
      printf("DAC_data2 malloc failed!\n");
  }

  for (uint32_t i = 0; i < flip_cards_audio_len; ++i) {
      DAC_data_flip[i] = DAC_config_chan_A | (flip_cards_audio[i] & 0x0fff);
  }

  // DAC_data_background = (unsigned short *)malloc(background_audio_len * sizeof(unsigned short));
  // if (!DAC_data_background) {
  //     printf("DAC_data3 malloc failed!\n");
  // }

  // for (uint32_t i = 0; i < background_audio_len; ++i) {
  //     DAC_data_background[i] = DAC_config_chan_A | (background_audio[i] & 0x0fff);
  // }

  data_chan = dma_claim_unused_channel(true);
  printf("DMA channel %d claimed\n", data_chan);
  dma_channel_config c = dma_channel_get_default_config(data_chan);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
  channel_config_set_read_increment(&c, true);
  channel_config_set_write_increment(&c, false);
  dma_timer_set_fraction(0, 0x0006, 0xffff); // ~11.025 kHz
  channel_config_set_dreq(&c, 0x3b); // Timer 0 pacing

  dma_channel_configure(
      data_chan, &c,
      &spi_get_hw(SPI_PORT)->dr,
      DAC_data_deal, //DAC_data
      deal_cards_audio_len, //sample_count
      false // DO NOT Start immediately
  );

  // dma_channel_configure(
  //     data_chan, &c,
  //     &spi_get_hw(SPI_PORT)->dr,
  //     DAC_data_background, //DAC_data
  //     background_audio_len, //sample_count
  //     false // DO NOT Start immediately
  // );

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
