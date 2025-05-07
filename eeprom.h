#include "input_handler.h"
#define ENTRY_COUNT 10
#define ENTRY_SIZE 5
#define EEPROM_BASE_ADDR 0x0000

void init_eeprom();

void eeprom_write_byte(uint16_t mem_addr, uint8_t data);

uint8_t eeprom_read_byte(uint16_t mem_addr);

void eeprom_write_uint16(uint16_t, uint16_t);

uint16_t eeprom_read_uint16(uint16_t);

void eeprom_write_name(uint16_t addr, const char* name);

void eeprom_read_name(uint16_t addr, char* out);

void insert_score(const char* name, uint16_t score);

void print_leaderboard();