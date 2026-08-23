/* 
 Copyright (c) 2020-2021 Sinric Pro

 The copyright in these pages (including without limitation all text, graphics and computer code relating thereto or associated therewith) 
 and all other intellectual property and proprietary rights therein belongs to Sinric Pro and all rights are reserved.

 Permission is given for the downloading and temporary storage of one or more of these pages for the sole purpose of viewing them on a stand-alone personal computer or monitor. 
 Permanent copying or redistribution, or reproduction of all or part of this library in any form is strictly prohibited without express permission of Sinric Pro.
*/

#pragma once 
#ifdef ESP32

#include <NimBLEDevice.h>
#include <WiFi.h>
#include "CryptoMbedTLS.h" 
#include <ArduinoJson.h>
#include <NimBLEUUID.h>
#include "ProvDebug.h"

#if !defined(CONFIG_BT_NIMBLE_TASK_STACK_SIZE) || (CONFIG_BT_NIMBLE_TASK_STACK_SIZE != 12288)
#error "Please change CONFIG_BT_NIMBLE_TASK_STACK_SIZE to 12288 in NimBLE nimconfig.h"
#endif

class BLEProvClass : protected NimBLECharacteristicCallbacks, NimBLEServerCallbacks {
  public:
    using WiFiCredentialsCallback = std::function<bool(const String)>;
    using AuthCredentialsCallback = std::function<bool(const String)>;
    using DeviceInfoCallback = std::function<String(void)>;
    using BleProvDoneCallback = std::function<void(void)>;
    
    BLEProvClass();
    virtual ~BLEProvClass();

    void begin(const String &deviceName = "PROV_xx");
    void stop();
    void onWiFiCredentials(WiFiCredentialsCallback cb);
    void onAuthCredentials(AuthCredentialsCallback cb);
    void onDeviceInfo(DeviceInfoCallback cb);
    void onBleProvDone(BleProvDoneCallback cb);
    bool bleConfigDone();    
    String getBLEMac(); 
  
  private:
    void bufferResponse(NimBLECharacteristic * pCharacteristic, const std::string& jsonString);

  protected:
    void handleKeyExchange(const std::string& public_key_pem, NimBLECharacteristic* pCharacteristic);
    void handleWiFiConfig(const std::string& wificonfig, NimBLECharacteristic* pCharacteristic);
    void handleAuthConfig(const std::string& authconfig, NimBLECharacteristic* pCharacteristic);
    void handleWiFiList(NimBLECharacteristic* pCharacteristic);

    virtual void onWrite(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc) override;
    virtual void onConnect(NimBLEServer* pServer) override;
    virtual void onConnect(BLEServer* pServer, ble_gap_conn_desc* desc) override;
    virtual void onDisconnect(NimBLEServer* pServer) override;
    virtual void onMTUChange(uint16_t MTU, ble_gap_conn_desc* desc) override;

    bool _begin; 
    
    WiFiCredentialsCallback m_wifiCredentialsCallback;
    AuthCredentialsCallback m_authCredentialsCallback;
    DeviceInfoCallback m_deviceInfoCallback;
    BleProvDoneCallback m_bleProvDoneCallback;

    NimBLEServer *m_pServer;
    NimBLEService *m_pService;
    NimBLEAdvertising *m_pAdvertising;

    NimBLECharacteristic *m_provWiFiConfig; 
    NimBLECharacteristic *m_provWiFiConfigStatus; 
    NimBLECharacteristic *m_provKeyExchange;
    NimBLECharacteristic *m_provAuthConfig;  
    NimBLECharacteristic *m_provWiFiList;  
    NimBLECharacteristic *m_provWiFiListStatus;  

    CryptoMbedTLS m_crypto; 
    int m_authConfigSize = -1;
    std::string m_authConfigReceived;
    volatile bool m_provConfigDone = false;

