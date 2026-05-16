#pragma once

#include <LoRa.h>

#define IS_SERIAL // シリアル通信でのデバッグ出力を有効にする

#define DEFAULT_FREQUENCY 433E6

class Ra01{
    private:
    public:
        void begin(uint8_t cs, uint8_t reset, uint8_t dio0);
};