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
static variables *staticVars;
   static void twai_receive_task(void *arg);
class twaiLib
{
private:
public:

 
    twaiLib();
    void init(variables *VARS);
    void sendData();
};