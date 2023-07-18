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
};