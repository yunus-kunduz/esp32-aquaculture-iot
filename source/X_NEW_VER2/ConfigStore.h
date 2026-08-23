/* 
 Copyright (c) 2019-2021 Sinric Pro

 The copyright in these pages (including without limitation all text, graphics and computer code relating thereto or associated therewith) 
 and all other intellectual property and proprietary rights therein belongs to Sinric Pro and all rights are reserved.

 Permission is given for the downloading and temporary storage of one or more of these pages for the sole purpose of viewing them on a stand-alone personal computer or monitor. 
 Permanent copying or redistribution, or reproduction of all or part of this library in any form is strictly prohibited without express permission of Sinric Pro.
*/

#pragma once 

#include <ArduinoJson.h>

#ifdef ESP32
  #include "nvs.h"
  #include "nvs_flash.h"
#endif

struct DeviceConfig
{
  char appKey[38];
  char appSecret[76];
  char sw1_id[26];
  char sw2_id[26];
  char sw3_id[26];
  char sw4_id[26];
  char sw5_id[26];
  char sw6_id[26];
  char cd1_id[26]; 
  char blnd1_id[26];
  char fan1_id[26];
};

class ConfigStore {
  public:
    ConfigStore(DeviceConfig &config);
    ~ConfigStore();
    
    bool loadConfig();
    bool saveJsonConfig(const JsonDocument &doc);
    bool clear();
    
  private:
    DeviceConfig &config;
    
    #ifdef ESP32
      Preferences preferences;
    #endif
};

ConfigStore::ConfigStore(DeviceConfig &config) : config(config) { }
ConfigStore::~ConfigStore(){ }

/**
* @brief Load the product configurations
* @return
*      ok
*/  
bool ConfigStore::loadConfig() {
  DEBUG_PROV(PSTR("[ConfigStore.loadConfig()] Loading config...\r\n"));

  // Local Change: Can't store in NVS. changed to config file.
  
  File configFile = SPIFFS.open(SC_CONFIG_FILE, "r");
  if (!configFile) {
    DEBUG_PROV(PSTR("[ConfigStore.loadConfig()]: Config file does not exists!\r\n"));
    return false;
  }

  DynamicJsonDocument doc(2048);
  DeserializationError err = deserializeJson(doc, configFile);

  if (err) {
    // Print size.
     Serial.print("File size: ");
     Serial.println(configFile.size());

     Serial.print("File contents: "); 
     while (configFile.available()) { 
      Serial.write(configFile.read());
     }
     
    configFile.close();
    DEBUG_PROV(PSTR("[ConfigStore.loadConfig()]: deserializeJson() failed: %s\r\n"), err.c_str());
    return false;
  }

  serializeJsonPretty(doc, Serial);
    
  strlcpy(config.appKey, doc[F("credentials")][F("appkey")] | "", sizeof(config.appKey));
  strlcpy(config.appSecret, doc[F("credentials")][F("appsecret")] | "", sizeof(config.appSecret));

  #if defined(ModuleS1)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));
    strlcpy(config.sw2_id, doc[F("devices")][1][F("id")] | "", sizeof(config.sw2_id));  
    
  #elif defined(ModuleS2)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));
    strlcpy(config.sw2_id, doc[F("devices")][1][F("id")] | "", sizeof(config.sw2_id));
          
  #elif defined(ModuleP)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));
    strlcpy(config.sw2_id, doc[F("devices")][1][F("id")] | "", sizeof(config.sw2_id));
    
  #elif defined(ModuleE)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));
    strlcpy(config.sw2_id, doc[F("devices")][1][F("id")] | "", sizeof(config.sw2_id));
    strlcpy(config.cd1_id, doc[F("devices")][2][F("id")] | "", sizeof(config.cd1_id));
    
  #elif defined(ModuleC)
    strlcpy(config.blnd1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.blnd1_id));
    
  #elif defined(ModuleT)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));
    strlcpy(config.sw2_id, doc[F("devices")][1][F("id")] | "", sizeof(config.sw2_id));
    
  #elif defined(ModuleR) || defined(ModuleR2)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));
    strlcpy(config.sw2_id, doc[F("devices")][1][F("id")] | "", sizeof(config.sw2_id));
    strlcpy(config.sw3_id, doc[F("devices")][2][F("id")] | "", sizeof(config.sw3_id));
    strlcpy(config.sw4_id, doc[F("devices")][3][F("id")] | "", sizeof(config.sw4_id));
    strlcpy(config.sw5_id, doc[F("devices")][4][F("id")] | "", sizeof(config.sw5_id));
    strlcpy(config.sw6_id, doc[F("devices")][5][F("id")] | "", sizeof(config.sw6_id));
    
  #elif defined(ModuleU) || defined(ModuleU2) || defined(ModuleU3) || defined(ModuleM3)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));
    strlcpy(config.sw2_id, doc[F("devices")][1][F("id")] | "", sizeof(config.sw2_id));
    strlcpy(config.sw3_id, doc[F("devices")][2][F("id")] | "", sizeof(config.sw3_id));
    strlcpy(config.sw4_id, doc[F("devices")][3][F("id")] | "", sizeof(config.sw4_id));
    strlcpy(config.fan1_id, doc[F("devices")][4][F("id")] | "", sizeof(config.fan1_id));    
    
  #elif defined(ModuleM1)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));
    strlcpy(config.sw2_id, doc[F("devices")][1][F("id")] | "", sizeof(config.sw2_id));
    strlcpy(config.sw3_id, doc[F("devices")][2][F("id")] | "", sizeof(config.sw3_id));
    strlcpy(config.sw4_id, doc[F("devices")][3][F("id")] | "", sizeof(config.sw4_id));
    strlcpy(config.sw5_id, doc[F("devices")][4][F("id")] | "", sizeof(config.sw5_id));
    strlcpy(config.sw6_id, doc[F("devices")][5][F("id")] | "", sizeof(config.sw6_id));
    strlcpy(config.fan1_id, doc[F("devices")][6][F("id")] | "", sizeof(config.fan1_id)); 

  #elif defined(ModuleM2)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));
    strlcpy(config.sw2_id, doc[F("devices")][1][F("id")] | "", sizeof(config.sw2_id));
    strlcpy(config.sw3_id, doc[F("devices")][2][F("id")] | "", sizeof(config.sw3_id));
    strlcpy(config.sw4_id, doc[F("devices")][3][F("id")] | "", sizeof(config.sw4_id));
    strlcpy(config.sw5_id, doc[F("devices")][4][F("id")] | "", sizeof(config.sw5_id));
    strlcpy(config.sw6_id, doc[F("devices")][5][F("id")] | "", sizeof(config.sw6_id));
    strlcpy(config.fan1_id, doc[F("devices")][6][F("id")] | "", sizeof(config.fan1_id)); 

  #elif defined(ModuleP2) || defined(ModuleT2) || defined(ModuleX)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));
    strlcpy(config.sw2_id, doc[F("devices")][1][F("id")] | "", sizeof(config.sw2_id));
    strlcpy(config.sw3_id, doc[F("devices")][2][F("id")] | "", sizeof(config.sw3_id));
    strlcpy(config.sw4_id, doc[F("devices")][3][F("id")] | "", sizeof(config.sw4_id));

  #elif defined(ModuleY)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));

  #elif defined(ModuleZ)
    strlcpy(config.fan1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.fan1_id));
          
  #else
    DEBUG_PROV(PSTR("ERROR! Module not found! \r\n"));
  #endif
   
  DEBUG_PROV(PSTR("success!\r\n"));
  doc.clear();
  configFile.close();
  return true;
}

