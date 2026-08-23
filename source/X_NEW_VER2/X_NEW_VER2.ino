/* 
 Copyright (c) 2019-2022 Sinric Pro

 The copyright in these pages (including without limitation all text, graphics and computer code relating thereto or associated therewith) 
 and all other intellectual property and proprietary rights therein belongs to Sinric Pro and all rights are reserved.

 Permission is given for the downloading and temporary storage of one or more of these pages for the sole purpose of viewing them on a stand-alone personal computer or monitor. 
 Permanent copying or redistribution, or reproduction of all or part of this library in any form is strictly prohibited without express permission of Sinric Pro.
*/

// Works with ESP8266/ESP32 only

// Notes:
//  1. Factory reset PIN is connected to GPIO 0

// Dependencies
//  1. ESP8266 Core   latest
//  2. WebSockets     latest
//  3. arduinoJson    latest
//  4. ESP32:         latest

// To enable Logs in ESP8266:
//  Tools -> Debug Serial Port -> Serial
//  Tools -> Flash Size -> 4MB (FS:1MB OTA:~1019KB)
//        -> Debug Level -> HTTP_CLIENT ** for AP issues (Optional)
//        -> Debug Level -> HTTP_UPDATE ** for OTA update issues (Optional)
// To enable Logs in ESP32:
//  Tools -> Flash Size -> Minimun SPIFF
//  Tools -> Core Debug Level -> Verbose

#if defined(ESP8266)
//  #define ModuleP
//  #define ModuleT
//  #define ModuleU
//  #define ModuleM2
#elif defined(ESP32)
//  #define ModuleS1
//  #define ModuleS2
//  #define ModuleR
//  #define ModuleM1
//  #define ModuleU
//  #define ModuleU2
//  #define ModuleU3
// #define ModuleM3
//  #define ModuleR2
//  #define ModuleP2
//  #define ModuleT2
#define ModuleX
// #define ModuleY
//  #define ModuleZ
//  #define ModuleC
//  #define ModuleE
#endif


#define ENABLE_DEBUG  // Enable Logs.

#if defined(ESP8266)
// Disable SSL on ESP8266 due to memory limitations
#define SINRICPRO_NOSSL
#endif

#ifdef ENABLE_DEBUG
#define DEBUG_ESP_PORT Serial
#define NODEBUG_WEBSOCKETS
#define NDEBUG
#define DEBUG_PROV_LOG  // Print provisioning debug logs
#endif

#include <Arduino.h>


#ifdef ESP32
#include <WiFi.h>
#include <Preferences.h>
#include "SPIFFS.h"
#else
#include <ESP8266WiFi.h>
#endif

#include "Settings.h"  // Must be above SinricPro.h

#include <SinricPro.h>
#include <SinricProConfig.h>
#include <SinricProSwitch.h>
#include <SinricProBlinds.h>
#include "WaterTank.h"
#include "MFan.h"
#include "WiFiMonitor.h"
#include "EEPROM.h"
#include <esp_task_wdt.h>


#if !defined(SINRICPRO_VERISON_INT) || (SINRICPRO_VERISON_INT < 2010003)
#error "Requires SinricPro SDK Version 2.10.3 or newer!!!"
#endif

#include "ConfigStore.h"
#include "lib/ProvUtil.h"
#include "lib/WiFiProv.h"
#include "lib/OTAUpdater.h"

#define BAUDRATE 115200
//#define DEVICE_RESET_PIN  0

#define SW1_RELAY_PIN 18  //Light2
#define SW2_RELAY_PIN 23  //Light5
#define SW3_RELAY_PIN 13  //Light3
#define SW4_RELAY_PIN 33  //Socket


#define button1 15
#define button2 5
#define button3 14
#define button4 25


bool button1PowerState;
bool button2PowerState;
bool button3PowerState;
bool button4PowerState;


unsigned long lastBtn1Press = 0;
unsigned long lastBtn2Press = 0;
unsigned long lastBtn3Press = 0;
unsigned long lastBtn4Press = 0;


