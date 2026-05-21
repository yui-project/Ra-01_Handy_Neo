#include <Arduino.h>
#include "display.h"

uint8_t Display::begin(){
     // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        #ifdef IS_SERIAL
        Serial.println(F("SSD1306 allocation failed"));
        #endif

        return FAILURE;
        
        // while(1);
    }

    display.clearDisplay();
    display.display();

    return SUCCESS;
}

uint8_t Display::showRecv(char *recv1, uint32_t recv1Millis, char *recv2, uint32_t recv2Millis){
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.println("Received:");

    display.print(recv1Millis);
    display.println(" :");
    if(strlen(recv1) > 0 && strlen(recv1) < CHAR_MAX_LEN){
        display.println(recv1);
    } else if(strlen(recv1) > 0 && strlen(recv1) >= CHAR_MAX_LEN){
        for(uint8_t i = 0; i < CHAR_MAX_LEN - 3; i++){
            display.print(recv1[i]);
        }
        display.println("...");
    } else {
        display.println("No Data");
    }

    display.print(recv2Millis);
    display.println(" :");
    if(strlen(recv2) > 0 && strlen(recv2) < CHAR_MAX_LEN){
        display.println(recv2);
    } else if(strlen(recv2) > 0 && strlen(recv2) >= CHAR_MAX_LEN){
        for(uint8_t i = 0; i < CHAR_MAX_LEN - 3; i++){
            display.print(recv2[i]);
        }
        display.println("...");
    } else {
        display.println("No Data");
    }

    return SUCCESS;
}

uint8_t Display::showSeparateLine(){
    display.drawLine(0, 44, display.width(), 44, WHITE);
    return SUCCESS;
}

uint8_t Display::showGuideToMenu(uint8_t recvMode){
    if(whatToShow != DEFAULT_MODE) return FAILURE;
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,48);             // Start at top-left corner
    display.println("0: Open Menu");
    display.print("Recv mode: ");
    display.println(recvMode ? "ON" : "OFF");
    return SUCCESS;
}

uint8_t Display::flush(){
    display.display();
    return SUCCESS;
}

uint8_t Display::clear(){
    display.clearDisplay();
    return SUCCESS;
}

uint8_t Display::showMenu(){
    if(whatToShow != MENU_MODE) return FAILURE;

    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,48);             // Start at top-left corner
    display.println("Menu:");

    if(millis() - menuLastMillis >= MENU_SCROLL_INTERVAL_MS){
        menuCount = (menuCount + 1) % strlen(MENU_MSG);
        menuLastMillis = millis();
    }

    uint8_t msgLen = strlen(MENU_MSG);
    for(uint8_t i = 0; i < CHAR_MAX_LEN; i++){
        char ch = MENU_MSG[(menuCount + i) % msgLen];
        display.print(ch);
    }
    
    return SUCCESS;
}

uint8_t Display::showChangeRecvMode(uint8_t recvMode){
    if(whatToShow != CHANGE_RECV_MODE) return FAILURE;
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,48);             // Start at top-left corner
    display.print("Recv mode: ");
    display.println(recvMode ? "ON" : "OFF");
    display.println("0: Back to Menu");
    return SUCCESS;
}

uint8_t Display::showSend(const char* input, const char* pendingChar){
    if(whatToShow != SEND_MODE) return FAILURE;
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,48);             // Start at top-left corner
    display.println("Text (20c) / E: Send");
    display.print(input);
    display.print(pendingChar);
    return SUCCESS;
}

uint8_t Display::showSendDone(){
    if(whatToShow != SEND_DONE_MODE) return FAILURE;
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,48);             // Start at top-left corner
    display.println("Send completed.");
    display.println("0: Back to Menu");
    return SUCCESS;
}

uint8_t Display::showChangeParams(const char* input){
    if(whatToShow != CHANGE_TX_POWER_MODE && whatToShow != CHANGE_FREQ_MODE && whatToShow != CHANGE_SF_MODE && whatToShow != CHANGE_BW_MODE) return FAILURE;
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,48);             // Start at top-left corner

    const char* msg;

    if(whatToShow == CHANGE_TX_POWER_MODE){
        msg = CHANGE_TX_POWER_MODE_MSG;
    } else if(whatToShow == CHANGE_FREQ_MODE){
        msg = CHANGE_FREQ_MODE_MSG;
    } else if(whatToShow == CHANGE_SF_MODE){
        msg = CHANGE_SF_MODE_MSG;
    } else if(whatToShow == CHANGE_BW_MODE){
        msg = CHANGE_BW_MODE_MSG;
    }

    if(millis() - nonMenuLastMillis >= MENU_SCROLL_INTERVAL_MS){
        nonMenuCount = (nonMenuCount + 1) % strlen(msg);
        nonMenuLastMillis = millis();
    }

    uint8_t msgLen = strlen(msg);
    for(uint8_t i = 0; i < CHAR_MAX_LEN; i++){
        char ch = msg[(nonMenuCount + i) % msgLen];
        display.print(ch);
    }

    display.println(input);
    return SUCCESS;
}

