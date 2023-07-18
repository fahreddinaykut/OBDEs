#ifndef bleLib_H
#define bleLib_H
#include <Arduino.h>
#include "variables.h"
#include "config.h"
#include <NimBLECharacteristic.h>
#include "NimBLEDevice.h"
#include "esp_ota_ops.h"
#include "ArduinoJson.h"
#include "twaiLib.h"
#define CHUNK_SIZE 512
#define SERVICE_UUID "0000ffe0-0000-1000-8000-00805f9b34fc"
#define CHARACTERISTIC_UUID "0000ffe1-0000-1000-8000-00805f9b34fc"
#define FULL_PACKET 512
class bleLib; // forward declaration
class twaiLib; // forward declaration

typedef struct mybleLibraries {
    variables*  vars;
    twaiLib*    twai;

} bleLibs;

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
    int incomingStringLength;
    int receivedLength;
    std::string receivedData;
    int expectedLength=0;
    uint8_t PacketPreamble = 0x50;
    uint8_t notifyData[1] = {0x55};
    BLECallback(bleLib *ble) { myBleClass = ble; }
    bleLib *myBleClass;

    void onWrite(NimBLECharacteristic *pCharacteristic);
};
class bleLib
{
private:
public:
    int lastEase = 100;
    bleLibs libs;
    uint8_t PacketPreamble = 0x50;
    NimBLECharacteristic *cmdCharacteristic;
    NimBLEServer *pServer;
    bool updateFlag = false;
    bool readyFlag = false;
    void sendBLE(uint8_t data[], uint8_t dataLen);
    void sendOK();
    void sendNOK();
    void printArrays();
    void init(variables *VARS,twaiLib *TWAI);
    void init();
    void deinit();
    void mirror(std::string value);
    void sendJsonOverBle(DynamicJsonDocument &doc);
    void processData(std::string rawData);
};
#endif