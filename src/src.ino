#include <Arduino.h>
#include <SDHCI.h>
#include <File.h>
#include <GNSS.h>
#include <RTC.h>
#include "Ra-01.h"
#include "DefinesForSprExt.h"
#include "Keyboard.h"
#include "display.h"

static Keyboard key;
static Display disp;
static Ra01 ra;
static SDClass sd;
static File logFile;
static SpGnss gnss;

static bool isSDEnabled = false; // SDカードを使用するかのフラグ
static bool rtcInitialized = false; // RTCが初期化されているかのフラグ

#define NOW_TIME_STR_LEN 64
static char nowTimeStr[NOW_TIME_STR_LEN] = "";

typedef enum {LOG_RECV, LOG_SEND, LOG_CHANGE_RECV_MODE, LOG_CHANGE_TX_POWER, LOG_CHANGE_FREQ, LOG_CHANGE_SF, LOG_CHANGE_BW} logType;

typedef struct{
    logType type;
    // uint32_t millis;
    char TimeStr[NOW_TIME_STR_LEN];
    char data[LOG_DATA_MAX_LEN];
    int rssi;
} logEntry;

static logEntry logArray[LOG_CAPACITY] = {0};
static uint8_t logCount = 0;
static uint8_t lastLogCount = 0;

static uint8_t recvMode = ON;
static long freq = DEFAULT_FREQUENCY;
static uint8_t txPower = DEFAULT_TX_POWER;
static uint8_t sf = DEFAULT_SF;
static uint8_t bw = DEFAULT_BW;

void makeLog(logType type, const char* data, int rssi = 0){
    logArray[logCount].type = type;
    // logArray[logCount].millis = millis();
    getNowTimeStr();
    strncpy(logArray[logCount].TimeStr, nowTimeStr, NOW_TIME_STR_LEN - 1);
    logArray[logCount].TimeStr[NOW_TIME_STR_LEN - 1] = '\0'; // Ensure null-termination
    strncpy(logArray[logCount].data, data, LOG_DATA_MAX_LEN - 1);
    logArray[logCount].data[LOG_DATA_MAX_LEN - 1] = '\0'; // Ensure null-termination
    logArray[logCount].rssi = rssi;
    logCount = (logCount + 1) % LOG_CAPACITY;
}

void saveLogToSDCard(logEntry* logBuffer, uint8_t logSize){
    if(isSDEnabled){
        logFile = sd.open("LoRa_Handy_Neo_Logs/log.txt", FILE_WRITE);
        if(logFile){
            for(uint8_t i = 0; i < logSize; i++){
                // logFile.print(logBuffer[i].millis);
                logFile.print(logBuffer[i].TimeStr);
                logFile.print(", ");

                if(logBuffer[i].type == LOG_RECV){
                    logFile.print("RECV");
                } else if(logBuffer[i].type == LOG_SEND){
                    logFile.print("SEND");
                } else if(logBuffer[i].type == LOG_CHANGE_RECV_MODE){
                    logFile.print("CHANGE_RECV_MODE");
                } else if(logBuffer[i].type == LOG_CHANGE_TX_POWER){
                    logFile.print("CHANGE_TX_POWER");
                } else if(logBuffer[i].type == LOG_CHANGE_FREQ){
                    logFile.print("CHANGE_FREQ");
                } else if(logBuffer[i].type == LOG_CHANGE_SF){
                    logFile.print("CHANGE_SF");
                } else if(logBuffer[i].type == LOG_CHANGE_BW){
                    logFile.print("CHANGE_BW");
                }

                logFile.print(", ");
                logFile.print(logBuffer[i].data);
                logFile.print(", ");
                logFile.println(logBuffer[i].rssi);
            }
            logFile.close();
        } else {
            #ifdef IS_SERIAL
            Serial.println("Failed to open log file for writing.");
            #endif
        }
    }
}

bool syncRtcWithGnss(){
    if(gnss.waitUpdate(0)){
        SpNavData navData;
        gnss.getNavData(&navData);

        if(navData.time.year > 2000){ // Spresense未初期化RTCは2000年を返すため、2000年より大きい場合のみ有効とする
            RtcTime rtc_time{
                navData.time.year,
                navData.time.month,
                navData.time.day,
                navData.time.hour,
                navData.time.minute,
                navData.time.sec
            };

            RTC.setTime(rtc_time);

            #ifdef IS_SERIAL
            Serial.println("RTC synchronized with GNSS time:");
            #endif
            
            return true;
        }
    }

    return false;
}

