#include <Arduino.h>
#include "DefinesForSprExt.h"
#include "Keyboard.h"

Keyboard key;

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
        uint16_t buttonStat = key.idle();
        static char entered[100] = "";
        uint8_t enteredLen = strlen(entered);
        // 下位1bit目が1なら
        if(buttonStat & 0b1){
            strcat(entered, "1");
        }
        if(buttonStat & 0b10){
            strcat(entered, "2");
        }
        if(buttonStat & 0b100){
            strcat(entered, "3");
        }
        if(buttonStat & 0b1000){
            strcat(entered, "4");
        }
        if(buttonStat & 0b10000){
            strcat(entered, "5");
        }
        if(buttonStat & 0b100000){
            strcat(entered, "6");
        }
        if(buttonStat & 0b1000000){
            strcat(entered, "7");
        }
        if(buttonStat & 0b10000000){
            strcat(entered, "8");
        }
        if(buttonStat & 0b100000000){
            strcat(entered, "9");
        }
        if(buttonStat & 0b1000000000){
            strcat(entered, "0");
        }
        if(buttonStat & 0b10000000000){
            entered[enteredLen - 1] = '\0';
        }
        if(buttonStat & 0b100000000000){
            Serial.println("-------------------------------------------");
            Serial.print("Entered: ");
            Serial.println(entered);
            Serial.println("-------------------------------------------");
            delay(1000);
        }
        if(buttonStat & 0b1000000000000){
            Serial.println("S Key was pressed.");
            memset(entered, 0, sizeof(entered)); // 本来のキー動作と関係ないけど
        }

        Serial.println(entered);

    #endif

    #ifdef DEFAULT

    #endif
}