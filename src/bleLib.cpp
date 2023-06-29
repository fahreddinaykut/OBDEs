#include "bleLib.h"
esp_ota_handle_t otaHandler = 0;
void ServerCallbacks::onConnect(NimBLEServer *pServer)
{
    Serial.println("ble connected");
    myBleClass->vars->bleConnection = true;
};

void ServerCallbacks::onDisconnect(NimBLEServer *pServer)
{
    Serial.println("ble disconnected");
    myBleClass->vars->bleConnection = false;
}
void BLECallback::onWrite(NimBLECharacteristic *pCharacteristic)
{
    std::string incomingData = pCharacteristic->getValue();
    static std::string value = "";
    static boolean waitForSecondHalf = false;
    static byte totalLen;
    static byte receivedLength;
    static boolean commandFullyReceived = false;
    if (!waitForSecondHalf)
    {
        totalLen = incomingData[0];
        value = incomingData;
        receivedLength = incomingData.length();
        if (totalLen > receivedLength)
        {
            waitForSecondHalf = true;
            commandFullyReceived = false;
        }
        else
        {
            waitForSecondHalf = false;
            commandFullyReceived = true;
        }
    }
    else
    {
        value += incomingData;
        receivedLength += incomingData.length();
        if (totalLen == receivedLength)
        {
            waitForSecondHalf = false;
            commandFullyReceived = true;
        }
        else
        {
            commandFullyReceived = false;
            waitForSecondHalf = true;
        }
    }
    if (commandFullyReceived)
    {
        value = value.substr(1);
        Serial.printf("Received %d bytes: %s\r\n", value.length(), value.c_str());
        value = "";
        commandFullyReceived = false;
        waitForSecondHalf = false;
    }
}
void otaCallback::onWrite(NimBLECharacteristic *pCharacteristic)
{
    std::string rxData = pCharacteristic->getValue();
    if (!_p_ble->updateFlag)
    { // If it's the first packet of OTA since bootup, begin OTA

        // _p_ble->vars->setmessage(8);
        disableCore0WDT();
        const esp_partition_t *update_partition = NULL;
        esp_err_t err;
        update_partition = esp_ota_get_next_update_partition(NULL);
        err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &otaHandler);
        enableCore0WDT();
        if (err != ESP_OK)
        {
            // _p_ble->vars->setmessage(0);
            // trlog.println("OTA Error: %s", esp_err_to_name(err));
        }
        _p_ble->updateFlag = true;
    }
    pCharacteristic->notify((uint8_t *)notifyData, 1, 1);
    if (rxData.length() > 0)
    {
        esp_ota_write(otaHandler, rxData.c_str(), rxData.length());
        if (rxData.length() != FULL_PACKET)
        {
            esp_ota_end(otaHandler);
            if (ESP_OK == esp_ota_set_boot_partition(esp_ota_get_next_update_partition(NULL)))
            {
                delay(2000);
                ESP.restart();
            }
            else
            {
            }
        }
    }
    // uint8_t txData[1] = { 0x55 };
    // pCharacteristic->setValue((uint8_t*)txData, 1);

    // _p_ble->vars->setmessage(8);
};
void bleLib::init(variables *VARS)
{
    vars = VARS;
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
        cmdService->createCharacteristic(CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::WRITE_NR);

    cmdCharacteristic->setValue(" ");
    cmdCharacteristic->setCallbacks(new BLECallback(this));
    cmdService->start();
    /////////// ota
    NimBLEService *pOtaService = pServer->createService(SERVICE_UUID_OTA);

    pOtaCharacteristic = pOtaService->createCharacteristic(CHARACTERISTIC_UUID_FW, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::WRITE);

    pOtaCharacteristic->setCallbacks(new otaCallback(this));
    pOtaService->start();
    /////////////////////

    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(cmdService->getUUID());
    pAdvertising->setScanResponse(true);
    pAdvertising->start();

    vars->blechar = cmdCharacteristic;
}
void bleLib::deinit()
{
    NimBLEDevice::deinit();
}

void bleLib::processData(std::string value)
{
    int index;

    //===========Some android BLE MTU 20 bytes fix. overhead is 4 bytes. if received value length is not equal to expected length wait for another packet and concat.

    //============MTU fix end===============
#ifdef BT_DEBUG

    // value.print("incomingData, length ");
    // USBSerial.print(value.length());
    // USBSerial.print(": ");
    // for (int i = 0; i < value.length(); i++) {
    //     USBSerial.print(" 0x");
    //     if (incomingData[i] == 0)
    //         USBSerial.print("0");
    //     USBSerial.print(incomingData[i], HEX);
    // }
    // USBSerial.println("");

    Serial.print("value, length ");
    Serial.printf("%d", value.length());
    Serial.print(": ");
    for (int i = 0; i < value.length(); i++)
    {
        Serial.print(" 0x");
        if (value[i] == 0)
            Serial.print("0");
        Serial.printf("%d", value[i]);
    }
    Serial.print("\n");
#endif
    if (((byte)value[0] == PacketPreamble))
    { // Preamble "T"
        switch ((byte)value[1])
        {
        case 0x1:
            vars->sendBLE("testmessage");
            break;
        default:
            break;
        }
    }
}
void bleLib::sendBLE(uint8_t data[], uint8_t dataLen)
{
    cmdCharacteristic->setValue(data, dataLen);
    cmdCharacteristic->notify();
}
void bleLib::sendStringOverBle(String string)
{
    byte stringLength = string.length();
    byte stringBytes[stringLength];
    string.getBytes(stringBytes, stringLength + 1);

    byte bytesWithLen[stringLength + 1]; // subtract 1 here
    bytesWithLen[0] = stringLength + 1;
    memcpy(bytesWithLen + 1, stringBytes, sizeof(stringBytes));
    int maxLength = 19; // the maximum length of each split array
    int bytesWithLenLength = stringLength + 1;
    for (int i = 0; i < bytesWithLenLength; i += maxLength)
    {
        int length = min(maxLength, bytesWithLenLength - i);
        byte splitBytes[length];
        memcpy(splitBytes, bytesWithLen + i, length);
        sendBLE(splitBytes, length);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        if (i + maxLength >= bytesWithLenLength)
        {
            break;
        }
    }
}
void bleLib::sendOK() {}
void bleLib::sendNOK() {}