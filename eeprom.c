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
}

void eeprom_write_byte(uint16_t mem_addr, uint8_t data)
{
    uint8_t buffer[3];
    buffer[0] = (mem_addr >> 8) & 0xFF; // High byte
    buffer[1] = mem_addr & 0xFF;        // Low byte
    buffer[2] = data;
    // printf("Writing: 0x%02X to address 0x%04X\n", data, mem_addr);
    i2c_write_blocking(EEPROM_I2C, EEPROM_ADDR, buffer, 3, false);
    // printf("Write complete\n");
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

void eeprom_write_uint16(uint16_t addr, uint16_t value)
{
    eeprom_write_byte(addr, (value >> 8) & 0xFF); // High byte
    eeprom_write_byte(addr + 1, value & 0xFF);    // Low byte
}

uint16_t eeprom_read_uint16(uint16_t addr)
{
    uint8_t high = eeprom_read_byte(addr);
    uint8_t low = eeprom_read_byte(addr + 1);
    return ((uint16_t)high << 8) | low;
}

void eeprom_write_name(uint16_t addr, const char *name)
{
    for (int i = 0; i < NAME_LEN; i++)
    {
        // If the name is shorter than NAME_LEN, pad with space (or 0x00)
        char c = name[i];
        eeprom_write_byte(addr + i, (c != '\0') ? c : ' '); // Pad with space
    }
}

void eeprom_read_name(uint16_t addr, char *out)
{
    for (int i = 0; i < NAME_LEN; i++)
    {
        out[i] = eeprom_read_byte(addr + i);
        printf("Name byte %d: 0x%02X\n", i, out[i]);
    }
    out[NAME_LEN] = '\0';
}

uint16_t get_entry_addr(int mode, int index)
{
    return EEPROM_BASE_ADDR + mode * ENTRY_COUNT * ENTRY_SIZE + index * ENTRY_SIZE;
}

void erase_all_eeprom()
{
    for (uint16_t addr = 0; addr < 1024; addr++)
    { // adjust size if needed
        eeprom_write_byte(addr, 0xFF);
    }
}

void insert_score(const char *name, uint16_t score, int mode)
{
    for (int i = 0; i < ENTRY_COUNT; i++)
    {
        uint16_t addr = get_entry_addr(mode, i);
        uint16_t existing_score = eeprom_read_uint16(addr + NAME_LEN);
        printf("existing_score: %u\n", existing_score);
        if (existing_score == 0xFFFF || score > existing_score)
        {
            // Shift down
            for (int j = ENTRY_COUNT - 1; j > i; j--)
            {
                for (int k = 0; k < ENTRY_SIZE; k++)
                {
                    printf("106\n");
                    uint8_t b = eeprom_read_byte(get_entry_addr(mode, j - 1) + k);
                    printf("108\n");
                    eeprom_write_byte(get_entry_addr(mode, j) + k, b);
                    printf("k loop: %d\n", k);
                }
            }
            eeprom_write_name(get_entry_addr(mode, i), name);
            eeprom_write_uint16(get_entry_addr(mode, i) + NAME_LEN, score);
            printf("Score written: %u\n", score);
            return;
        }
        printf("did not enter condition\n");
    }
}