#pragma once

#include <LoRa.h>
#include "macros.h"

#ifndef IS_SERIAL
#define IS_SERIAL // シリアル通信でのデバッグ出力を有効にする
#endif

#define DEFAULT_FREQUENCY 433E6
#define DEFAULT_TX_POWER 17

// macros
#define SUCCESS 0
#define FAILURE 255
#define ENTER 1

class Ra01{
    private:
    public:
        void begin(uint8_t cs, uint8_t reset, uint8_t dio0);
};