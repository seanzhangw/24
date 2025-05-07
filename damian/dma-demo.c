// Play background_audio once using DMA (no loop)

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/spi.h"

// #include "background.h"
#include "bingo.h" 
#include "buzzer.h"
#include "deal_cards.h"
#include "final_victory.h"

// // SPI pin definitions
// #define PIN_MISO 4
// #define PIN_CS   5
// #define PIN_SCK  6
// #define PIN_MOSI 7
// #define SPI_PORT spi0

// SPI pin definitions
#define PIN_MISO 12
#define PIN_CS   13
#define PIN_SCK  14
#define PIN_MOSI 15
#define SPI_PORT spi1

// DAC command configuration: A-channel, 1x gain, active mode
#define DAC_config_chan_A 0b0011000000000000

int main() {
    stdio_init_all();

    // Initialize SPI channel to 20 MHz
    spi_init(SPI_PORT, 20000000);
    spi_set_format(SPI_PORT, 16, 0, 0, 0);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Wrap raw audio data with DAC command bits
    static unsigned short *DAC_data;
    static const uint32_t sample_count = deal_cards_audio_len; // background_audio_len is already in samples
    DAC_data = (unsigned short *)malloc(sample_count * sizeof(unsigned short));
    if (!DAC_data) {
        printf("DAC_data malloc failed!\n");
        return 1;
    }
    for (uint32_t i = 0; i < sample_count; ++i) {
        DAC_data[i] = DAC_config_chan_A | (deal_cards_audio[i] & 0x0fff);
    }

    // Setup single DMA channel for one-shot playback
    int data_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(data_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    // dma_timer_set_fraction(0, 0x0017, 0xffff); // ~44 kHz
    dma_timer_set_fraction(0, 0x0006, 0xffff); // ~11.025 kHz
    channel_config_set_dreq(&c, 0x3b); // Timer 0 pacing

    dma_channel_configure(
        data_chan, &c,
        &spi_get_hw(SPI_PORT)->dr,
        DAC_data,
        sample_count,
        true // Start immediately
    );

    printf("DMA one-shot playback started\n");

    // Wait for DMA to finish
    while (dma_channel_is_busy(data_chan)) {
        tight_loop_contents();
    }

    printf("DMA playback finished\n");
    return 0;
}