uint8_t Display::showChangeParamsDone(uint32_t param){
    if(whatToShow != CHANGE_TX_POWER_DONE_MODE && whatToShow != CHANGE_FREQ_DONE_MODE && whatToShow != CHANGE_SF_DONE_MODE && whatToShow != CHANGE_BW_DONE_MODE) return FAILURE;
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,48);             // Start at top-left corner

    if(whatToShow == CHANGE_TX_POWER_DONE_MODE){
        display.print("TX Power[dbm]: ");
        display.println(param);
    } else if(whatToShow == CHANGE_FREQ_DONE_MODE){
        display.print("Freq[Hz]: ");
        display.println(param);
    } else if(whatToShow == CHANGE_SF_DONE_MODE){
        display.print("SF: ");
        display.println(param);
    } else if(whatToShow == CHANGE_BW_DONE_MODE){
        display.print("BW: ");
        display.println(param);
    }
    display.println("0: Back to Menu");
    return SUCCESS;
}

uint8_t Display::showNowSettings(bool isSDEnabled, uint8_t recvMode, uint32_t txPower, uint32_t freq, uint32_t sf, uint32_t bw){
    if(whatToShow != SHOW_NOW_SETTINGS_MODE) return FAILURE;
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,48);             // Start at top-left corner

    display.println("Now settings:");

    char msg[255] = "  SD Card: ";
    strcat(msg, isSDEnabled ? "MOUNTED" : "NOT MOUNTED");
    strcat(msg, " / Recv mode: ");
    strcat(msg, recvMode ? "ON" : "OFF");
    strcat(msg, " / TX Power: ");
    sprintf(msg + strlen(msg), "%d", txPower);
    strcat(msg, " dBm / Freq: ");
    sprintf(msg + strlen(msg), "%ld", freq);
    strcat(msg, " Hz / SF: ");
    sprintf(msg + strlen(msg), "%d", sf);
    strcat(msg, " / BW: ");
    sprintf(msg + strlen(msg), "%ld", bw);
    strcat(msg, "  / 0: Back to Menu / ");
    
    if(millis() - nonMenuLastMillis >= MENU_SCROLL_INTERVAL_MS){
        nonMenuCount = (nonMenuCount + 1) % strlen(msg);
        nonMenuLastMillis = millis();
    }

    uint8_t msgLen = strlen(msg);
    for(uint8_t i = 0; i < CHAR_MAX_LEN; i++){
        char ch = msg[(nonMenuCount + i) % msgLen];
        display.print(ch);
    }

    return SUCCESS;
}

uint8_t Display::showLogSaveDone(uint8_t num){
    if(whatToShow != SHOW_LOG_SAVE_DONE_MODE) return FAILURE;
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,48);             // Start at top-left corner
    display.print(num);
    display.println(" logs saved.");
    display.println("0: Back to Menu");
    return SUCCESS;
}

uint8_t Display::showYuiLogo(){
    display.drawBitmap(
        (display.width()  - YUI_LOGO_WIDTH ) / 2, (uint8_t)(CHAR_DEFAULT_HEIGHT * 1.5), epd_bitmap_yui, YUI_LOGO_WIDTH, YUI_LOGO_HEIGHT, WHITE);

    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text

    display.setCursor(8, display.height() - (uint8_t)(CHAR_DEFAULT_HEIGHT * 2.5));
    display.println("Ra-01 Handy Neo");

    display.setCursor(8, display.height() - (uint8_t)(CHAR_DEFAULT_HEIGHT * 1.5));
    display.println("Kazuma Kurokawa");

    return SUCCESS;
}

uint8_t Display::getWhatToShow(){
    return whatToShow;
}

uint8_t Display::changeWhatToShow(uint8_t wts){
    whatToShow = wts;

    if(whatToShow == MENU_MODE){
        menuLastMillis = millis();
        menuCount = 0;
        nonMenuLastMillis = 0;
        nonMenuCount = 0;
    }else if(whatToShow == DEFAULT_MODE){
        menuLastMillis = 0;
        menuCount = 0;
    }else if(whatToShow == CHANGE_TX_POWER_MODE || whatToShow == CHANGE_FREQ_MODE || whatToShow == CHANGE_SF_MODE || whatToShow == CHANGE_BW_MODE){
        nonMenuLastMillis = millis();
        nonMenuCount = 0;
    }
    
    return SUCCESS;
}