#include "Keyboard.h"

ShiftIn<2> sh;

uint32_t keyingTime = 0;
// 直近でボタン状態が変化した時刻（millis()）
// (now - keyingTime) > DEBOUNCE_TIME_MS で安定判定

uint32_t now = 0; // 現在の時間（ミリ秒）

uint16_t nowButtonStat = 0;     // 現在のボタンの状態
uint16_t lastButtonStat = 0;    // 1つ前のボタンの状態
uint16_t reportedButtonStat = 0; // 最後に確定報告した状態

void Keyboard::begin(int ploadPin, int dataPin, int clockPin){
    sh.begin(ploadPin, 25, dataPin, clockPin);
};

uint8_t Keyboard::getButtonStat(uint8_t num){
    if(num > BUTTON_NUM) return 255; // ボタン番号が上限値を越えている場合

    return sh.state(num);
}

uint16_t Keyboard::getAllButtonStat(){ // 下位ビット - ボタン0 〜 上位ビット - ボタン${BUTTON_NUM} で埋める
    uint16_t result = 0;

    for(int i = 0; i < BUTTON_NUM; i++){
        result |= (sh.state(i) << i);
    }

    return result;
}

void Keyboard::reloadButtonStat(){
    sh.update();
}

uint16_t Keyboard::idle(){
    reloadButtonStat();
    now = millis();
    nowButtonStat = getAllButtonStat();

    if(nowButtonStat != lastButtonStat){ // ボタンの状態が変化している場合
        keyingTime = now; // 変化開始時刻を記録
        lastButtonStat = nowButtonStat;
    }

    if((now - keyingTime) > DEBOUNCE_TIME_MS && nowButtonStat != reportedButtonStat){ // 状態が安定かつ未報告
        reportedButtonStat = nowButtonStat;
        return (((~nowButtonStat>>BUTTON_S)&1)<<12)|(((~nowButtonStat>>BUTTON_E)&1)<<11)|(((~nowButtonStat>>BUTTON_B)&1)<<10)|(((~nowButtonStat>>BUTTON_0)&1)<<9)|(((~nowButtonStat>>BUTTON_9)&1)<<8)|(((~nowButtonStat>>BUTTON_8)&1)<<7)|(((~nowButtonStat>>BUTTON_7)&1)<<6)|(((~nowButtonStat>>BUTTON_6)&1)<<5)|(((~nowButtonStat>>BUTTON_5)&1)<<4)|(((~nowButtonStat>>BUTTON_4)&1)<<3)|(((~nowButtonStat>>BUTTON_3)&1)<<2)|(((~nowButtonStat>>BUTTON_2)&1)<<1)|(((~nowButtonStat>>BUTTON_1)&1)<<0);
    }

    return 0;
}

static const char* const alphabetCandidates[10] = {
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

uint8_t Keyboard::getCharInput(char *output, uint16_t maxLen){
    uint16_t buttonStat = idle();
    if(strlen(output) + 1 >= maxLen) return FAILURE;

    if(mode == NUMBER_MODE){
        const char* numChars[] = {"1","2","3","4","5","6","7","8","9","0"};
        for(int i = 0; i < 10; i++){
            if(buttonStat & (1 << i)) strcat(output, numChars[i]);
        }

        if(buttonStat & (1 << 10)){ // Backspace
            if(strlen(output) > 0) output[strlen(output) - 1] = '\0';
            else {
                #ifdef IS_SERIAL
                Serial.println("Nothing to delete.");
                #endif
                return FAILURE;
            }
        }
        if(buttonStat & (1 << 11)){ // Enter
            #ifdef IS_SERIAL
            Serial.println("-------------------------------------------");
            Serial.print("Entered: "); Serial.println(output);
            Serial.println("-------------------------------------------");
            delay(1000);
            #endif
            return ENTER;
        }
        if(buttonStat & (1 << 12)){ // Switch
            mode = ALPHABET_MODE;
            #ifdef IS_SERIAL
            Serial.println("Switched to ALPHABET mode.");
            #endif
        }
    } else { // ALPHABET_MODE
        // 数字ボタン（bit 0〜9）の押下チェック
        for(int i = 0; i < 10; i++){
            if(buttonStat & (1 << i)){
                if(pendingButtonBit == (uint8_t)i){
                    // 同じボタン → 次の候補へサイクル
                    uint8_t len = strlen(alphabetCandidates[i]);
                    pendingCharIndex = (pendingCharIndex + 1) % len;
                } else {
                    // 別のボタン → 新しい選択を開始
                    pendingButtonBit = i;
                    pendingCharIndex = 0;
                }
                break;
            }
        }

        if(buttonStat & (1 << 10)){ // Backspace
            if(pendingButtonBit != 255){
                // 未確定文字をキャンセル
                pendingButtonBit = 255;
                pendingCharIndex = 0;
            } else if(strlen(output) > 0){
                output[strlen(output) - 1] = '\0';
            } else {
                #ifdef IS_SERIAL
                Serial.println("Nothing to delete.");
                #endif
                return FAILURE;
            }
        }
        if(buttonStat & (1 << 11)){ // Enter
            if(pendingButtonBit != 255){
                // 未確定文字を確定してバッファへ追加
                char ch[2] = {alphabetCandidates[pendingButtonBit][pendingCharIndex], '\0'};
                strcat(output, ch);
                pendingButtonBit = 255;
                pendingCharIndex = 0;
            } else {
                // 未確定なし → 入力完了
                #ifdef IS_SERIAL
                Serial.println("-------------------------------------------");
                Serial.print("Entered: "); Serial.println(output);
                Serial.println("-------------------------------------------");
                delay(1000);
                #endif
                return ENTER;
            }
        }
        if(buttonStat & (1 << 12)){ // Switch
            pendingButtonBit = 255;
            pendingCharIndex = 0;
            mode = NUMBER_MODE;
            #ifdef IS_SERIAL
            Serial.println("Switched to NUMBER mode.");
            #endif
        }
    }

    #ifdef IS_SERIAL
    /*Serial.print(output);
    if(mode == ALPHABET_MODE && pendingButtonBit != 255){
        // 選択中の候補文字を [ ] で表示
        Serial.print("[");
        Serial.print(alphabetCandidates[pendingButtonBit][pendingCharIndex]);
        Serial.print("]");
    }
    Serial.println();*/
    #endif

    return SUCCESS;
}