bool rele1;
bool rele2;
bool rele3;
bool rele4;

SemaphoreHandle_t touchSemaphore4;
SemaphoreHandle_t touchSemaphore5;
SemaphoreHandle_t touchSemaphore6;
SemaphoreHandle_t touchSemaphore7;

int period = 5000;
unsigned long time_now = 0;
unsigned char m = 0;
void touch1(void);
void eeprem(void);
void handleFullReset(void);
const unsigned long timeout_uS = 600000;
DeviceConfig config;

#if defined(OTA_ENABLE)
OTAUpdater otaUpdater;
#endif


bool onPowerState(const String& deviceId, bool& state) {
  Serial.printf("[main.onPowerState()]: Device: %s, power state changed to %s\r\n", deviceId.c_str(), state ? "on" : "off");

  // Handle your power on/off command with device id. Example for ModuleS

  // Avoid delay() function!

#if defined(ModuleS1)
  if (strcmp(config.sw1_id, deviceId.c_str()) == 0) {  // is for switch 1 ?
    button1PowerState = state;
    digitalWrite(RELAY1_PIN, button1PowerState ? HIGH : LOW);

  } else if (strcmp(config.sw2_id, deviceId.c_str()) == 0) {  // is for switch 2 ?
    button2PowerState = state;
    digitalWrite(RELAY2_PIN, button2PowerState ? HIGH : LOW);

  } else {
    Serial.printf("[main.onPowerState()]: Devie device: %s not found!\r\n", deviceId.c_str());
  }
#elif defined(ModuleS2)
  if (strcmp(config.sw1_id, deviceId.c_str()) == 0) {  // is for switch 1 ?

  } else if (strcmp(config.sw2_id, deviceId.c_str()) == 0) {  // is for switch 2 ?
  }
#elif defined(ModuleP)
  if (strcmp(config.sw1_id, deviceId.c_str()) == 0) {  // is for switch 1 ?

  } else if (strcmp(config.sw2_id, deviceId.c_str()) == 0) {  // is for switch 2 ?
  }
#elif defined(ModuleE)
  if (strcmp(config.sw1_id, deviceId.c_str()) == 0) {  // is for switch 1 ?

  } else if (strcmp(config.sw2_id, deviceId.c_str()) == 0) {  // is for switch 2 ?
  }
#elif defined(ModuleC)
  if (strcmp(config.blnd1_id, deviceId.c_str()) == 0) {  // is for blind
  }
#elif defined(ModuleT)
  if (strcmp(config.sw1_id, deviceId.c_str()) == 0) {  // is for switch 1 ?

  } else if (strcmp(config.sw2_id, deviceId.c_str()) == 0) {  // is for switch 2 ?
  }
#elif defined(ModuleR) || defined(ModuleR2)
  if (strcmp(config.sw1_id, deviceId.c_str()) == 0) {  // is for switch 1 ?

  } else if (strcmp(config.sw2_id, deviceId.c_str()) == 0) {  // is for switch 2 ?

  } else if (strcmp(config.sw3_id, deviceId.c_str()) == 0) {  // is for switch 3 ?

  } else if (strcmp(config.sw4_id, deviceId.c_str()) == 0) {  // is for switch 4 ?

  } else if (strcmp(config.sw5_id, deviceId.c_str()) == 0) {  // is for switch 5 ?

  } else if (strcmp(config.sw6_id, deviceId.c_str()) == 0) {  // is for switch 6 ?
  }
#elif defined(ModuleU) || defined(ModuleU2) || defined(ModuleU3) || defined(ModuleM3)
  if (strcmp(config.sw1_id, deviceId.c_str()) == 0) {  // is for switch 1 ?

  } else if (strcmp(config.sw2_id, deviceId.c_str()) == 0) {  // is for switch 2 ?

  } else if (strcmp(config.sw3_id, deviceId.c_str()) == 0) {  // is for switch 3 ?

  } else if (strcmp(config.sw4_id, deviceId.c_str()) == 0) {  // is for switch 4 ?

  } else if (strcmp(config.fan1_id, deviceId.c_str()) == 0) {  // is for fan ?
  }
#elif defined(ModuleM1)
  if (strcmp(config.sw1_id, deviceId.c_str()) == 0) {  // is for switch 1 ?

  } else if (strcmp(config.sw2_id, deviceId.c_str()) == 0) {  // is for switch 2 ?

  } else if (strcmp(config.sw3_id, deviceId.c_str()) == 0) {  // is for switch 3 ?

  } else if (strcmp(config.sw4_id, deviceId.c_str()) == 0) {  // is for switch 4 ?

  } else if (strcmp(config.sw5_id, deviceId.c_str()) == 0) {  // is for switch 5 ?

  } else if (strcmp(config.sw6_id, deviceId.c_str()) == 0) {  // is for switch 6 ?

  } else if (strcmp(config.fan1_id, deviceId.c_str()) == 0) {  // is for fan ?
  }
#elif defined(ModuleM2)
  if (strcmp(config.sw1_id, deviceId.c_str()) == 0) {  // is for switch 1 ?

  } else if (strcmp(config.sw2_id, deviceId.c_str()) == 0) {  // is for switch 2 ?

  } else if (strcmp(config.sw3_id, deviceId.c_str()) == 0) {  // is for switch 3 ?

  } else if (strcmp(config.sw4_id, deviceId.c_str()) == 0) {  // is for switch 4 ?

  } else if (strcmp(config.sw5_id, deviceId.c_str()) == 0) {  // is for switch 5 ?

  } else if (strcmp(config.sw6_id, deviceId.c_str()) == 0) {  // is for switch 6 ?

  } else if (strcmp(config.fan1_id, deviceId.c_str()) == 0) {  // is for fan ?
  }
#elif defined(ModuleP2) || defined(ModuleT2) || defined(ModuleX)
  if (strcmp(config.sw1_id, deviceId.c_str()) == 0) {  // is for switch 1 ?

    digitalWrite(SW1_RELAY_PIN, state);
    EEPROM.write(1, state);
    button1PowerState = state;
    EEPROM.commit();

  } else if (strcmp(config.sw2_id, deviceId.c_str()) == 0) {  // is for switch 2 ?

    digitalWrite(SW2_RELAY_PIN, state);
    EEPROM.write(2, state);
    button2PowerState = state;
    EEPROM.commit();

  } else if (strcmp(config.sw3_id, deviceId.c_str()) == 0) {  // is for switch 3 ?
    digitalWrite(SW3_RELAY_PIN, state);
    button3PowerState = state;
    EEPROM.write(3, state);
    EEPROM.commit();

  } else if (strcmp(config.sw4_id, deviceId.c_str()) == 0) {  // is for switch 4 ?
    digitalWrite(SW4_RELAY_PIN, state);
    EEPROM.write(4, state);
    button4PowerState = state;
    EEPROM.commit();
  }
#elif defined(ModuleZ)
  if (strcmp(config.fan1_id, deviceId.c_str()) == 0) {  // is for fan ?
  }
#elif defined(ModuleY)
  if (strcmp(config.sw1_id, deviceId.c_str()) == 0) {  // is for switch 1 ?
    digitalWrite(SW1_RELAY_PIN, state);
    EEPROM.write(1, state);
    button1PowerState = state;
    EEPROM.commit();
  }
#endif
  return true;  // request handled properly
}

