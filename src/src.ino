#include <Arduino.h>
#include "Ra-01.h"
#include "DefinesForSprExt.h"
#include "Keyboard.h"
#include "display.h"

Keyboard key;
Display disp;

#define INPUT_MAX_LEN 20
char input[INPUT_MAX_LEN] = {0};

static uint8_t recvMode = ON;

void setup() {
    Serial.begin(115200);

    key.begin(BSR_PL, BSR_SO, BSR_CP);
    disp.begin();

    #ifdef OPENING_YUI_LOGO
    disp.showYuiLogo();
    disp.flush();
    delay(3000);
    #endif
}

void loop() {
    #ifdef BUTTON_TEST
    key.idle();
    
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
            disp.changeWhatToShow(MENU_MODE);
            Serial.println("Menu opened.");
        }

        if(disp.showGuideToMenu() == FAILURE && keyStat & (1 << 7)){
            disp.changeWhatToShow(DEFAULT_MODE);
            Serial.println("Menu closed.");
        }

        disp.showSeparateLine();
        //disp.showGuideToMenu();
        //disp.showMenu();
        disp.flush();

        delay(100);
    #endif

    #ifdef YUI_LOGO_TEST
    disp.clear();
    disp.showYuiLogo();
    disp.flush();

    while(1);
    #endif

    #ifdef DEFAULT
        disp.clear();
        if(recvMode){
            disp.showRecv("HELLOHELLOHELLOHELLO", millis(), "HELLOHELLOHELLOHELLOHELLO", millis() + 11111);
        }
        key.idle();
        uint8_t wts = disp.getWhatToShow();

        if(wts == DEFAULT_MODE && key.getButtonStat(BUTTON_0)){
            disp.changeWhatToShow(MENU_MODE);
            Serial.println("Menu opened.");
        }

        if(wts == CHANGE_RECV_MODE && key.getButtonStat(BUTTON_0)){
            disp.changeWhatToShow(MENU_MODE);
            Serial.println("Menu opened.");
        }

        if(wts == MENU_MODE){
            if(key.getButtonStat(BUTTON_0)){
                disp.changeWhatToShow(DEFAULT_MODE);
                Serial.println("Menu closed.");
            }else if(key.getButtonStat(BUTTON_1)){

            }else if(key.getButtonStat(BUTTON_2)){
                recvMode = !recvMode;
                disp.changeWhatToShow(CHANGE_RECV_MODE);
                Serial.println("Receive mode changed.");

            }else if(key.getButtonStat(BUTTON_3)){

            }else if(key.getButtonStat(BUTTON_4)){

            }else if(key.getButtonStat(BUTTON_5)){

            }else if(key.getButtonStat(BUTTON_6)){

            }else if(key.getButtonStat(BUTTON_7)){

            }else if(key.getButtonStat(BUTTON_8)){

            }
        }

        disp.showSeparateLine();
        disp.showGuideToMenu();
        disp.showMenu();
        disp.showChangeRecvMode(recvMode);
        disp.flush();
    #endif
}