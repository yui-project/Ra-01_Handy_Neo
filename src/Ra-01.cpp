#include "Ra-01.h"

uint8_t Ra01::begin(uint8_t cs, uint8_t reset, uint8_t dio0){
    LoRa.setPins(cs, reset, dio0);
    if (!LoRa.begin(DEFAULT_FREQUENCY)) {
        #ifdef IS_SERIAL
        Serial.println("Starting LoRa failed!");
        #endif
        // while (1);
        return FAILURE;
    }
    return SUCCESS;
}

uint8_t Ra01::send(const char* data, uint8_t len){
    LoRa.beginPacket();
    LoRa.write((uint8_t*)data, len);
    return LoRa.endPacket();
}

uint8_t Ra01::recv(char* buf, uint8_t len){
    if (LoRa.parsePacket() > 0) {
        uint8_t i = 0;
        while (LoRa.available() && i < len - 1) {
            buf[i++] = (char)LoRa.read();
        }
        buf[i] = '\0'; // Null-terminate the string
        return i; // Return the number of bytes read
    }
    return 0; // No packet received
}

int Ra01::getRssi(){
    return LoRa.packetRssi();
}

uint8_t Ra01::setFreq(uint32_t freq){
    LoRa.setFrequency(freq);
    return SUCCESS;
}

uint8_t Ra01::setTxPower(uint8_t power){
    if(power < 2 || power > 20){
        return FAILURE; // 2~20以外は入力値として不適切
    }

    LoRa.setTxPower(power);
    return SUCCESS;
}

uint8_t Ra01::setSF(uint8_t sf){
    if(sf < 6 || sf > 12){
        return FAILURE; // 6~12以外は入力値として不適切
    }

    LoRa.setSpreadingFactor(sf);
    return SUCCESS;
}

uint8_t Ra01::setBW(uint8_t bw){
    if(bw > 9) return FAILURE; // 0~9以外は入力値として不適切

    LoRa.setSignalBandwidth(bw);
    return SUCCESS;
} 

/*
uint8_t Ra01::getSF(){
    return (uint8_t)LoRa.getSpreadingFactor();
}

uint8_t Ra01::getBW(){
    long bwLong = LoRa.getSignalBandwidth();

    if(bwLong == 7.8E3) return 0;
    else if(bwLong == 10.4E3) return 1;
    else if(bwLong == 15.6E3) return 2;
    else if(bwLong == 20.8E3) return 3;
    else if(bwLong == 31.25E3) return 4;
    else if(bwLong == 41.7E3) return 5;
    else if(bwLong == 62.5E3) return 6;
    else if(bwLong == 125E3) return 7;
    else if(bwLong == 250E3) return 8;
    else if(bwLong == 500E3) return 9;

    return FAILURE; // LoRaライブラリが返す値が上記のいずれでもない場合は異常値とみなす
}
*/