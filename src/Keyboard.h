#pragma once

#include <ShiftIn.h>
#include "macros.h"

// params
#define INPUT_MAX_LEN 20

// buttons
#define BUTTON_NUM 16

#define BUTTON_1 2 // 1 or (, ), !, ?, &
#define BUTTON_2 1 // 2 or A, B, C
#define BUTTON_3 0 // 3 or D, E, F
#define BUTTON_4 3 // 4 or G, H, I
#define BUTTON_5 4 // 5 or J, K, L
#define BUTTON_6 5 // 6 or M, N, O
#define BUTTON_7 7 // 7 or P, Q, R, S
#define BUTTON_8 6 // 8 or T, U, V
#define BUTTON_9 8 // 9 or W, X, Y, Z
#define BUTTON_0 14 // 0 or Space
#define BUTTON_B 15 // Backspace
#define BUTTON_E 13 // Enter
#define BUTTON_S 12 // Switch InputMode

// macros
#define ENTER 1

#define NUMBER_MODE 0
#define ALPHABET_MODE 1

constexpr uint32_t DEBOUNCE_TIME_MS = 20;

const char* const alpKeyBinds[10] = {
    "()!?&", // bit 0: BUTTON_1
    "ABC",   // bit 1: BUTTON_2
    "DEF",   // bit 2: BUTTON_3
    "GHI",   // bit 3: BUTTON_4
    "JKL",   // bit 4: BUTTON_5
    "MNO",   // bit 5: BUTTON_6
    "PQRS",  // bit 6: BUTTON_7
    "TUV",   // bit 7: BUTTON_8
    "WXYZ",  // bit 8: BUTTON_9
    " ",     // bit 9: BUTTON_0
};

const uint8_t alpKeyBindsLen[10] = {
    strlen(alpKeyBinds[0]), // bit 0: BUTTON_1
    strlen(alpKeyBinds[1]), // bit 1: BUTTON_2
    strlen(alpKeyBinds[2]), // bit 2: BUTTON_3
    strlen(alpKeyBinds[3]), // bit 3: BUTTON_4
    strlen(alpKeyBinds[4]), // bit 4: BUTTON_5
    strlen(alpKeyBinds[5]), // bit 5: BUTTON_6
    strlen(alpKeyBinds[6]), // bit 6: BUTTON_7
    strlen(alpKeyBinds[7]), // bit 7: BUTTON_8
    strlen(alpKeyBinds[8]), // bit 8: BUTTON_9
    strlen(alpKeyBinds[9])  // bit 9: BUTTON_0
};
class Keyboard{
    private:
        uint8_t mode = NUMBER_MODE;
        uint8_t pendingButtonBit = 255; // 選択中のボタンのビット位置（255=未選択）
        uint8_t pendingCharIndex = 0;   // 候補文字のインデックス

        void pendingInit();

    public:
        void begin(int ploadPin, int dataPin, int clockPin);
        uint16_t idle();
        uint8_t processCharInput();
        const char* getCharInput();
        const char* getPendingChar();

        uint16_t getAllChStat();
        uint8_t getChStat(uint8_t num);
        bool getButtonStat(uint8_t num);
        void reloadButtonStat();

        void inputInit();
        void consumeAllEdges();
        char input[INPUT_MAX_LEN] = {0};
};