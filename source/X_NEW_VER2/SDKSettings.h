/* 
 Copyright (c) 2019-2022 Sinric Pro

 The copyright in these pages (including without limitation all text, graphics and computer code relating thereto or associated therewith) 
 and all other intellectual property and proprietary rights therein belongs to Sinric Pro and all rights are reserved.

 Permission is given for the downloading and temporary storage of one or more of these pages for the sole purpose of viewing them on a stand-alone personal computer or monitor. 
 Permanent copying or redistribution, or reproduction of all or part of this library in any form is strictly prohibited without express permission of Sinric Pro.
*/

#pragma once 

// DO NOT CHANGE ! 

#include <ArduinoJson.h>
#include "lib/ProvUtil.h"

#define PROV_MODE_BLE                 0 /* Bluetooth (ESP32 only).  */
#define PROV_MODE_SMARTCONFIG         1 /* SmartConfig aka ESP-TOUCH */
#define PROV_MODE_SMART_AP            2 /* Smart AP mode via app */
#define PROV_MODE_CAPTIVE_PORTAL      3 /* On device captive like portal*/

#define PROV_VERSION                  "21"                  // Sketch version. DO NOT CHANGE
#define API_HOST                      "api.marvinno.in"  
#define OTA_HOST                      "ota.marvinno.in"
#define OTA_SERVER_PORT               80
#define OTA_HTTP_TIMEOUT              30000                 // OTA HTTP connect timeout.
#define HTTP_SERVER_PORT              80                    // HTTP Server port for AP mode
#define BLE_DEVICE_PREFIX             "PROV_X_"               // Must start with PROV_ 
#define SC_CONFIG_FILE                "/pconfig.json"       // product configuration file 
#define SC_NOTIFICATION_PORT          1982                  // Port to notify back to app.
#define STATIC_IP                     { 8, 8, 8, 8}  // SoftAP mode IP
#define GATEWAY                       { 8, 8, 8, 8 }
#define SUBNET                        { 255, 255, 255, 0 }
#define PROV_HTTP_REQUEST_SECRET      "7075746120"          // Used to sign auto configure http requests
#define OTA_SECRET                    "7075746120"          // Used to answer challenge
 
// for SmartConfig WiFi password decryption.
static const uint8_t SMARTCONFIG_CIPHER_KEY [16] = {0x06, 0xa9, 0x21, 0x40, 0x36, 0xb8, 0xa1, 0x5b, 0x51, 0x2e, 0x03, 0xd5, 0x34, 0x12, 0x00, 0x06};
static const uint8_t SMARTCONFIG_CIPHER_IV  [16] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};

// for SmartAP encryption.
static const uint8_t SMARTAP_CIPHER_KEY [16] = {2, 0, 1, 9, 0, 6, 2, 6, 1, 2, 5, 9, 0, 0, 0, 0};
static const uint8_t SMARTAP_CIPHER_IV [16] = {1, 9, 8, 4, 0, 6, 2, 6, 0, 6, 0, 5, 0, 0, 0, 0}; 
 
#if defined(ModuleS1)
  #define PRODUCT_CODE                  "marvinno.products.types.ModuleS1"
#elif defined(ModuleS2)
  #define PRODUCT_CODE                  "marvinno.products.types.ModuleS2"
#elif defined(ModuleP)
  #define PRODUCT_CODE                  "marvinno.products.types.ModuleP"
#elif defined(ModuleE)
  #define PRODUCT_CODE                  "marvinno.products.types.ModuleE"
#elif defined(ModuleC)
  #define PRODUCT_CODE                  "marvinno.products.types.ModuleC"
#elif defined(ModuleT)
  #define PRODUCT_CODE                  "marvinno.products.types.ModuleT"
#elif defined(ModuleR)
  #define PRODUCT_CODE                  "marvinno.products.types.ModuleR"
#elif defined(ModuleU)
  #define PRODUCT_CODE                  "marvinno.products.types.ModuleU"
#elif defined(ModuleM1)
  #define PRODUCT_CODE                  "marvinno.products.types.ModuleM1"
#elif defined(ModuleM2)
  #define PRODUCT_CODE                  "marvinno.products.types.ModuleM2"
#elif defined(ModuleU2)
  #define PRODUCT_CODE                  "marvinno.products.types.ModuleU2"
#elif defined(ModuleU3)
  #define PRODUCT_CODE                  "marvinno.products.types.ModuleU3"
#elif defined(ModuleM3)
  #define PRODUCT_CODE                  "marvinno.products.types.ModuleM3"
#elif defined(ModuleR2)
  #define PRODUCT_CODE                  "marvinno.products.types.ModuleR2"
#elif defined(ModuleP2)
  #define PRODUCT_CODE                  "marvinno.products.types.ModuleP2"
#elif defined(ModuleT2)
  #define PRODUCT_CODE                  "marvinno.products.types.ModuleT2"
