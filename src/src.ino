#include <Arduino.h>
#include "Ra-01.h"
#include "DefinesForSprExt.h"
#include "Keyboard.h"
#include "display.h"

Keyboard key;
Display disp;

#define INPUT_MAX_LEN 20
char input[INPUT_MAX_LEN] = {0};

void setup() {
    Serial.begin(115200);

    key.begin(BSR_PL, BSR_SO, BSR_CP);
    disp.begin();
}

void loop() {
    #ifdef BUTTON_TEST
    key.reloadButtonStat();
    
    for (int i = 0; i < BUTTON_NUM; i++) {
        Serial.print(key.getButtonStat(i));
    }

    //Serial.println(key.getButtonStat(15));
    Serial.println();

    delay(100);
    #endif

    #ifdef ENTER_TEST
    uint8_t val = key.getCharInput();
    if(val == ENTER){
        key.inputInit();
        delay(2000);
    }
    #endif

    #ifdef DISPLAY_TEST
        disp.clear();
        disp.showRecv("HELLOHELLOHELLOHELLO", millis(), "HELLOHELLOHELLOHELLOHELLO", millis() + 11111);

        uint16_t keyStat = key.idle();

        if(disp.showMenu() == FAILURE && keyStat & (1 << 9)){
            disp.openMenu();
            Serial.println("Menu opened.");
        }

        if(disp.showGuideToMenu() == FAILURE && keyStat & (1 << 7)){
            disp.closeMenu();
            Serial.println("Menu closed.");
        }

        disp.showSeparateLine();
        //disp.showGuideToMenu();
        //disp.showMenu();
        disp.flush();

        delay(100);
    #endif

    #ifdef DEFAULT

    #endif
}