    const std::string BLE_SERVICE_UUID            = "0000ffff-0000-1000-8000-00805f9b34fb";
    const std::string BLE_WIFI_CONFIG_UUID        = "00000001-0000-1000-8000-00805f9b34fb";
    const std::string BLE_KEY_EXCHANGE_UUID       = "00000002-0000-1000-8000-00805f9b34fb";
    const std::string BLE_AUTH_CONFIG_UUID        = "00000003-0000-1000-8000-00805f9b34fb";
    const std::string BLE_WIFI_CONFIG_STATUS_UUID = "00000004-0000-1000-8000-00805f9b34fb";    
    const std::string BLE_WIFI_LIST_UUID          = "00000005-0000-1000-8000-00805f9b34fb";    
    const std::string BLE_WIFI_LIST_STATUS_UUID   = "00000006-0000-1000-8000-00805f9b34fb";    

    NimBLEUUID m_uuidService;   
    NimBLEUUID m_uuidWiFiConfig; 
    NimBLEUUID m_uuidWiFiConfigStatus;
    NimBLEUUID m_uuidKeyExchange;
    NimBLEUUID m_uuidAuthConfig;
    NimBLEUUID m_uuidWiFiList;
    NimBLEUUID m_uuidWiFiListStatus;
};

BLEProvClass::BLEProvClass() 
: _begin(false)
, m_wifiCredentialsCallback(nullptr),
  m_authCredentialsCallback(nullptr),
  m_deviceInfoCallback(nullptr),
  m_authConfigReceived(""),
  m_uuidService(BLE_SERVICE_UUID),
  m_uuidWiFiConfig(BLE_WIFI_CONFIG_UUID),
  m_uuidWiFiConfigStatus(BLE_WIFI_CONFIG_STATUS_UUID),
  m_uuidKeyExchange(BLE_KEY_EXCHANGE_UUID),
  m_uuidAuthConfig(BLE_AUTH_CONFIG_UUID),  
  m_uuidWiFiList(BLE_WIFI_LIST_UUID),
  m_uuidWiFiListStatus(BLE_WIFI_LIST_STATUS_UUID) {}

/**
* @brief Setup BLE provisioning endpoints 
*/
void BLEProvClass::begin(const String &deviceName) {
  if (_begin) stop();

  DEBUG_PROV(PSTR("[BLEProvClass.begin] Setup BLE endpoints ..\r\n"));
  
  NimBLEDevice::init(deviceName.c_str());
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY); 
  NimBLEDevice::setMTU(512);
   
  m_pServer = NimBLEDevice::createServer();
  m_pServer->setCallbacks(this);
  m_pServer->advertiseOnDisconnect(false);

  m_pService = m_pServer->createService(m_uuidService);

  m_provWiFiConfig = m_pService->createCharacteristic(m_uuidWiFiConfig, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE); 
  m_provWiFiConfig->setValue("wificonfig");
  m_provWiFiConfig->setCallbacks(this);

  m_provKeyExchange = m_pService->createCharacteristic(m_uuidKeyExchange, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE); 
  m_provKeyExchange->setValue("keyexchange");
  m_provKeyExchange->setCallbacks(this); 

  m_provAuthConfig = m_pService->createCharacteristic(m_uuidAuthConfig, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE); 
  m_provAuthConfig->setValue("auth_config");
  m_provAuthConfig->setCallbacks(this); 

  m_provWiFiConfigStatus = m_pService->createCharacteristic(m_uuidWiFiConfigStatus, NIMBLE_PROPERTY::NOTIFY); 
  m_provWiFiConfigStatus->setValue("wificonfigstatus");
  m_provWiFiConfigStatus->setCallbacks(this);

  m_provWiFiList = m_pService->createCharacteristic(m_uuidWiFiList, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE); 
  m_provWiFiList->setValue("wifi_list");
  m_provWiFiList->setCallbacks(this); 

  m_provWiFiListStatus = m_pService->createCharacteristic(m_uuidWiFiListStatus, NIMBLE_PROPERTY::NOTIFY); 
  m_provWiFiListStatus->setValue("wifi_list_status");
  m_provWiFiListStatus->setCallbacks(this);
   
  m_pService->start();

  m_pAdvertising = NimBLEDevice::getAdvertising();
  m_pAdvertising->setScanResponse(true); // must be true or else BLE name gets truncated
  
  m_pAdvertising->addServiceUUID(m_uuidService);
  m_pAdvertising->start(); 

  DEBUG_PROV(PSTR("[BLEProvClass.begin] done!\r\n"));
  
  _begin = true;
}