#elif defined(ModuleX)
  #define PRODUCT_CODE                  "marvinno.products.types.ModuleX"
#elif defined(ModuleY)
  #define PRODUCT_CODE                  "marvinno.products.types.ModuleY"
#elif defined(ModuleZ)
  #define PRODUCT_CODE                  "marvinno.products.types.ModuleZ"
  
#endif

#define SWITCH_PRODUCT_CODE             "sinric.devices.types.SWITCH" 
#define BLIND_PRODUCT_CODE              "sinric.devices.types.BLIND" 
#define FAN_PRODUCT_CODE                "sinric.devices.types.FAN" 
#define CUSTOM_PRODUCT_CODE             "sinric.devices.types.CUSTOM" 

/**
* @brief Product structure setup.
* @return
*      Product setup
*/
String getProductStructure() { 
  DynamicJsonDocument doc(2048);
  String chipId = String(ProvUtil::getChipId32(), HEX);
    
  JsonObject productInfo = doc.createNestedObject(F("productInfo"));
  productInfo[F("type")] = PRODUCT_CODE;  
  productInfo[F("chipId")] = chipId;
  productInfo[F("bssid")] = ProvUtil::getMacAddress();

  JsonArray devices = doc.createNestedArray(F("devices"));
  
  #if defined(ModuleS1)
    JsonObject device1 = devices.createNestedObject();
    device1[F("name")] = "sw1_id"; 
    device1[F("code")] = SWITCH_PRODUCT_CODE;
  
    JsonObject device2 = devices.createNestedObject();
    device2[F("name")] = "sw2_id"; 
    device2[F("code")] = SWITCH_PRODUCT_CODE;

  #elif defined(ModuleS2)
    JsonObject device1 = devices.createNestedObject();
    device1[F("name")] = "sw1_id"; 
    device1[F("code")] = SWITCH_PRODUCT_CODE;
  
    JsonObject device2 = devices.createNestedObject();
    device2[F("name")] = "sw2_id"; 
    device2[F("code")] = SWITCH_PRODUCT_CODE;
    
  #elif defined(ModuleP)
    JsonObject device1 = devices.createNestedObject();
    device1[F("name")] = "sw1_id"; 
    device1[F("code")] = SWITCH_PRODUCT_CODE;
  
    JsonObject device2 = devices.createNestedObject();
    device2[F("name")] = "sw2_id"; 
    device2[F("code")] = SWITCH_PRODUCT_CODE;
    
  #elif defined(ModuleE)
    JsonObject device1 = devices.createNestedObject();
    device1[F("name")] = "sw1_id"; 
    device1[F("code")] = SWITCH_PRODUCT_CODE;
  
    JsonObject device2 = devices.createNestedObject();
    device2[F("name")] = "sw2_id"; 
    device2[F("code")] = SWITCH_PRODUCT_CODE;

    JsonObject device3 = devices.createNestedObject();
    device3[F("name")] = "cd1_id"; 
    device3[F("code")] = CUSTOM_PRODUCT_CODE;
    device3[F("productId")] = "6219f3ec888e5fd927f06d16"; 
    
  #elif defined(ModuleC)
    JsonObject device1 = devices.createNestedObject();
    device1[F("name")] = "blnd1_id"; 
    device1[F("code")] = BLIND_PRODUCT_CODE;
    
  #elif defined(ModuleT)
    JsonObject device1 = devices.createNestedObject();
    device1[F("name")] = "sw1_id"; 
    device1[F("code")] = SWITCH_PRODUCT_CODE;
  
    JsonObject device2 = devices.createNestedObject();
    device2[F("name")] = "sw2_id"; 
    device2[F("code")] = SWITCH_PRODUCT_CODE;
    
  #elif defined(ModuleR) || defined(ModuleR2)
    JsonObject device1 = devices.createNestedObject();
    device1[F("name")] = "sw1_id"; 
    device1[F("code")] = SWITCH_PRODUCT_CODE;
  
    JsonObject device2 = devices.createNestedObject();
    device2[F("name")] = "sw2_id"; 
    device2[F("code")] = SWITCH_PRODUCT_CODE;

    JsonObject device3 = devices.createNestedObject();
    device3[F("name")] = "sw3_id"; 
    device3[F("code")] = SWITCH_PRODUCT_CODE;
  
    JsonObject device4 = devices.createNestedObject();
    device4[F("name")] = "sw4_id"; 
    device4[F("code")] = SWITCH_PRODUCT_CODE;

    JsonObject device5 = devices.createNestedObject();
    device5[F("name")] = "sw5_id"; 
    device5[F("code")] = SWITCH_PRODUCT_CODE;
  
    JsonObject device6 = devices.createNestedObject();
    device6[F("name")] = "sw6_id"; 
    device6[F("code")] = SWITCH_PRODUCT_CODE;
    
  #elif defined(ModuleU) || defined(ModuleU2) || defined(ModuleU3) || defined(ModuleM3)
    JsonObject device1 = devices.createNestedObject();
    device1[F("name")] = "sw1_id"; 
    device1[F("code")] = SWITCH_PRODUCT_CODE;
  
    JsonObject device2 = devices.createNestedObject();
    device2[F("name")] = "sw2_id"; 
    device2[F("code")] = SWITCH_PRODUCT_CODE;

    JsonObject device3 = devices.createNestedObject();
    device3[F("name")] = "sw3_id"; 
    device3[F("code")] = SWITCH_PRODUCT_CODE;
  
    JsonObject device4 = devices.createNestedObject();
    device4[F("name")] = "sw4_id"; 
    device4[F("code")] = SWITCH_PRODUCT_CODE;

    JsonObject device5 = devices.createNestedObject();
    device5[F("name")] = "fan1_id"; 
    device5[F("code")] = FAN_PRODUCT_CODE;
    
  #elif defined(ModuleM1)
    JsonObject device1 = devices.createNestedObject();
    device1[F("name")] = "sw1_id"; 
    device1[F("code")] = SWITCH_PRODUCT_CODE;
  
    JsonObject device2 = devices.createNestedObject();
    device2[F("name")] = "sw2_id"; 
    device2[F("code")] = SWITCH_PRODUCT_CODE;

    JsonObject device3 = devices.createNestedObject();
    device3[F("name")] = "sw3_id"; 
    device3[F("code")] = SWITCH_PRODUCT_CODE;
  
    JsonObject device4 = devices.createNestedObject();
    device4[F("name")] = "sw4_id"; 
    device4[F("code")] = SWITCH_PRODUCT_CODE;

    JsonObject device5 = devices.createNestedObject();
    device5[F("name")] = "sw5_id"; 
    device5[F("code")] = SWITCH_PRODUCT_CODE;
  
    JsonObject device6 = devices.createNestedObject();
    device6[F("name")] = "sw6_id"; 
    device6[F("code")] = SWITCH_PRODUCT_CODE;

    JsonObject device7 = devices.createNestedObject();
    device7[F("name")] = "fan1_id"; 
    device7[F("code")] = FAN_PRODUCT_CODE;
    
  #elif defined(ModuleM2)
    JsonObject device1 = devices.createNestedObject();
    device1[F("name")] = "sw1_id"; 
    device1[F("code")] = SWITCH_PRODUCT_CODE;
  
    JsonObject device2 = devices.createNestedObject();
    device2[F("name")] = "sw2_id"; 
    device2[F("code")] = SWITCH_PRODUCT_CODE;

    JsonObject device3 = devices.createNestedObject();
    device3[F("name")] = "sw3_id"; 
    device3[F("code")] = SWITCH_PRODUCT_CODE;
  
    JsonObject device4 = devices.createNestedObject();
    device4[F("name")] = "sw4_id"; 
    device4[F("code")] = SWITCH_PRODUCT_CODE;

    JsonObject device5 = devices.createNestedObject();
    device5[F("name")] = "sw5_id"; 
    device5[F("code")] = SWITCH_PRODUCT_CODE;
  
    JsonObject device6 = devices.createNestedObject();
    device6[F("name")] = "sw6_id"; 
    device6[F("code")] = SWITCH_PRODUCT_CODE;

    JsonObject device7 = devices.createNestedObject();
    device7[F("name")] = "fan1_id"; 
    device7[F("code")] = FAN_PRODUCT_CODE;

  #elif defined(ModuleP2) || defined(ModuleT2) || defined(ModuleX)
    JsonObject device1 = devices.createNestedObject();
    device1[F("name")] = "sw1_id"; 
    device1[F("code")] = SWITCH_PRODUCT_CODE;
  
    JsonObject device2 = devices.createNestedObject();
    device2[F("name")] = "sw2_id"; 
    device2[F("code")] = SWITCH_PRODUCT_CODE;

    JsonObject device3 = devices.createNestedObject();
    device3[F("name")] = "sw3_id"; 
    device3[F("code")] = SWITCH_PRODUCT_CODE;
  
    JsonObject device4 = devices.createNestedObject();
    device4[F("name")] = "sw4_id"; 
    device4[F("code")] = SWITCH_PRODUCT_CODE;

  #elif defined(ModuleY)
    JsonObject device1 = devices.createNestedObject();
    device1[F("name")] = "sw1_id"; 
    device1[F("code")] = SWITCH_PRODUCT_CODE;

  #elif defined(ModuleZ)
    JsonObject device1 = devices.createNestedObject();
    device1[F("name")] = "fan1_id"; 
    device1[F("code")] = FAN_PRODUCT_CODE;
    
  #endif
   
  String data = "";
  serializeJson(doc, data);
 
  return data;
}
 