bool onRangeValue(const String& deviceId, int& position) {
  Serial.printf("Device %s set position to %d\r\n", deviceId.c_str(), position);

#if defined(ModuleU) || defined(ModuleU2) || defined(ModuleU3) || defined(ModuleM3)
#elif defined(ModuleM1)
#elif defined(ModuleM2)
#elif defined(ModuleZ)
#endif
  return true;  // request handled properly
}

bool onAdjustRangeValue(const String& deviceId, int& positionDelta) {
  Serial.printf("Device %s position changed %i\r\n", deviceId.c_str(), positionDelta);
#if defined(ModuleU)
#elif defined(ModuleU2)
#elif defined(ModuleU3)
#elif defined(ModuleM3)
#elif defined(ModuleZ)
#endif

  return true;  // request handled properly
}

void setWiFi(const char* ssid, const char* password) {
  Serial.printf("[main.setWiFi()]: Disconnect from current WiFi.\r\n");
  WiFi.disconnect();
  WiFi.persistent(false);

  Serial.printf("[main.setWiFi()]: Connecting to new WiFi:%s .\r\n", ssid);
#if defined(ESP32)
  WiFi.setMinSecurity(WIFI_AUTH_WEP);
#endif

  WiFi.begin(ssid, password);

  uint64_t start = millis();
  int timeout = 20 * 1000;  // 20 seconds
  touch1();
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");

    if (millis() - start > timeout) {
      break;
    }
  }

  Serial.printf("\r\n");

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[main.setWiFi()]: Connect success! Save new WiFi.\r\n");
    WiFi.persistent(true);
    WiFi.setAutoReconnect(true);
  } else {
    Serial.printf("[main.setWiFi()]: Connect failed.. Connect to last known WiFi.\r\n");
    ProvUtil::setupWiFi();  // Connect to Old WiFi..
  }
}

