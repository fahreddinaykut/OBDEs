#pragma once
#include "Arduino.h"
#include <NimBLECharacteristic.h>
class variables
{
private:
public:
    int16_t RPM;
    uint8_t SPEED;
    uint8_t TEMP;
    String message = "";
    String messageTimer = "0";
    uint8_t bleConnection=0;
    NimBLECharacteristic *blechar;
    uint8_t delayInterupt=0;
    void sendMessage(String msgtosent)
    {
        message = msgtosent;
        messageTimer = (String)millis();
    }
    void sendBLE(uint8_t data[], uint8_t dataLen)
    {

        blechar->setValue(data, dataLen);
        blechar->notify();
    }
    void sendBLE(String message)
    {
        Serial.print("Sending message len:");
        Serial.println(message.length());
        byte plain[message.length()];
        message.getBytes(plain, message.length());
        blechar->setValue(plain, message.length());
        blechar->notify();
    }
    void delay()
    {

    }
};