void getNowTimeStr(){
    nowTimeStr[0] = '\0'; // Clear the string first

    if(isRtcInitialized()){
        RtcTime now = RTC.getTime();
        sprintf(nowTimeStr, "%02d/%02d/%02d %02d:%02d:%02d", now.year() % 100, now.month(), now.day(), now.hour(), now.minute(), now.second());
    } else {
        sprintf(nowTimeStr, "%lu [ms]", millis());
    }
}

bool isRtcInitialized(){
    RtcTime now = RTC.getTime();
    return (now.year() > 2000); // RTCの年が2000年より大きければ初期化されているとみなす
}

void setup() {
    #ifdef IS_SERIAL
    Serial.begin(115200);
    #endif

    key.begin(BSR_PL, BSR_SO, BSR_CP);
    disp.begin();
    if(ra.begin(RA_CS, RA_RESET, RA_DIO0) == FAILURE){
        #ifdef IS_SERIAL
        Serial.println("Failed to initialize Ra01.");
        #endif
    }

    RTC.begin();
    if(!isRtcInitialized()){
        if(gnss.begin() != SUCCESS){
            #ifdef IS_SERIAL
            Serial.println("Failed to initialize GNSS.");
            #endif
        }

        gnss.select(GPS);
        gnss.select(QZ_L1CA); // TODO: 必要？
        gnss.select(QZ_L1S); // TODO: 必要？

        if(gnss.start(COLD_START) != SUCCESS){
            #ifdef IS_SERIAL
            Serial.println("Failed to start GNSS.");
            #endif
        }

        #ifdef IS_SERIAL
        Serial.println("Waiting for GNSS signals...");
        #endif
    }

    #ifdef OPENING_YUI_LOGO
    disp.showYuiLogo();
    disp.flush();
    #endif

    for(uint8_t i = 0; i < SD_CARD_SEARCHING_COUNT; i++){
        if(sd.begin()){
            isSDEnabled = true;

            sd.mkdir("LoRa_Handy_Neo_Logs/");

            logFile = sd.open("LoRa_Handy_Neo_Logs/log.txt", FILE_WRITE);
            logFile.println();
            logFile.println("----- New Session -----");
            logFile.println("This file is automatically generated by Ra-01 Handy Neo (made by Kazuma Kurokawa).");
            logFile.println();
            logFile.println("Timestamp, Log Type, Data, RSSI (for received data) [dBm]");
            logFile.println();
            logFile.close();

            #ifdef IS_SERIAL
            Serial.println("SD Card found successfully.");
            #endif

            break;
        }
    }

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
    uint8_t val = key.processCharInput();
    if(val == ENTER){
        key.inputInit();
        delay(2000);
    }
    #endif

    #ifdef DISPLAY_TEST
        disp.clear();
        disp.showRecv("HELLOHELLOHELLOHELLO", "2025/01/01 00:00:00", "HELLOHELLOHELLOHELLOHELLO", "2025/01/01 00:01:00");

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
        if(!rtcInitialized){
            if(syncRtcWithGnss()){
                rtcInitialized = true;
                gnss.stop();
                gnss.end();

                #ifdef IS_SERIAL
                Serial.println("Sync between RTC and GNSS has done!");
                #endif
            }
        }

        disp.clear();

        if(logCount >= LOG_CAPACITY - 1){
            saveLogToSDCard(logArray, logCount);
            // lastLogCount = logCount;
            logCount = 0;
        }

        if(recvMode){
            char tempData[LOG_DATA_MAX_LEN];
            if(ra.recv(tempData, LOG_DATA_MAX_LEN) > 0){
                makeLog(LOG_RECV, tempData, ra.getRssi());

                #ifdef IS_SERIAL
                Serial.print("Received: ");
                Serial.println(logArray[(logCount - 1 + LOG_CAPACITY) % LOG_CAPACITY].data);
                #endif
            }
        }

        uint8_t wts = disp.getWhatToShow();
        if(wts != SEND_MODE && wts != CHANGE_TX_POWER_MODE && wts != CHANGE_FREQ_MODE && wts != CHANGE_SF_MODE && wts != CHANGE_BW_MODE){
            key.idle();
        }

        if(wts == DEFAULT_MODE && key.getButtonStat(BUTTON_0)){
            disp.changeWhatToShow(MENU_MODE);

            #ifdef IS_SERIAL
            Serial.println("Menu opened.");
            #endif
        }

        if(wts == CHANGE_RECV_MODE && key.getButtonStat(BUTTON_0)){
            disp.changeWhatToShow(MENU_MODE);
            makeLog(LOG_CHANGE_RECV_MODE, recvMode ? "ON" : "OFF");

            #ifdef IS_SERIAL
            Serial.println("Menu opened.");
            #endif
        }

        if(wts == SEND_MODE && key.processCharInput() == ENTER){
            disp.changeWhatToShow(SEND_DONE_MODE);
            makeLog(LOG_SEND, key.getCharInput());
            ra.send(key.getCharInput(), strlen(key.getCharInput()));

            #ifdef IS_SERIAL
            Serial.print("Send: ");
            Serial.println(key.getCharInput());
            #endif

            key.inputInit();
        }

        if(wts == CHANGE_TX_POWER_MODE && key.processCharInput() == ENTER){
            disp.changeWhatToShow(CHANGE_TX_POWER_DONE_MODE);

            uint8_t val = atoi(key.getCharInput());
            if(val >= 2 && val <= 20){
                txPower = val;
                ra.setTxPower(val);
                makeLog(LOG_CHANGE_TX_POWER, key.getCharInput());

            } else {
                key.inputInit();
                #ifdef IS_SERIAL
                Serial.println("Invalid TX Power value. Must be between 2 and 20.");
                #endif
            }
            
            #ifdef IS_SERIAL
            Serial.print("TX Power[dbm] set to: ");
            Serial.println(txPower);
            #endif

            key.inputInit();
        }

        if(wts == CHANGE_FREQ_MODE && key.processCharInput() == ENTER){
            disp.changeWhatToShow(CHANGE_FREQ_DONE_MODE);

            long val = atol(key.getCharInput());
            if(val >= 137000000 && val <= 1020000000){
                freq = val;
                ra.setFreq(val);
                makeLog(LOG_CHANGE_FREQ, key.getCharInput());
            } else {    
                key.inputInit();
                #ifdef IS_SERIAL
                Serial.println("Invalid Frequency value. Must be between 137000000 and 1020000000.");
                #endif
            }

            #ifdef IS_SERIAL
            Serial.print("Frequency set to: ");
            Serial.println(freq);
            #endif

            key.inputInit();
        }

        if(wts == CHANGE_SF_MODE && key.processCharInput() == ENTER){
            disp.changeWhatToShow(CHANGE_SF_DONE_MODE);

            uint8_t val = atoi(key.getCharInput());
            if(val >= 6 && val <= 12){
                sf = val;
                ra.setSF(val);
                makeLog(LOG_CHANGE_SF, key.getCharInput());
            } else {
                key.inputInit();
                #ifdef IS_SERIAL
                Serial.println("Invalid SF value. Must be between 6 and 12.");
                #endif
            }

            #ifdef IS_SERIAL
            Serial.print("SF set to: ");
            Serial.println(key.getCharInput());
            #endif

            key.inputInit();
        }

        if(wts == CHANGE_BW_MODE && key.processCharInput() == ENTER){
            disp.changeWhatToShow(CHANGE_BW_DONE_MODE);

            uint8_t val = atoi(key.getCharInput());
            if(val <= 9){
                bw = val;
                makeLog(LOG_CHANGE_BW, key.getCharInput());
                ra.setBW(val);
            } else {
                key.inputInit();
                #ifdef IS_SERIAL
                Serial.println("Invalid BW value. Must be between 0 and 9.");
                #endif
            }

            #ifdef IS_SERIAL
            Serial.print("BW set to: ");
            Serial.println(key.getCharInput());
            #endif

            key.inputInit();
        }
        
        if((wts == SEND_DONE_MODE || wts == CHANGE_TX_POWER_DONE_MODE || wts == CHANGE_FREQ_DONE_MODE || wts == CHANGE_SF_DONE_MODE || wts == CHANGE_BW_DONE_MODE || wts == SHOW_NOW_SETTINGS_MODE || wts == SHOW_LOG_SAVE_DONE_MODE) && key.getButtonStat(BUTTON_0)){
            disp.changeWhatToShow(MENU_MODE);

            #ifdef IS_SERIAL
            Serial.println("Menu opened.");
            #endif
        }

        if(wts == MENU_MODE){
            if(key.getButtonStat(BUTTON_0)){
                disp.changeWhatToShow(DEFAULT_MODE);

                #ifdef IS_SERIAL
                Serial.println("Menu closed.");
                #endif
            }else if(key.getButtonStat(BUTTON_1)){
                disp.changeWhatToShow(SEND_MODE);

                #ifdef IS_SERIAL
                Serial.println("Send mode opened.");
                #endif
            }else if(key.getButtonStat(BUTTON_2)){
                recvMode = !recvMode;
                disp.changeWhatToShow(CHANGE_RECV_MODE);

                #ifdef IS_SERIAL
                Serial.println("Receive mode changed.");
                #endif
            }else if(key.getButtonStat(BUTTON_3)){
                disp.changeWhatToShow(CHANGE_TX_POWER_MODE);

                #ifdef IS_SERIAL
                Serial.println("Change TX power mode opened.");
                #endif
            }else if(key.getButtonStat(BUTTON_4)){
                disp.changeWhatToShow(CHANGE_FREQ_MODE);

                #ifdef IS_SERIAL
                Serial.println("Change Freq. mode opened.");
                #endif
            }else if(key.getButtonStat(BUTTON_5)){
                disp.changeWhatToShow(CHANGE_SF_MODE);

                #ifdef IS_SERIAL
                Serial.println("Change SF mode opened.");
                #endif
            }else if(key.getButtonStat(BUTTON_6)){
                disp.changeWhatToShow(CHANGE_BW_MODE);

                #ifdef IS_SERIAL
                Serial.println("Change BW mode opened.");
                #endif
            }else if(key.getButtonStat(BUTTON_7)){
                disp.changeWhatToShow(SHOW_NOW_SETTINGS_MODE);

                #ifdef IS_SERIAL
                Serial.println("Now settings mode opened.");
                #endif

            }else if(key.getButtonStat(BUTTON_8)){
                disp.changeWhatToShow(SHOW_LOG_SAVE_DONE_MODE);
                saveLogToSDCard(logArray, logCount);
                lastLogCount = logCount;
                logCount = 0;

                #ifdef IS_SERIAL
                Serial.println("Logs saved to SD card.");
                #endif
            }
        }

        int nowRecvDataNum = RECV_DATA_NUM_INIT;
        int beforeRecvDataNum = RECV_DATA_NUM_INIT;
        
        for(int i = 1; i <= LOG_CAPACITY; i++){
            int checkIndex = (logCount - i + LOG_CAPACITY) % LOG_CAPACITY;

            if(logArray[checkIndex].type == LOG_RECV){
                if(nowRecvDataNum == RECV_DATA_NUM_INIT){
                    nowRecvDataNum = checkIndex;
                }
                else if(beforeRecvDataNum == RECV_DATA_NUM_INIT){
                    beforeRecvDataNum = checkIndex;
                    break;
                }
            }
        }

        disp.showRecv(
            (beforeRecvDataNum != RECV_DATA_NUM_INIT) ? logArray[beforeRecvDataNum].data : "",
            (beforeRecvDataNum != RECV_DATA_NUM_INIT) ? logArray[beforeRecvDataNum].TimeStr : "",
            (nowRecvDataNum != RECV_DATA_NUM_INIT) ? logArray[nowRecvDataNum].data : "",
            (nowRecvDataNum != RECV_DATA_NUM_INIT) ? logArray[nowRecvDataNum].TimeStr : ""
        );

        disp.showSeparateLine();
        disp.showGuideToMenu(recvMode);
        disp.showMenu();
        disp.showChangeRecvMode(recvMode);
        disp.showSend(key.getCharInput(), key.getPendingChar());
        disp.showSendDone();
        disp.showChangeParams(key.getCharInput());

        if(wts == CHANGE_TX_POWER_DONE_MODE){
            disp.showChangeParamsDone(txPower);
        } else if(wts == CHANGE_FREQ_DONE_MODE){
            disp.showChangeParamsDone(freq);
        } else if(wts == CHANGE_SF_DONE_MODE){
            disp.showChangeParamsDone(sf);
        } else if(wts == CHANGE_BW_DONE_MODE){
            disp.showChangeParamsDone(bw);
        }

        getNowTimeStr();
        disp.showNowSettings(nowTimeStr, isSDEnabled, recvMode, txPower, freq, sf, bw);
        disp.showLogSaveDone(lastLogCount);
        disp.flush();
        key.consumeAllEdges();
    #endif
}