bool onSetSetting(const String& deviceId, const String& settingId, const String& settingValue) {
  Serial.printf("[main.onSetSetting()]: Device: %s, id: %s, value: %s\r\n", deviceId.c_str(), settingId.c_str(), settingValue.c_str());

  if (settingId.equals("setWiFi")) {
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, settingValue);

    if (error) {
      Serial.print(F("[main.onSetSetting()]: deserializeJson() failed: "));
      Serial.println(error.f_str());
      return false;
    }

    const char* ssid = doc[F("ssid")];          // "wifi"
    const char* password = doc[F("password")];  // "password"
    setWiFi(ssid, password);
  }

  return true;
}

/**
 * Setup devices.
 */
void setupSinricPro() {
#if defined(ModuleS1)
  Serial.printf("[setupSinricPro()]: Setup Module S1!\r\n");
  SinricProSwitch& mySwitch1 = SinricPro[config.sw1_id];
  mySwitch1.onPowerState(onPowerState);
  mySwitch1.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch2 = SinricPro[config.sw2_id];
  mySwitch2.onPowerState(onPowerState);
  mySwitch2.onSetSetting(onSetSetting);

#elif defined(ModuleS2)
  SinricProSwitch& mySwitch1 = SinricPro[config.sw1_id];
  mySwitch1.onPowerState(onPowerState);
  mySwitch1.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch2 = SinricPro[config.sw2_id];
  mySwitch2.onPowerState(onPowerState);
  mySwitch2.onSetSetting(onSetSetting);

#elif defined(ModuleP)
  SinricProSwitch& mySwitch1 = SinricPro[config.sw1_id];
  mySwitch1.onPowerState(onPowerState);
  mySwitch1.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch2 = SinricPro[config.sw2_id];
  mySwitch2.onPowerState(onPowerState);
  mySwitch2.onSetSetting(onSetSetting);

#elif defined(ModuleE)
  SinricProSwitch& mySwitch1 = SinricPro[config.sw1_id];
  mySwitch1.onPowerState(onPowerState);
  mySwitch1.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch2 = SinricPro[config.sw2_id];
  mySwitch2.onPowerState(onPowerState);
  mySwitch2.onSetSetting(onSetSetting);

  WaterTank& waterTank = SinricPro[config.cd1_id];
  // How to update the water level:
  //  waterTank.sendRangeValueEvent("rangeInstance1", 50); // 50%
  // Alexa, what is the water tank (device name) water level ?

#elif defined(ModuleC)
  SinricProBlinds& myBlinds = SinricPro[config.blnd1_id];
  myBlinds.onPowerState(onPowerState);
  myBlinds.onRangeValue(onRangeValue);
  myBlinds.onAdjustRangeValue(onAdjustRangeValue);

#elif defined(ModuleT)
  SinricProSwitch& mySwitch1 = SinricPro[config.sw1_id];
  mySwitch1.onPowerState(onPowerState);
  mySwitch1.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch2 = SinricPro[config.sw2_id];
  mySwitch2.onPowerState(onPowerState);
  mySwitch2.onSetSetting(onSetSetting);

#elif defined(ModuleR) || defined(ModuleR2)
  SinricProSwitch& mySwitch1 = SinricPro[config.sw1_id];
  mySwitch1.onPowerState(onPowerState);
  mySwitch1.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch2 = SinricPro[config.sw2_id];
  mySwitch2.onPowerState(onPowerState);
  mySwitch2.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch3 = SinricPro[config.sw3_id];
  mySwitch3.onPowerState(onPowerState);
  mySwitch3.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch4 = SinricPro[config.sw4_id];
  mySwitch4.onPowerState(onPowerState);
  mySwitch4.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch5 = SinricPro[config.sw5_id];
  mySwitch5.onPowerState(onPowerState);
  mySwitch5.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch6 = SinricPro[config.sw6_id];
  mySwitch6.onPowerState(onPowerState);
  mySwitch6.onSetSetting(onSetSetting);

#elif defined(ModuleU) || defined(ModuleU2) || defined(ModuleU3) || defined(ModuleM3)
  SinricProSwitch& mySwitch1 = SinricPro[config.sw1_id];
  mySwitch1.onPowerState(onPowerState);
  mySwitch1.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch2 = SinricPro[config.sw2_id];
  mySwitch2.onPowerState(onPowerState);
  mySwitch2.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch3 = SinricPro[config.sw3_id];
  mySwitch3.onPowerState(onPowerState);
  mySwitch3.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch4 = SinricPro[config.sw4_id];
  mySwitch4.onPowerState(onPowerState);
  mySwitch4.onSetSetting(onSetSetting);

  MFan& myFan = SinricPro[config.fan1_id];
  myFan.onPowerState(onPowerState);
  myFan.onRangeValue(onRangeValue);
  myFan.onAdjustRangeValue(onAdjustRangeValue);

  /*
      example of sending the temperature, humidity. Only 1 event per minute is allowed.      
      MFan &myFan = SinricPro[config.fan1_id];
      myFan.sendTemperatureEvent(temperature, humidity);
    */

#elif defined(ModuleM1)
  SinricProSwitch& mySwitch1 = SinricPro[config.sw1_id];
  mySwitch1.onPowerState(onPowerState);
  mySwitch1.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch2 = SinricPro[config.sw2_id];
  mySwitch2.onPowerState(onPowerState);
  mySwitch2.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch3 = SinricPro[config.sw3_id];
  mySwitch3.onPowerState(onPowerState);
  mySwitch3.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch4 = SinricPro[config.sw4_id];
  mySwitch4.onPowerState(onPowerState);
  mySwitch4.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch5 = SinricPro[config.sw5_id];
  mySwitch5.onPowerState(onPowerState);
  mySwitch5.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch6 = SinricPro[config.sw6_id];
  mySwitch6.onPowerState(onPowerState);
  mySwitch6.onSetSetting(onSetSetting);

  MFan& myFan = SinricPro[config.fan1_id];
  myFan.onPowerState(onPowerState);
  myFan.onRangeValue(onRangeValue);
  myFan.onAdjustRangeValue(onAdjustRangeValue);

#elif defined(ModuleM2)
  SinricProSwitch& mySwitch1 = SinricPro[config.sw1_id];
  mySwitch1.onPowerState(onPowerState);
  mySwitch1.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch2 = SinricPro[config.sw2_id];
  mySwitch2.onPowerState(onPowerState);
  mySwitch2.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch3 = SinricPro[config.sw3_id];
  mySwitch3.onPowerState(onPowerState);
  mySwitch3.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch4 = SinricPro[config.sw4_id];
  mySwitch4.onPowerState(onPowerState);
  mySwitch4.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch5 = SinricPro[config.sw5_id];
  mySwitch5.onPowerState(onPowerState);
  mySwitch5.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch6 = SinricPro[config.sw6_id];
  mySwitch6.onPowerState(onPowerState);
  mySwitch6.onSetSetting(onSetSetting);

  MFan& myFan = SinricPro[config.fan1_id];
  myFan.onPowerState(onPowerState);
  myFan.onRangeValue(onRangeValue);
  myFan.onAdjustRangeValue(onAdjustRangeValue);

#elif defined(ModuleP2) || defined(ModuleT2) || defined(ModuleX)
  SinricProSwitch& mySwitch1 = SinricPro[config.sw1_id];
  mySwitch1.onPowerState(onPowerState);
  mySwitch1.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch2 = SinricPro[config.sw2_id];
  mySwitch2.onPowerState(onPowerState);
  mySwitch2.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch3 = SinricPro[config.sw3_id];
  mySwitch3.onPowerState(onPowerState);
  mySwitch3.onSetSetting(onSetSetting);

  SinricProSwitch& mySwitch4 = SinricPro[config.sw4_id];
  mySwitch4.onPowerState(onPowerState);
  mySwitch4.onSetSetting(onSetSetting);

#elif defined(ModuleY)
  SinricProSwitch& mySwitch1 = SinricPro[config.sw1_id];
  mySwitch1.onPowerState(onPowerState);
  mySwitch1.onSetSetting(onSetSetting);

#elif defined(ModuleZ)
  MFan& myFan = SinricPro[config.fan1_id];
  myFan.onPowerState(onPowerState);
  myFan.onRangeValue(onRangeValue);
  myFan.onAdjustRangeValue(onAdjustRangeValue);

#else
  DEBUG_PROV(PSTR("ERROR! Module not found! \r\n"));
#endif

  SinricPro.onConnected([]() {
    Serial.printf("[main.setupSinricPro()]: Connected to marvinno\r\n");
  });
  SinricPro.onDisconnected([]() {
    Serial.printf("[main.setupSinricPro()]: Disconnected from marvinno\r\n");
    if (millis() >= timeout_uS) {
      ESP.restart();
    }
    touch1();
  });
  SinricPro.begin(config.appKey, config.appSecret, "ws.marvinno.in");
}


