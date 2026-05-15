#include <Arduino.h>
#include "DefinesForSprExt.h"
#include "Keyboard.h"

Keyboard key;

#define INPUT_MAX_LEN 20
char input[INPUT_MAX_LEN] = {0};

void setup() {
    Serial.begin(115200);

    key.begin(BSR_PL, BSR_SO, BSR_CP);
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
    key.getCharInput(input, INPUT_MAX_LEN);
    for(int i = 0; i  < INPUT_MAX_LEN; i++){
        Serial.print(input[i]);
    }
    Serial.println();
    #endif

    #ifdef DEFAULT

    #endif
}