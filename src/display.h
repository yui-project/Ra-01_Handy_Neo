#pragma once

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "macros.h"

#ifndef IS_SERIAL
#define IS_SERIAL // シリアル通信でのデバッグ出力を有効にする
#endif

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define CHAR_MAX_LEN 21

#define MENU_SCROLL_INTERVAL_MS 100

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)

constexpr char* const MENU_MSG = "0. Send / 1. Recv ON/OFF / 2. TX Power / 3. Freq. / 4. SF / 5. BW / 6. Now Settings / 7. Log / 8. Close Menu /  ";

class Display{
    private:
        Adafruit_SSD1306 display{SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET};

        bool isMenuOpen = false;
        uint32_t menuLastMillis = 0;
        uint16_t menuCount = 0;

    public:
        uint8_t begin();
        uint8_t flush();
        uint8_t clear();
        uint8_t showRecv(char *recv1, uint32_t recv1Millis, char *recv2, uint32_t recv2Millis);
        uint8_t showSeparateLine();
        uint8_t showGuideToMenu();
        uint8_t showMenu();
        uint8_t openMenu();
        uint8_t closeMenu();

};