/**
 * Setup PINs for devices.
 */
void setupPins() {
  Serial.printf("[main.setupPins()]: Setup pin definition.\r\n");
  pinMode(SW1_RELAY_PIN, OUTPUT);
  pinMode(SW2_RELAY_PIN, OUTPUT);
  pinMode(SW3_RELAY_PIN, OUTPUT);
  pinMode(SW4_RELAY_PIN, OUTPUT);
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);
  pinMode(button4, INPUT);
}


void setup() {
  Serial.begin(BAUDRATE);
  Serial.println();
  delay(1000);
  EEPROM.begin(512);
  eeprem();

  touchSemaphore4 = xSemaphoreCreateBinary();

  xTaskCreate(&touch_task4, "touch_task4", 2048, NULL, 5, NULL);
  xTaskCreate(&processing_task4, "processing_task4", 2048, NULL, 5, NULL);


  touchSemaphore5 = xSemaphoreCreateBinary();

  xTaskCreate(&touch_task5, "touch_task5", 2048, NULL, 5, NULL);
  xTaskCreate(&processing_task5, "processing_task5", 2048, NULL, 5, NULL);


  touchSemaphore6 = xSemaphoreCreateBinary();

  xTaskCreate(&touch_task6, "touch_task6", 2048, NULL, 5, NULL);
  xTaskCreate(&processing_task6, "processing_task6", 2048, NULL, 5, NULL);


  touchSemaphore7 = xSemaphoreCreateBinary();

  xTaskCreate(&touch_task7, "touch_task7", 2048, NULL, 5, NULL);
  xTaskCreate(&processing_task7, "processing_task7", 2048, NULL, 5, NULL);



  uint64_t start = millis();
  int timeout1 = 15 * 60 * 1000;
  Serial.printf("[main.setup()]: Firmware: %s, SinricPro SDK: %s, Prov:%s\r\n", FIRMWARE_VERSION, SINRICPRO_VERSION, PROV_VERSION);
  Serial.printf("[main.setup()]: Initialize SPIFFS...\r\n");

#ifdef ESP32
  if (SPIFFS.begin(true)) {
    Serial.println(F("[main.setup()]: done."));
  } else {
    Serial.println(F("[main.setup()]: fail."));
  }
#else
  if (SPIFFS.begin()) {
    Serial.println(F("[main.setup()]: done."));
  } else {
    Serial.println(F("[main.setup()]: fail."));
  }
#endif

  delay(1000);

  WiFiProv prov(config);

  if (!prov.hasProvisioned()) {
    Serial.printf("[main.setup()]: Begin provisioning!\r\n");
    prov.beginProvision();
  } else {
    Serial.printf("[main.setup()]: Already provisioned!\r\n");

    Serial.printf("[main.setup()]: Setup Pins\r\n");
    setupPins();

    // Connect to WiFi
    while (!ProvUtil::setupWiFi()) {
      Serial.printf(PSTR("[main.setup()]: Cannot connect to WiFi any longer. WiFi Router down? Waiting 1 min to retry.\r\n"));
      delay(250);
      Serial.printf(PSTR("[main.setup()]: Trying again..."));
      if (millis() - start > timeout1) {
        ESP.restart();
      }
      touch1();
    }
  }


  Serial.printf("[main.setup()]: Monitor WiFi!\r\n");
  monitorWiFi();

  Serial.printf("[main.setup()]: Setup SinricPro!\r\n");
  setupSinricPro();

#if defined(OTA_ENABLE)
  Serial.printf("[main.setup()]: Setup OTA!\r\n");
  otaUpdater.setup();
#endif

  Serial.printf("[main.setup()]: Free Heap: %u\r\n", ESP.getFreeHeap());
  time_now = millis();
}

