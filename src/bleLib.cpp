#include "bleLib.h"
void bleLib::init(variables *VARS,twaiLib *TWAI)
{
    libs={VARS,TWAI};
    bleLib::init();
}
void bleLib::init()
{
    NimBLEDevice::init("OBDEs");

    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
    NimBLEDevice::setMTU(517);
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks(this));

    NimBLEService *cmdService = pServer->createService(SERVICE_UUID);
    cmdCharacteristic =
        cmdService->createCharacteristic(CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::WRITE);

    cmdCharacteristic->setValue(" ");
    cmdCharacteristic->setCallbacks(new BLECallback(this));
    cmdService->start();

    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(cmdService->getUUID());
    pAdvertising->setScanResponse(true);
    pAdvertising->start();

    libs.vars->blechar = cmdCharacteristic;
}
void bleLib::deinit()
{
    NimBLEDevice::deinit();
}

void ServerCallbacks::onConnect(NimBLEServer *pServer)
{
    Serial.println("ble connected");
    myBleClass->libs.vars->bleConnection = true;
};

void ServerCallbacks::onDisconnect(NimBLEServer *pServer)
{
    Serial.println("ble disconnected");
    myBleClass->libs.vars->bleConnection = false;
}
void BLECallback::onWrite(NimBLECharacteristic *pCharacteristic)
{
    std::string newData = pCharacteristic->getValue();
    if (newData[0] == 0x54 && newData[1] == 0x54 && newData[2] == 0x54 && newData[3] == 0x54)
    {
        expectedLength = newData[4] + newData[5] * 256 + newData[6] * 65536 + newData[7] * 16777216;
        receivedData = "";
    }
    else
    {
        receivedData += newData;
        myBleClass->cmdCharacteristic->setValue((uint8_t *)notifyData, 1);
        // Check if the complete JSON data has been received
        if (receivedData.length() == expectedLength)
        {
            myBleClass->processData(receivedData);
            Serial.print("data:");
            Serial.println(receivedData.c_str());
            receivedData.clear();
        }
    }
}
void bleLib::processData(std::string rawData)
{
    Serial.println("processing...");
    DynamicJsonDocument doc(rawData.length()+1);
    deserializeJson(doc, rawData);
    const char *request = doc["type"];
    Serial.println(request);
    if ((String)request=="speed")
    {
        Serial.println("speed");
            libs.twai->requestSPEED();
             DynamicJsonDocument doc(512);
            doc["type"]="terminal";
            doc["value"]="Speed requested";
            sendJsonOverBle(doc);
    }else if ((String)request=="rpm")
    {
        Serial.println("rpm");
            libs.twai->requestRPM();
             DynamicJsonDocument doc(512);
            doc["type"]="terminal";
            doc["value"]="RPM requested";
            sendJsonOverBle(doc);
    }
}

void bleLib::sendBLE(uint8_t data[], uint8_t dataLen)
{
    cmdCharacteristic->setValue(data, dataLen);
    cmdCharacteristic->notify();
}

void bleLib::sendOK() {}
void bleLib::sendNOK() {}
void bleLib::sendJsonOverBle(DynamicJsonDocument &doc)
{
    String output;
    serializeJson(doc, output);
    int value = output.length();
    byte data[8] = {0x54, 0x54, 0x54, 0x54, (byte)(value & 0xff), (byte)((value >> 8) & 0xff), (byte)((value >> 16) & 0xff), (byte)((value >> 24) & 0xff)};
    cmdCharacteristic->setValue(data, 8);
    cmdCharacteristic->notify();
    vTaskDelay(20 / portTICK_PERIOD_MS);
    int maxChunkSize = 20;
    int chunks = (value / maxChunkSize) + 1;
    for (int i = 0; i < chunks; i++)
    {
        int index = i * maxChunkSize;
        int length = min(maxChunkSize, value - index);
        byte data[length + 1];
        output.getBytes(data, length + 1, index);
        cmdCharacteristic->setValue(data, length);
        cmdCharacteristic->notify();
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
    // sendStringOverBle(output);
}