/**
* @brief Save Json config to file
* @return
*      ok
*/ 
bool ConfigStore::saveJsonConfig(const JsonDocument &doc){
  DEBUG_PROV(PSTR("[ConfigStore.saveJsonConfig()]: Saving config...\r\n")); 
   
  String appKey = doc[F("credentials")][F("appkey")] | "";
  String appSecret =  doc[F("credentials")][F("appsecret")] | "";
  
  if(appKey.length() == 0 || appSecret.length() == 0) {
    DEBUG_PROV(PSTR("[ConfigStore.saveJsonConfig()]: Failed!. Invalid configurations!\r\n"));
    return false;
  }
 
  DEBUG_PROV(PSTR("[ConfigStore.saveJsonConfig()]: config: \r\n")); 
  serializeJsonPretty(doc, Serial);

  if(SPIFFS.exists(SC_CONFIG_FILE)) {
    DEBUG_PROV(PSTR("[ConfigStore.saveJsonConfig()]: Removing existing config file..\r\n")); 
    SPIFFS.remove(SC_CONFIG_FILE);
  }

  File configFile = SPIFFS.open(SC_CONFIG_FILE, "w");

  if (!configFile) {
    DEBUG_PROV(PSTR("[ConfigStore.saveJsonConfig] Open config file failed!!!\r\n")); 
    return false;
  }
 
  //size_t bytesWritten = serializeJson(doc, configFile);
  String jsonStr;
  serializeJson(doc, jsonStr);
  size_t bytesWritten = configFile.print(jsonStr);
  configFile.close();

  DEBUG_PROV(PSTR("[ConfigStore.saveJsonConfig] Bytes written: %u\r\n"), bytesWritten); 
  
  strlcpy(config.appKey, appKey.c_str(), sizeof(config.appKey));
  strlcpy(config.appSecret, appSecret.c_str(), sizeof(config.appSecret));

  #if defined(ModuleS1)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));
    strlcpy(config.sw2_id, doc[F("devices")][1][F("id")] | "", sizeof(config.sw2_id));  
    
  #elif defined(ModuleS2)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));
    strlcpy(config.sw2_id, doc[F("devices")][1][F("id")] | "", sizeof(config.sw2_id));  
    
  #elif defined(ModuleP)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));
    strlcpy(config.sw2_id, doc[F("devices")][1][F("id")] | "", sizeof(config.sw2_id));
    
  #elif defined(ModuleE)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));
    strlcpy(config.sw2_id, doc[F("devices")][1][F("id")] | "", sizeof(config.sw2_id));
    strlcpy(config.cd1_id, doc[F("devices")][2][F("id")] | "", sizeof(config.cd1_id));
    
  #elif defined(ModuleC)
    strlcpy(config.blnd1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.blnd1_id));
    
  #elif defined(ModuleT)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));
    strlcpy(config.sw2_id, doc[F("devices")][1][F("id")] | "", sizeof(config.sw2_id));
    
  #elif defined(ModuleR) || defined(ModuleR2)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));
    strlcpy(config.sw2_id, doc[F("devices")][1][F("id")] | "", sizeof(config.sw2_id));
    strlcpy(config.sw3_id, doc[F("devices")][2][F("id")] | "", sizeof(config.sw3_id));
    strlcpy(config.sw4_id, doc[F("devices")][3][F("id")] | "", sizeof(config.sw4_id));
    strlcpy(config.sw5_id, doc[F("devices")][4][F("id")] | "", sizeof(config.sw5_id));
    strlcpy(config.sw6_id, doc[F("devices")][5][F("id")] | "", sizeof(config.sw6_id));
    
  #elif defined(ModuleU) || defined(ModuleU2) || defined(ModuleU3) || defined(ModuleM3)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));
    strlcpy(config.sw2_id, doc[F("devices")][1][F("id")] | "", sizeof(config.sw2_id));
    strlcpy(config.sw3_id, doc[F("devices")][2][F("id")] | "", sizeof(config.sw3_id));
    strlcpy(config.sw4_id, doc[F("devices")][3][F("id")] | "", sizeof(config.sw4_id));
    strlcpy(config.fan1_id, doc[F("devices")][4][F("id")] | "", sizeof(config.fan1_id));
    
  #elif defined(ModuleM1)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));
    strlcpy(config.sw2_id, doc[F("devices")][1][F("id")] | "", sizeof(config.sw2_id));
    strlcpy(config.sw3_id, doc[F("devices")][2][F("id")] | "", sizeof(config.sw3_id));
    strlcpy(config.sw4_id, doc[F("devices")][3][F("id")] | "", sizeof(config.sw4_id));
    strlcpy(config.sw5_id, doc[F("devices")][4][F("id")] | "", sizeof(config.sw5_id));
    strlcpy(config.sw6_id, doc[F("devices")][5][F("id")] | "", sizeof(config.sw6_id));
    strlcpy(config.fan1_id, doc[F("devices")][6][F("id")] | "", sizeof(config.fan1_id)); 
    
  #elif defined(ModuleM2)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));
    strlcpy(config.sw2_id, doc[F("devices")][1][F("id")] | "", sizeof(config.sw2_id));
    strlcpy(config.sw3_id, doc[F("devices")][2][F("id")] | "", sizeof(config.sw3_id));
    strlcpy(config.sw4_id, doc[F("devices")][3][F("id")] | "", sizeof(config.sw4_id));
    strlcpy(config.sw5_id, doc[F("devices")][4][F("id")] | "", sizeof(config.sw5_id));
    strlcpy(config.sw6_id, doc[F("devices")][5][F("id")] | "", sizeof(config.sw6_id));
    strlcpy(config.fan1_id, doc[F("devices")][6][F("id")] | "", sizeof(config.fan1_id)); 
    
  #elif defined(ModuleP2) || defined(ModuleT2) || defined(ModuleX)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));
    strlcpy(config.sw2_id, doc[F("devices")][1][F("id")] | "", sizeof(config.sw2_id));
    strlcpy(config.sw3_id, doc[F("devices")][2][F("id")] | "", sizeof(config.sw3_id));
    strlcpy(config.sw4_id, doc[F("devices")][3][F("id")] | "", sizeof(config.sw4_id));
    
  #elif defined(ModuleY)
    strlcpy(config.sw1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.sw1_id));
    
  #elif defined(ModuleZ)
    strlcpy(config.fan1_id, doc[F("devices")][0][F("id")] | "", sizeof(config.fan1_id));
    
  #else
    DEBUG_PROV(PSTR("ERROR! Module not found! \r\n"));
  #endif
   
    
  DEBUG_PROV(PSTR("[ConfigStore.saveJsonConfig()]: success!\r\n")); 
  
  return true;
}