void handleFullReset() {
  //if (!digitalRead(DEVICE_RESET_PIN)){
  Serial.printf("[main.handleFullReset]: FULL RESET!\r\n");
  Serial.printf("[main.handleFullReset]: DELETING CONFIG!\r\n");

  ConfigStore configStore(config);
  configStore.clear();

  Serial.printf("[main.handleFullReset]: REBOOT\r\n");
  ESP.restart();
  //}
}

void eeprem() {
  rele1 = EEPROM.read(1);
  button1PowerState = rele1;
  digitalWrite(SW1_RELAY_PIN, rele1);
  rele2 = EEPROM.read(2);
  button2PowerState = rele2;
  digitalWrite(SW2_RELAY_PIN, rele2);
  rele3 = EEPROM.read(3);
  button3PowerState = rele3;
  digitalWrite(SW3_RELAY_PIN, rele3);
  rele4 = EEPROM.read(4);
  button4PowerState = rele4;
  digitalWrite(SW4_RELAY_PIN, rele4);
}

void touch1() {

  unsigned long actualMillis = millis();  // get actual millis() and keep it in variable actualMillis

  digitalWrite(SW4_RELAY_PIN, button4PowerState ? HIGH : LOW);  // if myPowerState indicates device turned on: turn on led (builtin led uses inverted logic: LOW = LED ON / HIGH = LED OFF)
  EEPROM.write(4, button4PowerState);
  EEPROM.commit();
  // get Switch device back
 // SinricProSwitch& mySwitch = SinricPro[config.sw4_id];
  //     send powerstate event
 // mySwitch.sendPowerStateEvent(button4PowerState);  // send the new powerState to SinricPro server
  //Serial.printf("Device %s turned %s (manually via flashbutton light4)\r\n", mySwitch.getDeviceId().c_str(), button4PowerState ? "on" : "off");

  lastBtn4Press = actualMillis;  // update last button press variable
  while (digitalRead(button4) == HIGH) {
    m = m + 1;
    delay(1000);
    Serial.print("In Reset Loop");
    if (digitalRead(button4) == LOW) {
      break;
    }
  }
  if (m > 5) {
    Serial.print("hard reset");
    delay(1000);
    digitalWrite(SW4_RELAY_PIN, HIGH);
    delay(500);
    digitalWrite(SW4_RELAY_PIN, LOW);
    delay(500);
    handleFullReset();
    // ESP.restart();
  }
  m = 0;
}

