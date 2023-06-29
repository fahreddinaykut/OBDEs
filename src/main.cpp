/*
  WebSerial Demo AP
  ------
  This example code works for both ESP8266 & ESP32 Microcontrollers
  WebSerial is accessible at 192.168.4.1/webserial URL.

  Author: Ayush Sharma
  Checkout WebSerial Pro: https://webserial.pro
*/
#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Arduino_JSON.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "twaiLib.h"
#include "variables.h"
#include "mainpage.h"
#include <DNSServer.h>
#include "bleLib.h"
DNSServer dnsServer;
bleLib ble;
#define APMODE
// #define STAMODE
#define ENABLE_SERIAL
// #include "esp32_can.h"            // https://github.com/collin80/esp32_can AND https://github.com/collin80/can_common
variables vars;
twaiLib twai;

AsyncWebServer server(80);

AsyncEventSource events("/events");
JSONVar readings;

unsigned long lastTime = 0;
unsigned long lastTimeTemperature = 0;
unsigned long lastTimeAcc = 0;
unsigned long gyroDelay = 10;
unsigned long temperatureDelay = 1000;
unsigned long accelerometerDelay = 200;

// Create a sensor object
Adafruit_MPU6050 mpu;

sensors_event_t a, g, temp;

float gyroX, gyroY, gyroZ;
float accX, accY, accZ;
float temperature;

// Gyroscope sensor deviation
float gyroXerror = 0.07;
float gyroYerror = 0.03;
float gyroZerror = 0.01;
TwoWire wr = TwoWire(0);
void initMPU()
{
  wr.begin(10, 8);
  if (!mpu.begin(MPU6050_I2CADDR_DEFAULT, &wr, 0))
  {
    Serial.println("Failed to find MPU6050 chip");
    while (1)
    {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
}
void initSPIFFS()
{
  if (!SPIFFS.begin())
  {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}
String getGyroReadings()
{
  mpu.getEvent(&a, &g, &temp);

  float gyroX_temp = g.gyro.x;
  if (abs(gyroX_temp) > gyroXerror)
  {
    gyroX += gyroX_temp / 50.00;
  }

  float gyroY_temp = g.gyro.y;
  if (abs(gyroY_temp) > gyroYerror)
  {
    gyroY += gyroY_temp / 70.00;
  }

  float gyroZ_temp = g.gyro.z;
  if (abs(gyroZ_temp) > gyroZerror)
  {
    gyroZ += gyroZ_temp / 90.00;
  }

  readings["gyroX"] = String(gyroX);
  readings["gyroY"] = String(gyroY);
  readings["gyroZ"] = String(gyroZ);

  String jsonString = JSON.stringify(readings);
  return jsonString;
}

String getAccReadings()
{
  mpu.getEvent(&a, &g, &temp);
  // Get current acceleration values
  accX = a.acceleration.x;
  accY = a.acceleration.y;
  accZ = a.acceleration.z;
  readings["accX"] = String(accX);
  readings["accY"] = String(accY);
  readings["accZ"] = String(accZ);
  String accString = JSON.stringify(readings);
  return accString;
}

String getTemperature()
{
  mpu.getEvent(&a, &g, &temp);
  temperature = temp.temperature;
  return String(temperature);
}

void startServer();
void handleRoot(AsyncWebServerRequest *request);
void handleSendMessage(AsyncWebServerRequest *request);
void handleSavePID(AsyncWebServerRequest *request);
void handleNotFound(AsyncWebServerRequest *request);
void handleSendCarData(AsyncWebServerRequest *request);
void handleIPAddress(AsyncWebServerRequest *request);

class CaptiveRequestHandler : public AsyncWebHandler
{
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request)
  {
    // request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/html", MAIN_PAGE);
  }
};
void startACCServer()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });

  server.serveStatic("/", SPIFFS, "/");

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    gyroX=0;
    gyroY=0;
    gyroZ=0;
    request->send(200, "text/plain", "OK"); });

  server.on("/resetX", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    gyroX=0;
    request->send(200, "text/plain", "OK"); });

  server.on("/resetY", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    gyroY=0;
    request->send(200, "text/plain", "OK"); });

  server.on("/resetZ", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    gyroZ=0;
    request->send(200, "text/plain", "OK"); });

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client)
                   {
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000); });
  server.addHandler(&events);

  server.begin();
}
void setup()
{
  pinMode(0, OUTPUT);
#ifdef ENABLE_SERIAL
  Serial.begin(115200);
#endif
#ifdef APMODE
  // WiFi.mode(WIFI_AP);
  // WiFi.softAP("Ride", "12345678");
  Serial.print("IP:");
  Serial.println(WiFi.softAPIP());
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

  twai.init(&vars);
  ble.init(&vars);
  // startServer();
  // initSPIFFS();
  initMPU();
  // startACCServer();
}

