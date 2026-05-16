#include "Ra-01.h"

void Ra01::begin(uint8_t cs, uint8_t reset, uint8_t dio0){
    LoRa.setPins(cs, reset, dio0);
    if (!LoRa.begin(DEFAULT_FREQUENCY)) {
        #ifdef IS_SERIAL
        Serial.println("Starting LoRa failed!");
        #endif
        // while (1);
    }
}