void loop() {
  // Avoid delay() function!
  touch1();
  if (WiFi.status() != WL_CONNECTED) {
    ESP.restart();
  }


  SinricPro.handle();

#if defined(OTA_ENABLE)
  otaUpdater.handleCheckAndUpdate();
#endif

  if (millis() > time_now + period) {
    time_now = millis();
    Serial.printf("[main.loop]: Free Heap: %u\r\n", ESP.getFreeHeap());
  }
}

void touch_task4(void* pvParameter) {
  while (1) {
    if (gpio_get_level(GPIO_NUM_15) == 1) {

      xSemaphoreGive(touchSemaphore4);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void processing_task4(void* pvParameter) {
  while (1) {
    if (xSemaphoreTake(touchSemaphore4, portMAX_DELAY)) {
      button1PowerState = !button1PowerState;
      digitalWrite(SW1_RELAY_PIN, button1PowerState);
      SinricProSwitch& mySwitch = SinricPro[config.sw1_id];
      mySwitch.sendPowerStateEvent(button1PowerState);
      EEPROM.write(1, button1PowerState);
      EEPROM.commit();
    }
  }
}

void touch_task5(void* pvParameter) {
  while (1) {
    if (gpio_get_level(GPIO_NUM_5) == 1) {

      xSemaphoreGive(touchSemaphore5);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void processing_task5(void* pvParameter) {
  while (1) {
    if (xSemaphoreTake(touchSemaphore5, portMAX_DELAY)) {
      button2PowerState = !button2PowerState;
      digitalWrite(SW2_RELAY_PIN, button2PowerState);
      SinricProSwitch& mySwitch = SinricPro[config.sw2_id];
      mySwitch.sendPowerStateEvent(button2PowerState);
      EEPROM.write(2, button2PowerState);
      // Serial.print("check itteration");
      EEPROM.commit();
    }
  }
}

void touch_task6(void* pvParameter) {
  while (1) {
    if (gpio_get_level(GPIO_NUM_14) == 1) {

      xSemaphoreGive(touchSemaphore6);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void processing_task6(void* pvParameter) {
  while (1) {
    if (xSemaphoreTake(touchSemaphore6, portMAX_DELAY)) {
      button3PowerState = !button3PowerState;
      digitalWrite(SW3_RELAY_PIN, button3PowerState);
      SinricProSwitch& mySwitch = SinricPro[config.sw3_id];
      mySwitch.sendPowerStateEvent(button3PowerState);
      EEPROM.write(3, button3PowerState);
      EEPROM.commit();
    }
  }
}

void touch_task7(void* pvParameter) {
  while (1) {
    if (gpio_get_level(GPIO_NUM_25) == 1) {

      xSemaphoreGive(touchSemaphore7);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void processing_task7(void* pvParameter) {
  while (1) {
    if (xSemaphoreTake(touchSemaphore7, portMAX_DELAY)) {
      button4PowerState = !button4PowerState;
      digitalWrite(SW4_RELAY_PIN, button4PowerState);
      SinricProSwitch& mySwitch = SinricPro[config.sw4_id];
      mySwitch.sendPowerStateEvent(button4PowerState);
      EEPROM.write(4, button4PowerState);
      EEPROM.commit();
    }
  }
}