/**
* @brief Generate a session encryption key.
*/
void BLEProvClass::handleKeyExchange(const std::string& public_key_pem, NimBLECharacteristic* pCharacteristic) {
    std::string result;
    
    if(m_crypto.init_mbedtls()) {
      m_crypto.getSharedSecret(public_key_pem, result);      
    }    

    m_crypto.deinit_mbedtls();

    DEBUG_PROV(PSTR("[BLEProvClass.handleKeyExchange()] Session key is %s...\r\n"), result.c_str());      

    pCharacteristic->setValue(result);
    pCharacteristic->notify(true);
}
 
/**
* @brief Called when mobile sends Sinric Pro credentials (appkey, secret, devceids). 
* Mobile sends authentication config string in chucks due to BLE limitations
*/
void BLEProvClass::handleAuthConfig(const std::string& authConfigChuck, NimBLECharacteristic* pCharacteristic) {
  if(m_authConfigSize == -1) {
      m_authConfigSize = std::atoi(authConfigChuck.c_str());
      DEBUG_PROV(PSTR("[BLEProvClass.handleAuthConfig()] Config payload size: %d\r\n"), m_authConfigSize);
  } else {
    // Append data chucks
    m_authConfigReceived.append(authConfigChuck);
    DEBUG_PROV(PSTR("[BLEProvClass.handleAuthConfig()] %d/%d\r\n"), m_authConfigReceived.size(), m_authConfigSize);
    
    if(m_authConfigReceived.size() == m_authConfigSize) {
      m_authConfigSize = -1;
      DEBUG_PROV(PSTR("[BLEProvClass.handleAuthConfig()] Config payload receive completed\r\n")); 
      
      if (m_authCredentialsCallback) {
         std::vector<uint8_t> decodedConfig = m_crypto.base64Decode(m_authConfigReceived);
         m_crypto.aesCTRXdecrypt(m_crypto.key, m_crypto.iv, decodedConfig);
         std::string authConfig(decodedConfig.begin(), decodedConfig.end());    
    
         DEBUG_PROV(PSTR("[BLEProvClass.handleAuthConfig()] Decrypted config: %s\r\n"), authConfig.c_str());  
    
         // Connect to WiFi      
         bool success = m_authCredentialsCallback(String(authConfig.c_str())); 
         std::string jsonString;
         
         StaticJsonDocument<200> doc;
         doc["success"] = success ? true : false;
         serializeJsonPretty(doc, jsonString); 
         DEBUG_PROV(PSTR("[BLEProvClass.handleAuthConfig()] Response: %s\r\n"), jsonString.c_str());    
    
         pCharacteristic->setValue(jsonString);
         pCharacteristic->notify(true);
         DEBUG_PROV(PSTR("[BLEProvClass.handleAuthConfig()] Notified!\r\n"));    

         // Wait until client gets the response before we wrap up.
         ProvUtil::wait(2000);
      
         m_provConfigDone = true;

         if(success) ProvState::getInstance()->state = ProvState::MODE_PROV_DONE;
          
         if(success && m_bleProvDoneCallback) {
            m_bleProvDoneCallback();
         }
        } else {
          DEBUG_PROV(PSTR("[BLEProvClass.handleAuthConfig()] Auth callback not defined!\r\n"));  
          
          std::string jsonString;
          StaticJsonDocument<200> doc;
          doc[F("success")] = false;
          doc[F("message")] = F("Failed set authentication (nocallback)..");
          serializeJsonPretty(doc, jsonString);
          pCharacteristic->setValue(jsonString);
          pCharacteristic->notify(true);
        }    
    }          
  }     
}  

