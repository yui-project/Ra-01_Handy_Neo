#pragma once

#include <ShiftIn.h>

#define BUTTON_NUM 13;

ShiftIn<2> sh;

uint32_t keyingTime = 0;
// キーボードの入力状況が変化していない期間（ミリ秒）
// デバウンス処理で使用

class Keyboard{
    private:

    public:
        void begin(int ploadPin, int dataPin, int clockPin);
        uint16_t getAllButtonStat();
        uint8_t getButtonStat(uint8_t num);
        void reloadButtonStat();
}