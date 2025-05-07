#include "eeprom.h"

#define EEPROM_I2C i2c0
#define EEPROM_ADDR 0x50 // 0b1010000 (A2,A1,A0 = 0)
#define EEPROM_SDA_PIN 4
#define EEPROM_SCL_PIN 5

void init_eeprom()
{
    printf("Initializing EEPROM...\n");
    i2c_init(EEPROM_I2C, 100 * 1000); // 100kHz
    gpio_set_function(EEPROM_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(EEPROM_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(EEPROM_SDA_PIN);
    gpio_pull_up(EEPROM_SCL_PIN);

    // Write a byte to address 0x0010
    eeprom_write_byte(0x0010, 0x42);

    // Read it back
    uint8_t data = eeprom_read_byte(0x0010);
    printf("Read: 0x%02X\n", data);
}
void eeprom_write_byte(uint16_t mem_addr, uint8_t data)
{
    uint8_t buffer[3];
    buffer[0] = (mem_addr >> 8) & 0xFF; // High byte
    buffer[1] = mem_addr & 0xFF;        // Low byte
    buffer[2] = data;
    printf("Writing: 0x%02X to address 0x%04X\n", data, mem_addr);
    i2c_write_blocking(EEPROM_I2C, EEPROM_ADDR, buffer, 3, false);
    printf("Write complete\n");
    sleep_ms(10); // Wait for EEPROM to complete write operation
}

uint8_t eeprom_read_byte(uint16_t mem_addr)
{
    uint8_t addr_buf[2] = {
        (mem_addr >> 8) & 0xFF,
        mem_addr & 0xFF};
    uint8_t result;

    // Set memory address
    i2c_write_blocking(EEPROM_I2C, EEPROM_ADDR, addr_buf, 2, true);
    // Read one byte
    i2c_read_blocking(EEPROM_I2C, EEPROM_ADDR, &result, 1, false);

    return result;
}