/**
* @brief Called when mobile sends WiFi credentials.
*/
void BLEProvClass::handleWiFiConfig(const std::string& wificonfig, NimBLECharacteristic* pCharacteristic) {
  if (m_wifiCredentialsCallback && m_deviceInfoCallback) {
     std::vector<uint8_t> decodedConfig = m_crypto.base64Decode(wificonfig);
     m_crypto.aesCTRXdecrypt(m_crypto.key, m_crypto.iv, decodedConfig);
     std::string wiFiConfig(decodedConfig.begin(), decodedConfig.end());

     DEBUG_PROV(PSTR("[BLEProvClass.handleWiFiConfig()] Wi-Fi config: %s\r\n"), wiFiConfig.c_str());  

     // Connect to WiFi      
     ProvState::getInstance()->state = ProvState::MODE_CONNECTING_WIFI;
     
     bool success = m_wifiCredentialsCallback(String(wiFiConfig.c_str())); 

     if(success) {
        ProvState::getInstance()->state = ProvState::MODE_WIFI_CONNECTED_WAIT_APP_CREDENTIALS; 
     } else {
        ProvState::getInstance()->state = ProvState::MODE_WAIT_WIFI_CONFIG;
     }

     std::string jsonString = "";
     
     if(success) {
        jsonString.append("{\"success\":true, \"message\":\"Success\", ");
        jsonString.append("\"bssid\":\""); jsonString.append(String(WiFi.macAddress()).c_str()); jsonString.append("\", ");
        jsonString.append("\"ip\":\""); jsonString.append(WiFi.localIP().toString().c_str()); jsonString.append("\", ");         
        jsonString.append("\"deviceInfo\":");
        jsonString.append(getProductStructure().c_str());
        jsonString.append("}");
     } else {
       jsonString.append("{\"success\":false, \"message\":\"Failed to connect to WiFi..\"}");
     }

     DEBUG_PROV(PSTR("[BLEProvClass.handleWiFiConfig()] WiFi Config response size: %u\r\n"), jsonString.length());    
     DEBUG_PROV(PSTR("[BLEProvClass.handleWiFiConfig()] WiFi Config response: %s\r\n"), jsonString.c_str());    
      
     // Due to BLE packet size limitation, we write data in 180 byte chuncks
     bufferResponse(m_provWiFiConfigStatus, jsonString);

     DEBUG_PROV(PSTR("[BLEProvClass.handleWiFiConfig()] Done!\r\n"));          
    } else {
      DEBUG_PROV(PSTR("[BLEProvClass.handleWiFiConfig()] m_wifiCredentialsCallback or m_deviceInfoCallback not set!\r\n"));    
      
      std::string jsonString;
      StaticJsonDocument<200> doc;
      doc[F("success")] = false;
      doc[F("message")] = F("m_deviceInfoCallback not set!..");
      serializeJsonPretty(doc, jsonString);
      m_provWiFiConfigStatus->setValue(jsonString);
      m_provWiFiConfigStatus->notify();
   } 
}

/**
* @brief Called when mobile wants a list of WiFis ESP can connect to.
*/
void BLEProvClass::handleWiFiList(NimBLECharacteristic* pCharacteristic) {
  int total = WiFi.scanNetworks();

  String jsonString = "[";
  for (int i = 0; i < total; ++i) {
      if(i != 0) jsonString += ",";
      jsonString += "{\"ssid\":\"" + WiFi.SSID(i) + "\",";
      jsonString += "\"rssi\":" + String(WiFi.RSSI(i)) + "}";
  }
  jsonString += "]";

  DEBUG_PROV(PSTR("[BLEProvClass.handleWiFiList()] WiFi list: %s\r\n"), jsonString.c_str());

  // Free memory
  WiFi.scanDelete();
   
  // Due to BLE packet size limitation, we write data in 180 byte chuncks
  bufferResponse(m_provWiFiListStatus, std::string(jsonString.c_str()));      
      
  DEBUG_PROV(PSTR("[BLEProvClass.handleWiFiList()] Done!\r\n"));    
}

void BLEProvClass::bufferResponse(NimBLECharacteristic * pCharacteristic, const std::string& jsonString) {
  // Write response length
  pCharacteristic->setValue(ProvUtil::to_string(jsonString.length()));
  pCharacteristic->notify(true);
  delay(500);

  // Write response data
  int offset          = 0;
  int remainingLength = jsonString.length();
  const uint8_t* str  = reinterpret_cast<const uint8_t*>(jsonString.c_str());

  while (remainingLength > 0) {
    int bytesToSend = min(180, remainingLength); // send 180 bytes until all the bytes are sent
    DEBUG_PROV(PSTR("[BLEProvClass.bufferResponse()] Sending %u bytes!\r\n"), bytesToSend);    
    pCharacteristic->setValue((str + offset), bytesToSend);
    pCharacteristic->notify();
    delay(10);
    remainingLength -= bytesToSend;
    offset += bytesToSend;
  }
}

