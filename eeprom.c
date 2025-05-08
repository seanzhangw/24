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

void eeprom_write_uint16(uint16_t addr, uint16_t value) {
    eeprom_write_byte(addr, (value >> 8) & 0xFF);     // High byte
    eeprom_write_byte(addr + 1, value & 0xFF);        // Low byte
}

uint16_t eeprom_read_uint16(uint16_t addr) {
    uint8_t high = eeprom_read_byte(addr);
    uint8_t low = eeprom_read_byte(addr + 1);
    return ((uint16_t)high << 8) | low;
}

void eeprom_write_name(uint16_t addr, const char* name) {
    for (int i = 0; i < 3; i++) {
        eeprom_write_byte(addr + i, name[i]);
    }
}

void eeprom_read_name(uint16_t addr, char* out) {
    for (int i = 0; i < 3; i++) {
        out[i] = eeprom_read_byte(addr + i);
    }
    out[3] = '\0';
}

uint16_t get_entry_addr(int mode, int index) {
    return EEPROM_BASE_ADDR + mode * ENTRY_COUNT * ENTRY_SIZE + index * ENTRY_SIZE;
}

void erase_all_eeprom() {
    for (uint16_t addr = 0; addr < 512; addr++) { // adjust size if needed
        eeprom_write_byte(addr, 0xFF);
    }
}

void insert_score(const char* name, uint16_t score, int mode) {
    for (int i = 0; i < ENTRY_COUNT; i++) {
        uint16_t addr = get_entry_addr(mode, i);
        uint16_t existing_score = eeprom_read_uint16(addr + 3);
        if (score > existing_score) {
            // Shift down
            for (int j = ENTRY_COUNT - 1; j > i; j--) {
                for (int k = 0; k < ENTRY_SIZE; k++) {
                    uint8_t b = eeprom_read_byte(get_entry_addr(mode, j - 1) + k);
                    eeprom_write_byte(get_entry_addr(mode, j) + k, b);
                }
            }
            eeprom_write_name(get_entry_addr(mode, i), name);
            eeprom_write_uint16(get_entry_addr(mode, i) + 3, score);
            return;
        }
    }
}