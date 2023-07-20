#pragma once
#include <Arduino.h>
#include <EEManager.h>

struct Data {
    uint16_t width = 10000;
    uint16_t height = 8000;
    uint16_t max_spd = 2000;
    uint8_t lashx = 0;
    uint8_t lashy = 0;
    uint8_t spp = 18;
} data;

EEManager memory(data);

void data_init() {
    EEPROM.begin(memory.blockSize());
    memory.begin(0, 'a');
}