/**
* @brief Called when receive data via BLE.
*/
void BLEProvClass::onWrite(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc) {
  DEBUG_PROV(PSTR("[BLEProvClass.onWrite()] Got: %s\r\n"), pCharacteristic->getValue().c_str());
  
  if (pCharacteristic == m_provKeyExchange && m_provKeyExchange->getDataLength()) { 
     handleKeyExchange(m_provKeyExchange->getValue(), pCharacteristic); 
  }        
  else if (pCharacteristic == m_provWiFiConfig && m_provWiFiConfig->getDataLength()) { 
    handleWiFiConfig(m_provWiFiConfig->getValue(), pCharacteristic);
  }
  else if (pCharacteristic == m_provAuthConfig && m_provAuthConfig->getDataLength()) { 
    handleAuthConfig(m_provAuthConfig->getValue(), pCharacteristic);
  } 
  else if (pCharacteristic == m_provWiFiList) { 
    handleWiFiList(pCharacteristic);
  }     
}

/**
* @brief Called when client connect.
*/
void BLEProvClass::onConnect(NimBLEServer* pServer) {
  DEBUG_PROV(PSTR("[BLEProvClass.onConnect()]: Client connected\r\n"));
}

/**
* @brief Show connected client MTU.
*/
void BLEProvClass::onConnect(BLEServer* pServer, ble_gap_conn_desc* desc) {
  int bleClientMTU = pServer->getPeerMTU(desc->conn_handle);
  DEBUG_PROV(PSTR("[BLEProvClass.onConnect()]: MTU of client: %d\r\n"), bleClientMTU);
}

/**
* @brief Called when client disconnect..
*/
void BLEProvClass::onDisconnect(NimBLEServer* pServer) {
  DEBUG_PROV(PSTR("[BLEProvClass.onDisconnect()]: Client disconnected\r\n"));

  if (_begin) { 
    DEBUG_PROV(PSTR("[BLEProvClass.onDisconnect()]: Start advertising\r\n"));
    m_pAdvertising->start();
  } 
}

/**
* @brief Called when client request for different MTU..
*/
void BLEProvClass::onMTUChange(uint16_t MTU, ble_gap_conn_desc* desc) {
  DEBUG_PROV(PSTR("[BLEProvClass.onMTUChange()]: MTU updated: %u for connection ID: %u\r\n"), MTU, desc->conn_handle);
};

/**
* @brief Stop BLE provisioning..
*/
void BLEProvClass::stop() {
  if (_begin) {
    m_pAdvertising->stop();
    //NimBLEDevice::deinit(); // do not deinit here. still sending data to mobile.
    _begin = false;
  }
}

/**
* @brief Get called when receive WiFi credentials
*/
void BLEProvClass::onWiFiCredentials(WiFiCredentialsCallback cb) {
  m_wifiCredentialsCallback = cb;
}

/**
* @brief Get called when receive authentication configurations
*/
void BLEProvClass::onAuthCredentials(AuthCredentialsCallback cb) {
  m_authCredentialsCallback = cb;
}

/**
* @brief Get called when requesing for production structure
*/
void BLEProvClass::onDeviceInfo(DeviceInfoCallback cb) {
  m_deviceInfoCallback = cb;
}

/**
* @brief Get called when BLE provisioning completed
*/
void BLEProvClass::onBleProvDone(BleProvDoneCallback cb) {
  m_bleProvDoneCallback = cb;
}

/**
* @brief Get called when BLE MAC required
*/
String BLEProvClass::getBLEMac() {
  return String(NimBLEDevice::getAddress().toString().c_str());
}

/**
* @brief Get called to check whether BLE configuration has finished
*/
bool BLEProvClass::bleConfigDone() {
  return m_provConfigDone;
}

BLEProvClass::~BLEProvClass() {}

BLEProvClass BLEProv;
#endif
