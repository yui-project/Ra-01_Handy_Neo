#include "Keyboard.h"

void Keyboard::begin(int ploadPin, int dataPin, int clockPin){
    sh.begin(int ploadPin, -1, int dataPin, int clockPin);
};

uint8_t getButtonStat(uint8_t num){
    if(num > BUTTON_NUM) return 255; // ボタン番号が上限値を越えている場合

    return sh.state(num);
}

uint16_t getAllButtonStat(){ // 下位ビット - ボタン0 〜 上位ビット - ボタン${BUTTON_NUM} で埋める
    uint16_t result = 0;

    for(int i = 0; i < BUTTON_NUM; i++){
        result |= (sh.state(i) << i);
    }

    return result;
}

void reloadButtonStat(){
    sh.update();
}