/**
* @brief Clear stored configuations
* @return
*      ok
*/ 
bool ConfigStore::clear(){
  DEBUG_PROV(PSTR("[ConfigStore.clear()]: Clear config...")); 
   
  SPIFFS.begin();
  SPIFFS.remove(SC_CONFIG_FILE);
  SPIFFS.end();

  memset(config.appKey, 0, sizeof(config.appKey));
  memset(config.appSecret, 0, sizeof(config.appSecret));

  #if defined(ModuleS1)
    memset(config.sw1_id, 0, sizeof(config.sw1_id));
    memset(config.sw2_id, 0, sizeof(config.sw2_id));
    
  #elif defined(ModuleS2)
    memset(config.sw1_id, 0, sizeof(config.sw1_id));
    memset(config.sw2_id, 0, sizeof(config.sw2_id));
    
  #elif defined(ModuleP)
    memset(config.sw1_id, 0, sizeof(config.sw1_id));
    memset(config.sw2_id, 0, sizeof(config.sw2_id));
    
  #elif defined(ModuleE)
    memset(config.sw1_id, 0, sizeof(config.sw1_id));
    memset(config.sw2_id, 0, sizeof(config.sw2_id));
    memset(config.cd1_id, 0, sizeof(config.cd1_id));
    
  #elif defined(ModuleC)
    memset(config.blnd1_id, 0, sizeof(config.blnd1_id));
    
  #elif defined(ModuleT)
    memset(config.sw1_id, 0, sizeof(config.sw1_id));
    memset(config.sw2_id, 0, sizeof(config.sw2_id));
    
  #elif defined(ModuleR) || defined(ModuleR2)
    memset(config.sw1_id, 0, sizeof(config.sw1_id));
    memset(config.sw2_id, 0, sizeof(config.sw2_id));
    memset(config.sw3_id, 0, sizeof(config.sw3_id));
    memset(config.sw4_id, 0, sizeof(config.sw4_id));
    memset(config.sw5_id, 0, sizeof(config.sw5_id));
    memset(config.sw6_id, 0, sizeof(config.sw6_id));
    
  #elif defined(ModuleU) || defined(ModuleU2) || defined(ModuleU3) || defined(ModuleM3)
    memset(config.sw1_id, 0, sizeof(config.sw1_id));
    memset(config.sw2_id, 0, sizeof(config.sw2_id));
    memset(config.sw3_id, 0, sizeof(config.sw3_id));
    memset(config.sw4_id, 0, sizeof(config.sw4_id));
    memset(config.fan1_id, 0, sizeof(config.fan1_id));  
    
  #elif defined(ModuleM1)
    memset(config.sw1_id, 0, sizeof(config.sw1_id));
    memset(config.sw2_id, 0, sizeof(config.sw2_id));
    memset(config.sw3_id, 0, sizeof(config.sw3_id));
    memset(config.sw4_id, 0, sizeof(config.sw4_id));
    memset(config.sw5_id, 0, sizeof(config.sw5_id));
    memset(config.sw6_id, 0, sizeof(config.sw6_id));
    memset(config.fan1_id, 0, sizeof(config.fan1_id));
    
  #elif defined(ModuleM2)
    memset(config.sw1_id, 0, sizeof(config.sw1_id));
    memset(config.sw2_id, 0, sizeof(config.sw2_id));
    memset(config.sw3_id, 0, sizeof(config.sw3_id));
    memset(config.sw4_id, 0, sizeof(config.sw4_id));
    memset(config.sw5_id, 0, sizeof(config.sw5_id));
    memset(config.sw6_id, 0, sizeof(config.sw6_id));
    memset(config.fan1_id, 0, sizeof(config.fan1_id));
    
  #elif defined(ModuleP2) || defined(ModuleT2) || defined(ModuleX)
    memset(config.sw1_id, 0, sizeof(config.sw1_id));
    memset(config.sw2_id, 0, sizeof(config.sw2_id));
    memset(config.sw3_id, 0, sizeof(config.sw3_id));
    memset(config.sw4_id, 0, sizeof(config.sw4_id));
    
  #elif defined(ModuleY)
    memset(config.sw1_id, 0, sizeof(config.sw1_id));
    
  #elif defined(ModuleZ)
    memset(config.fan1_id, 0, sizeof(config.fan1_id));
    
  #else
    DEBUG_PROV(PSTR("ERROR! Module not found! \r\n"));  
  #endif 

  DEBUG_PROV(PSTR("Done...\r\n")); 
  return true;
}
