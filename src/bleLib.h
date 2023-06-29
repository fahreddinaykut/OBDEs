#ifndef bleLib_H
#define bleLib_H
#include <Arduino.h>
#include "variables.h"
#include "config.h"
#include <NimBLECharacteristic.h>
#include "NimBLEDevice.h"
#include "esp_ota_ops.h"

#define SERVICE_UUID "0000ffe0-0000-1000-8000-00805f9b34fc"
#define CHARACTERISTIC_UUID "0000ffe1-0000-1000-8000-00805f9b34fc"
#define SERVICE_UUID_OTA "c8659210-af91-4ad3-a995-a58d6fd26145" // UART service UUID
#define CHARACTERISTIC_UUID_FW "c8659211-af91-4ad3-a995-a58d6fd26145"
#define CHARACTERISTIC_UUID_HW_VERSION "c8659212-af91-4ad3-a995-a58d6fd26145"
#define FULL_PACKET 512
class bleLib; // forward declaration
class ServerCallbacks : public NimBLEServerCallbacks
{
public:
    ServerCallbacks(bleLib *ble) { myBleClass = ble; }
    bleLib *myBleClass;
    void onConnect(NimBLEServer *pServer);
    void onDisconnect(NimBLEServer *pServer);
};
class BLECallback : public NimBLECharacteristicCallbacks
{
public:
std::string incomingString;
  int incomingStringLength;
  int receivedLength;
  
    uint8_t PacketPreamble = 0x50;
    BLECallback(bleLib *ble) { myBleClass = ble; }
    bleLib *myBleClass;

    void onWrite(NimBLECharacteristic *pCharacteristic);
};
class otaCallback : public NimBLECharacteristicCallbacks
{
public:
    uint8_t notifyData[1] = {0x55};
    otaCallback(bleLib *ble) { _p_ble = ble; }
    bleLib *_p_ble;

    void onWrite(NimBLECharacteristic *pCharacteristic);
};
class bleLib
{
private:
public:
    int lastEase = 100;
    uint8_t PacketPreamble = 0x50;
    NimBLECharacteristic *cmdCharacteristic;
    NimBLECharacteristic *pOtaCharacteristic = NULL;
    NimBLEServer *pServer;
    bool updateFlag = false;
    bool readyFlag = false;
    variables *vars;

    void processData(std::string value);
    void sendBLE(uint8_t data[], uint8_t dataLen);
    void sendOK();
    void sendNOK();
    void printArrays();
    void init(variables *VARS);
    void init();
    void deinit();
    void mirror(std::string value);
    void sendStringOverBle(String string);
};
#endif