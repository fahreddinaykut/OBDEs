/*
  WebSerial Demo AP
  ------
  This example code works for both ESP8266 & ESP32 Microcontrollers
  WebSerial is accessible at 192.168.4.1/webserial URL.

  Author: Ayush Sharma
  Checkout WebSerial Pro: https://webserial.pro
*/
#include <Arduino.h>
#include "twaiLib.h"
#include "variables.h"
#include "mainpage.h"
#include "bleLib.h"
bleLib ble;
#define APMODE
// #define STAMODE
#define ENABLE_SERIAL
// #include "esp32_can.h"            // https://github.com/collin80/esp32_can AND https://github.com/collin80/can_common
variables vars;
twaiLib twai;

void setup()
{
  pinMode(0, OUTPUT);
#ifdef ENABLE_SERIAL
  Serial.begin(115200);
#endif
#ifdef APMODE
  // WiFi.mode(WIFI_AP);
  // WiFi.softAP("Ride", "12345678");

#elif defined(STAMODE)
  WiFi.mode(WIFI_STA);
  WiFi.begin("Aykut", "edirne12345");
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
  Serial.print("IP:");
  Serial.println(WiFi.localIP());
#endif
  ble.init(&vars, &twai);
  twai.init(&vars, &ble);
}

void loop()
{

  // dnsServer.processNextRequest();
  static long oldmillis = millis();
  if (millis() - oldmillis > 1000)
  {
    if (vars.bleConnection)
    {

    }
    oldmillis = millis();
  }
}