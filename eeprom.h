#include "input_handler.h"

void init_eeprom();

void eeprom_write_byte(uint16_t mem_addr, uint8_t data);

uint8_t eeprom_read_byte(uint16_t mem_addr);