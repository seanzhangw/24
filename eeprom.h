#include "input_handler.h"
#define ENTRY_SIZE 10
#define ENTRY_COUNT 10
#define MODE_COUNT 3
#define EEPROM_BASE_ADDR 0x0000

// Mode indexes
#define MODE_60S 0
#define MODE_120S 1
#define MODE_180S 2

#define MAX_DISPLAY 10
#define NAME_LEN 8

void init_eeprom();

void eeprom_write_byte(uint16_t mem_addr, uint8_t data);

uint8_t eeprom_read_byte(uint16_t mem_addr);

void eeprom_write_uint16(uint16_t, uint16_t);

uint16_t eeprom_read_uint16(uint16_t);

void eeprom_write_name(uint16_t addr, const char *name);

void eeprom_read_name(uint16_t addr, char *out);

void insert_score(const char *name, uint16_t score, int mode);

uint16_t get_entry_addr(int mode, int index);

void erase_all_eeprom();