void loop()
{
  
  // dnsServer.processNextRequest();
  static long oldmillis = millis();
  if (millis() - oldmillis > 500)
  {
    if(vars.bleConnection){
    uint8_t data[4]={0x00,0x00,0x00,0x00};
    ble.sendStringOverBle("test1test2test3test4test5test6test7test8test9");
  }

    // if ((millis() - lastTime) > gyroDelay)
    // {
    //   // Send Events to the Web Server with the Sensor Readings
    //   events.send(getGyroReadings().c_str(), "gyro_readings", millis());
    //   lastTime = millis();
    // }
    // if ((millis() - lastTimeAcc) > accelerometerDelay)
    // {
    //   // Send Events to the Web Server with the Sensor Readings
    //   events.send(getAccReadings().c_str(), "accelerometer_readings", millis());
    //   lastTimeAcc = millis();
    // }
    // if ((millis() - lastTimeTemperature) > temperatureDelay)
    // {
    //   // Send Events to the Web Server with the Sensor Readings
    //   events.send(getTemperature().c_str(), "temperature_reading", millis());
    //   lastTimeTemperature = millis();
    // }
    // // twai.sendData();
    oldmillis = millis();
  }
}
void startServer()
{
  // server.on("/", HTTP_GET, handleRoot);
  server.on("/message", HTTP_GET, handleSendMessage);
  server.on("/carData", HTTP_GET, handleSendCarData);
  server.on("/ipAddress", HTTP_GET, handleIPAddress);
  server.on("/savePID", HTTP_GET, handleSavePID);
  server.onNotFound(handleNotFound);
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER); // only when requested from AP
}
void handleRoot(AsyncWebServerRequest *request)
{
  Serial.print(request->args());
  Serial.println(" Controller connected");
  request->send(200, "text/html", MAIN_PAGE);
}

void handleSendMessage(AsyncWebServerRequest *request)
{
  String liveData;
  liveData.concat(vars.message);
  liveData.concat("|");
  liveData.concat(vars.messageTimer);
  request->send(200, "text/plane", String(liveData));
}
void handleSendCarData(AsyncWebServerRequest *request)
{
  String carData;
  carData.concat((String)vars.SPEED);
  carData.concat("|");
  carData.concat((String)vars.RPM);
  carData.concat("|");
  carData.concat((String)vars.TEMP);
  carData.concat("|");
  carData.concat(millis());
  request->send(200, "text/plane", String(carData));
}
void handleIPAddress(AsyncWebServerRequest *request)
{
  String ipAddressData;
#ifdef APMODE
  IPAddress ipAddress = WiFi.softAPIP();
#else
  IPAddress ipAddress = WiFi.localIP();
#endif
  ipAddressData.concat(String(ipAddress[0]) + String(".") +
                       String(ipAddress[1]) + String(".") +
                       String(ipAddress[2]) + String(".") +
                       String(ipAddress[3]));

  request->send(200, "text/plane", String(ipAddressData));
}
void handleSavePID(AsyncWebServerRequest *request)
{
  String kp = request->arg("p");
  String ki = request->arg("i");
  // Serial.print("message1: ");
  // Serial.print(kp);
  // Serial.print("message2: ");
  // Serial.print(ki);
  request->send(200, "text/plane", "1");
  vars.sendMessage("RPM message sent!");
  twai.sendData();
}

void handleNotFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/html", PAGE_404);
}
