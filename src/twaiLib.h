#pragma once
#include "Arduino.h"
#include "driver/twai.h"
#include <driver/gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "config.h"
#include "variables.h"
#include "bleLib.h"

static void twai_receive_task(void *arg);
class bleLib;
typedef struct myLibraries {
    variables*  vars;
    bleLib*     ble;

} GenericData_t;
class twaiLib
{
private:
public:
 GenericData_t libs;
    twaiLib();
    void init(variables *VARS,bleLib *BLE);
    void requestRPM();
    void requestSPEED();
};