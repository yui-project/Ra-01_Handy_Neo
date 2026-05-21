#pragma once

#include <LoRa.h>
#include "macros.h"

#define DEFAULT_FREQUENCY 436500000
#define DEFAULT_TX_POWER 17
#define DEFAULT_SF 7
#define DEFAULT_BW 7
// macros
#define SUCCESS 0
#define FAILURE 255
#define ENTER 1

class Ra01{
    private:
    public:
        uint8_t begin(uint8_t cs, uint8_t reset, uint8_t dio0);
        uint8_t send(const char* data, uint8_t len);
        uint8_t recv(char* buf, uint8_t len);
        int getRssi();
        uint8_t setFreq(uint32_t freq);
        uint8_t setTxPower(uint8_t power);
        uint8_t setSF(uint8_t sf);
        uint8_t setBW(uint8_t bw);
        // uint8_t getSF();
        // uint8_t getBW();
};