#pragma once

#include <ShiftIn.h>

#define BUTTON_NUM 16

#define BUTTON_1 1
#define BUTTON_2 0
#define BUTTON_3 3
#define BUTTON_4 4
#define BUTTON_5 5
#define BUTTON_6 7
#define BUTTON_7 6
#define BUTTON_8 8
#define BUTTON_9 14
#define BUTTON_0 2
#define BUTTON_B 15
#define BUTTON_E 13
#define BUTTON_S 12

constexpr uint32_t DEBOUNCE_TIME_MS = 20;
class Keyboard{
    private:

    public:
        void begin(int ploadPin, int dataPin, int clockPin);
        uint16_t idle();

        uint16_t getAllButtonStat();
        uint8_t getButtonStat(uint8_t num);
        void reloadButtonStat();
};