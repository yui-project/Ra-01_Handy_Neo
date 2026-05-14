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