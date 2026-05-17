#include "Keyboard.h"

ShiftIn<2> sh;

uint32_t keyingTime = 0;
// 直近でボタン状態が変化した時刻（millis()）
// (now - keyingTime) > DEBOUNCE_TIME_MS で安定判定

uint32_t now = 0; // 現在の時間（ミリ秒）

uint16_t nowButtonStat = 0;     // 現在のボタンの状態
uint16_t lastButtonStat = 0;    // 1つ前のボタンの状態
uint16_t reportedButtonStat = 0; // 最後に確定報告した状態
uint16_t risingEdgeBits = 0;    // idle()が確定した立ち上がりエッジ（getButtonStatで1ビットずつ消費）

void Keyboard::begin(int ploadPin, int dataPin, int clockPin){
    sh.begin(ploadPin, 25, dataPin, clockPin);
};

uint8_t Keyboard::getChStat(uint8_t num){
    if(num > BUTTON_NUM) return 255; // ボタン番号が上限値を越えている場合

    return sh.state(num);
}

bool Keyboard::getButtonStat(uint8_t num){
    bool result = (risingEdgeBits >> num) & 1;
    risingEdgeBits &= ~(1 << num); // 読んだビットを消費
    return result;
}

uint16_t Keyboard::getAllChStat(){ // 下位ビット - ボタン0 〜 上位ビット - ボタン${BUTTON_NUM} で埋める
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
    nowButtonStat = getAllChStat();

    if(nowButtonStat != lastButtonStat){ // ボタンの状態が変化している場合
        keyingTime = now; // 変化開始時刻を記録
        lastButtonStat = nowButtonStat;
    }

    if((now - keyingTime) > DEBOUNCE_TIME_MS && nowButtonStat != reportedButtonStat){ // 状態が安定かつ未報告
        risingEdgeBits |= (reportedButtonStat & ~nowButtonStat); // 立ち上がり（1→0）を蓄積
        reportedButtonStat = nowButtonStat;
        return (((~nowButtonStat>>BUTTON_S)&1)<<12)|(((~nowButtonStat>>BUTTON_E)&1)<<11)|(((~nowButtonStat>>BUTTON_B)&1)<<10)|(((~nowButtonStat>>BUTTON_0)&1)<<9)|(((~nowButtonStat>>BUTTON_9)&1)<<8)|(((~nowButtonStat>>BUTTON_8)&1)<<7)|(((~nowButtonStat>>BUTTON_7)&1)<<6)|(((~nowButtonStat>>BUTTON_6)&1)<<5)|(((~nowButtonStat>>BUTTON_5)&1)<<4)|(((~nowButtonStat>>BUTTON_4)&1)<<3)|(((~nowButtonStat>>BUTTON_3)&1)<<2)|(((~nowButtonStat>>BUTTON_2)&1)<<1)|(((~nowButtonStat>>BUTTON_1)&1)<<0);
    }

    return 0;
}

uint8_t Keyboard::getCharInput(){
    uint16_t buttonStat = idle();

    if(mode == NUMBER_MODE){
        const char* numChars[] = {"1","2","3","4","5","6","7","8","9","0"};
        for(int i = 0; i < 10; i++){
            if(buttonStat & (1 << i)){
                if(strlen(input) + 1 >= INPUT_MAX_LEN) return FAILURE;
                strcat(input, numChars[i]);
                break;
            }
        }

        if(buttonStat & (1 << 10)){ // Backspace
            if(strlen(input) > 0){
                input[strlen(input) - 1] = '\0';
            } else {
                #ifdef IS_SERIAL
                Serial.println("Nothing to delete.");
                #endif
                return FAILURE;
            }
        }
        if(buttonStat & (1 << 11)){ // Enter
            #ifdef IS_SERIAL
            Serial.println("-------------------------------------------");
            Serial.print("Entered: "); Serial.println(input);
            Serial.println("-------------------------------------------");
            #endif
            return ENTER;
        }
        if(buttonStat & (1 << 12)){ // Switch
            mode = ALPHABET_MODE;
            #ifdef IS_SERIAL
            Serial.println("Switched to ALPHABET mode.");
            #endif
        }
    } else if(mode == ALPHABET_MODE){ // ALPHABET_MODE
        // 数字ボタン（bit 0〜9）の押下チェック
        for(int i = 0; i < 10; i++){
            if(buttonStat & (1 << i)){
                if(pendingButtonBit == (uint8_t)i){
                    // 同じボタン → 次の候補へサイクル
                    pendingCharIndex = (pendingCharIndex + 1) % alpKeyBindsLen[i];
                } else {
                    // 別のボタン → 前の選択を確定してから新しい選択を開始
                    if(pendingButtonBit != 255){
                        if(strlen(input) + 1 >= INPUT_MAX_LEN) return FAILURE;
                        char ch[2] = {alpKeyBinds[pendingButtonBit][pendingCharIndex], '\0'};
                        strcat(input, ch);
                    }
                    pendingButtonBit = i;
                    pendingCharIndex = 0;
                }
                break;
            }
        }

        if(buttonStat & (1 << 10)){ // Backspace
            if(pendingButtonBit != 255){
                // 未確定文字をキャンセル
                pendingInit();
            } else if(strlen(input) > 0){
                input[strlen(input) - 1] = '\0';
            } else {
                #ifdef IS_SERIAL
                Serial.println("Nothing to delete.");
                #endif
                return FAILURE;
            }
        }
        if(buttonStat & (1 << 11)){ // Enter
            if(pendingButtonBit != 255){
                // 未確定文字を確定してバッファへ追加（満杯ならpendingをドロップして次のEnterで確定）
                if(strlen(input) + 1 < INPUT_MAX_LEN){
                    char ch[2] = {alpKeyBinds[pendingButtonBit][pendingCharIndex], '\0'};
                    strcat(input, ch);
                }
                pendingInit();
            } else {
                // 未確定なし → 入力完了
                #ifdef IS_SERIAL
                Serial.println("-------------------------------------------");
                Serial.print("Entered: "); Serial.println(input);
                Serial.println("-------------------------------------------");
                delay(1000);
                #endif
                return ENTER;
            }
        }
        if(buttonStat & (1 << 12)){ // Switch
            pendingInit();
            mode = NUMBER_MODE;
            #ifdef IS_SERIAL
            Serial.println("Switched to NUMBER mode.");
            #endif
        }
    }

    #ifdef IS_SERIAL
    Serial.print(input);
    if(mode == ALPHABET_MODE && pendingButtonBit != 255){
        Serial.print("[");
        Serial.print(alpKeyBinds[pendingButtonBit][pendingCharIndex]);
        Serial.print("]");
    }
    Serial.println();
    #endif

    return SUCCESS;
}

void Keyboard::pendingInit(){
    pendingButtonBit = 255;
    pendingCharIndex = 0;
}

void Keyboard::inputInit(){
    for(int i = 0; i < INPUT_MAX_LEN; i++){
        input[i] = '\0';
    }
}