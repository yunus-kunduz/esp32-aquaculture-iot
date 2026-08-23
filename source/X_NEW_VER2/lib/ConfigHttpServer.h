/* 
 Copyright (c) 2019-2021 Sinric Pro

 The copyright in these pages (including without limitation all text, graphics and computer code relating thereto or associated therewith) 
 and all other intellectual property and proprietary rights therein belongs to Sinric Pro and all rights are reserved.

 Permission is given for the downloading and temporary storage of one or more of these pages for the sole purpose of viewing them on a stand-alone personal computer or monitor. 
 Permanent copying or redistribution, or reproduction of all or part of this library in any form is strictly prohibited without express permission of Sinric Pro.
*/

#pragma once

#include <ArduinoJson.h>
#include <DNSServer.h>
#include "APIService.h"
#include "ProvBase64.h"
#include "ProvUtil.h"
#include "ProvStrings.h"
#include "index.html.gz.h"
#include "ProvDebug.h"
#include "ProvState.h"


#ifdef ESP32 
  #include <WebServer.h>  
  #include "SPIFFS.h"
  #include "CryptoMbedTLS.h"
#else 
  #include "CryptoBearSSL.h"
  #include <ESP8266WebServer.h>  
  #include <FS.h>
  extern "C" {
    #include "user_interface.h"
  }
#endif

class ConfigHttpServer {
  public:
    ConfigHttpServer(ConfigStore &configStore);
    ~ConfigHttpServer();

    bool startSoftAP();
    bool restart();
    bool start();
    bool stop();
    void handle();

    using AuthCredentialsCallback = std::function<bool(const String)>;
    void onAuthCredentials(AuthCredentialsCallback cb); 
    
  private:
    void onConfigure();
    void onIndex();
    
    void handleWiFiConfigure();
    void handleCreateAccount();
    void handleProductConfigure();
    void handleProductInfo();
    void handleRooms();
    void handleAuthentication();
    void handleWiFiList();
    void handleNotFound();
    void handleAutoConfigure();
    void handleSmartApConfigure();
    void handleWiFiStatus();
    void copyServerError(const DynamicJsonDocument &doc, String &res);
    void extractServerError(String &body, String &res);
    
    String decodeStr(String input, String key);    
    void decryptBuf(char *buf, const char *input, const char *key, int input_len, int key_len);

    ConfigStore &m_configStore;
    String m_hostName;    

    #ifdef ESP32
      WebServer m_webServer; 
    #else
      ESP8266WebServer m_webServer;      
    #endif

     
    DNSServer *m_dnsServer;    
    APIService m_apiService; 

    bool m_isConfigured = false;
    
    AuthCredentialsCallback m_authCredentialsCallback;
};

ConfigHttpServer::ConfigHttpServer(ConfigStore &_configStore) : m_configStore(_configStore), 
                                                                m_hostName(AP_HOSTNAME), 
                                                                m_webServer(HTTP_SERVER_PORT),
                                                                m_dnsServer(nullptr) { 
}

ConfigHttpServer::~ConfigHttpServer(){ }

/**
 * @brief Callback for authentication credentails eg: appkey/secret.
 */
void ConfigHttpServer::onAuthCredentials(AuthCredentialsCallback cb) {
  m_authCredentialsCallback = cb;
}

/**
 * @brief Gets called when index requested from the browser.
 */
void ConfigHttpServer::onIndex() {
  m_webServer.sendHeader("Content-Encoding", "gzip");      
  m_webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  m_webServer.send(200, PSTR("text/html"), "");
  m_webServer.sendContent_P(index_html_gz, index_html_gz_len); 
}

/**
 * @brief Start HTTP Server
 * @return 
 *    bool start successful
 */
bool ConfigHttpServer::start() {
  DEBUG_PROV(PSTR("[ConfigHttpServer.start()]: Starting HTTP server.\r\n"));
 
  // Webpage handles 
  m_webServer.on("/", std::bind(&ConfigHttpServer::onIndex, this));   
  m_webServer.on("/index.html", std::bind(&ConfigHttpServer::onIndex, this));
  
  // API endpoints
    
  // SmartConfig
  m_webServer.on(F("/api/product/configuration"), std::bind(&ConfigHttpServer::onConfigure, this));
  m_webServer.on(F("/api/product/info"), std::bind(&ConfigHttpServer::handleProductInfo, this));
  
  // AP
  m_webServer.on(F("/wifi/list"), std::bind(&ConfigHttpServer::handleWiFiList, this));
  m_webServer.on(F("/wifi/configure"), std::bind(&ConfigHttpServer::handleWiFiConfigure, this));
  m_webServer.on(F("/api/auth"), std::bind(&ConfigHttpServer::handleAuthentication, this));
  m_webServer.on(F("/api/info"), std::bind(&ConfigHttpServer::handleProductInfo, this));
  m_webServer.on(F("/api/configure"), std::bind(&ConfigHttpServer::handleProductConfigure, this));
  m_webServer.on(F("/api/rooms"), std::bind(&ConfigHttpServer::handleRooms, this));
  m_webServer.on(F("/api/createaccount"), std::bind(&ConfigHttpServer::handleCreateAccount, this));
  m_webServer.on(F("/api/autoconfigure"), std::bind(&ConfigHttpServer::handleAutoConfigure, this));
  m_webServer.on(F("/api/smartapconfigure"), std::bind(&ConfigHttpServer::handleSmartApConfigure, this));
  m_webServer.on(F("/wifi/status"), std::bind(&ConfigHttpServer::handleWiFiStatus, this));
  
  if(PRIMARY_PROV_MODE != PROV_MODE_SMART_AP) { // Do not open Captive UI automatically for Smart AP
    m_webServer.onNotFound(std::bind(&ConfigHttpServer::handleNotFound, this)); 
  }
        
  m_webServer.begin();
  DEBUG_PROV(PSTR("[ConfigHttpServer.start()]: HTTP config-server started!\r\n")); 
  
  DEBUG_PROV(PSTR("[ConfigHttpServer.start()]: Available heap: %d\n"), ESP.getFreeHeap());  
  return true;
}

/**
 * @brief Handle get when unavailble resouces requested
 */ 
void ConfigHttpServer::handleNotFound() {
  String message = "404 - URI: " + m_webServer.uri();  
  DEBUG_PROV(PSTR("[ConfigHttpServer.handleNotFound()] %s\r\n"), message.c_str());

  m_webServer.sendHeader(kAccessControlAllowOrigin, kAnyIP);
  m_webServer.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
  m_webServer.send(302, "text/plain", "");
  m_webServer.client().stop();

    // If does not work, serve the file from SPIFFS
//  File file = SPIFFS.open("/prov/index.htm", "r");
//  size_t sent = m_webServer.streamFile(file, "text/html");
//  file.close(); 
}

/**
 * @brief Stop HTTP config server and DNS redirects
 * @return
 *      ok
 */
bool ConfigHttpServer::stop(){
  DEBUG_PROV(PSTR("[ConfigHttpServer.stop()] Stopping ..."));
  
  m_webServer.stop(); 

  if(m_dnsServer) {
    m_dnsServer->stop();
    delete m_dnsServer;
    m_dnsServer = nullptr;
  }
  
  DEBUG_PROV("success!\r\n"); 
  return true;
}

/**
 * @brief Handle get all the available WiFi list API call
 * @return 
 *    String List of visible WiFi networks.
 */
void ConfigHttpServer::handleWiFiList() {
  DEBUG_PROV(PSTR("[ConfigHttpServer.handleWiFiList()] Scanning WiFi networks..\r\n"));
  
  int total = WiFi.scanNetworks();
  String response = "[";
  for (int i = 0; i < total; ++i) {
      if(i != 0) response += ",";
      response += "{\"ssid\":\"" + WiFi.SSID(i) + "\",";
      response += "\"rssi\":" + String(WiFi.RSSI(i)) + "}";
  }
  response += "]";

  DEBUG_PROV(PSTR("[ConfigHttpServer.handleWiFiList()] WiFi list: %s\r\n"), response.c_str());

  // Free memory
  WiFi.scanDelete();
  
  m_webServer.sendHeader(kAccessControlAllowOrigin, kAnyIP);
  m_webServer.send(HTTP_CODE_OK, kJson, response); 
} 

/**
 * @brief Handles authentication request from WebUI. 
 * If the user is authenticated, store the accessToken in memory for further API calls
 */
void ConfigHttpServer::handleAuthentication() {
  String res("");
  
  if(m_webServer.hasArg("token")) {
    String token = m_webServer.arg("token");
    token.trim();
    bool valid = token.length() > 0;
  
    if(!valid) {
      m_webServer.sendHeader(kAccessControlAllowOrigin, kAnyIP);
      m_webServer.send(401, "Unauthorized");      
      return;
    }  
    
    DEBUG_PROV(PSTR("[ConfigHttpServer.handleAuthentication()] Authenticate with token:%s ..\r\n"), token.c_str());
    HTTPResponse httpResponse = m_apiService.authenticate(token.c_str());
        
    if(httpResponse.code == HTTP_CODE_OK) {
      DynamicJsonDocument doc(512);
      DeserializationError error = deserializeJson(doc, httpResponse.body);
         
      if (error) {
        DEBUG_PROV(PSTR("[ConfigHttpServer.handleAuthentication()] Json decode error: %s\r\n"), error.c_str());
        DEBUG_PROV(PSTR("[ConfigHttpServer.handleAuthentication()] json :%s\n"), httpResponse.body.c_str());
        res = FPSTR(str_got_invalid_json_response);  
      } else {
        bool success = doc[F("success")].as<bool>(); // true
  
        if(success) { 
          const char* accessToken = doc[F("accessToken")];
          DEBUG_PROV(PSTR("[ConfigHttpServer.handleAuthentication()] Access token: %s\r\n"), accessToken);          
          m_apiService.setAccessToken(accessToken);          
          res = FPSTR(str_success);
        } else {          
          copyServerError(doc, res);
        }
      }
    } else {
      if(httpResponse.code == 403) {
        res = FPSTR(str_incorrect_username_or_password);
      }
      else {
        res = F("{\"success\":false, \"message\":\"");
        res += httpResponse.body;
        res += F("\"}");
      }
    }    
  } else {
    res = FPSTR(str_invalid_login_token); 
  }

  DEBUG_PROV(PSTR("[ConfigHttpServer.handleAuthentication()]: response: %s..\r\n"), res.c_str());  
  
  m_webServer.sendHeader(kAccessControlAllowOrigin, kAnyIP);
  m_webServer.send(HTTP_CODE_OK, kJson, res.c_str());      
}

/**
 * @brief Handles get list of rooms API call from captive portal. 
 */
void ConfigHttpServer::handleRooms() {
  DEBUG_PROV(PSTR("[ConfigHttpServer.handleRooms()]: Get rooms ..\r\n"));
  HTTPResponse httpResponse = m_apiService.getRooms();
  String res("");
 
  if(httpResponse.code == HTTP_CODE_OK) { 
    res = httpResponse.body;
  } else {
    res = F("{\"success\":false, \"message\":\"");
    res += httpResponse.body;
    res += F("\"}");
  }
 
  DEBUG_PROV(PSTR("[ConfigHttpServer.handleRooms()]: response: %s..\r\n"), res.c_str());  
  m_webServer.sendHeader(kAccessControlAllowOrigin, kAnyIP);
  m_webServer.send(HTTP_CODE_OK, kJson, res.c_str());    
}

/**
 * @brief Handles get product information API call from captive portal. 
 */
void ConfigHttpServer::handleProductInfo() {
  String res = getProductStructure();
  DEBUG_PROV(PSTR("[ConfigHttpServer.handleProductInfo()]: Get product info: %s\n"), res.c_str());  
  m_webServer.sendHeader(kAccessControlAllowOrigin, kAnyIP);
  m_webServer.send(HTTP_CODE_OK, kJson, res.c_str());
}

/**
 * @brief Handles product configuration API call from captive portal. 
 */
void ConfigHttpServer::handleProductConfigure() {
  String devices = m_webServer.arg("devices");
  int deviceCount = m_webServer.arg("deviceCount").toInt();
  String macAddress = ProvUtil::getMacAddress();  
  String ip = WiFi.localIP().toString();
  String res;
  String deviceIds[deviceCount];
    
  const char* accessKeyId = nullptr;
  const char* appKey = nullptr;
  const char* appSecret = nullptr;

  const char fmt[] = "devices[%u][%s]";
  char dataBuf[100] = "";
  
  DEBUG_PROV(PSTR("[ConfigHttpServer.handleProductConfigure()] Delete existing devices with MAC address: %s..\r\n"), macAddress.c_str());
  m_apiService.deleteDevicesByMacAddress(macAddress.c_str());  

  DEBUG_PROV(PSTR("[ConfigHttpServer.handleProductConfigure()] Creating %u new devices ..\r\n"), deviceCount);
  DynamicJsonDocument doc(2048);
  
  for (int i = 0; i < deviceCount; i++) {
      sprintf_P(dataBuf, fmt, i, "name");
      String deviceName = m_webServer.arg(dataBuf);

      sprintf_P(dataBuf, fmt, i, "description");
      String deviceDescription = m_webServer.arg(dataBuf);

      sprintf_P(dataBuf, fmt, i, "roomId");
      String deviceRoomId = m_webServer.arg(dataBuf); 

      sprintf_P(dataBuf, fmt, i, "productCode");
      String productCode = m_webServer.arg(dataBuf); 
      
      DEBUG_PROV(PSTR("[ConfigHttpServer.handleProductConfigure()] Creating device with name: %s, desc: %s, roomId: %s, accessKeyId: %s, ip: %s, mac: %s\r\n"), deviceName.c_str(), deviceDescription.c_str(), deviceRoomId.c_str(), (accessKeyId == nullptr ? "" : accessKeyId) , ip.c_str(), macAddress.c_str());
      HTTPResponse httpResponse = m_apiService.createDevice(deviceName, deviceDescription, ip, macAddress, productCode, deviceRoomId, accessKeyId);
            
      if(httpResponse.code == HTTP_CODE_OK) {
          DeserializationError error = deserializeJson(doc, httpResponse.body);
          
          if (error) {
            DEBUG_PROV(PSTR("[ConfigHttpServer.handleProductConfigure()] decode error: %s\r\n"), error.c_str());       
            DEBUG_PROV(PSTR("[ConfigHttpServer.handleProductConfigure()] json :%s\n"), httpResponse.body.c_str());
            res = FPSTR(str_got_invalid_http_response_code);  
            break;
          } else {
            bool success = doc[F("success")].as<bool>(); // true
      
            if(success) { 
              if(accessKeyId == nullptr) {
                accessKeyId = doc[F("device")][F("credential")][F("id")]; // To bind the next device to this access key id
                appKey = doc[F("device")][F("credential")][F("appkey")];
                appSecret = doc[F("device")][F("credential")][F("appsecert")];
              }
              
              String deviceId = doc[F("device")][F("id")].as<String>();
              deviceIds[i] = deviceId;
            } else {
              copyServerError(doc, res);
              break;
            }
          }
      } 
      else if(httpResponse.code == 422 || httpResponse.code == 500) {
        extractServerError(httpResponse.body, res);
        break;
      }
      else {
        res = F("{\"success\":false, \"message\":\"");
        res += httpResponse.body;
        res += F("\"}");
        break;
      }
  } 

  doc.clear(); // Clear document

  bool createSuccess = (res.length() <= 0); // no errors  
  
  if(createSuccess) {
    ProvState::getInstance()->state = ProvState::MODE_PROV_DONE;
    res = FPSTR(str_success);
  }
    
  m_webServer.sendHeader(kAccessControlAllowOrigin, kAnyIP);
  m_webServer.send(HTTP_CODE_OK, kJson, res);
  yield();

  if(createSuccess && m_authCredentialsCallback) {
    DEBUG_PROV(PSTR("[ConfigHttpServer.handleProductConfigure()] AppKey: %s\r\n"), appKey);
    DEBUG_PROV(PSTR("[ConfigHttpServer.handleProductConfigure()] AppSecret: %s\r\n"), appSecret);
    
    int size = sizeof(deviceIds)/sizeof(deviceIds[0]);
    DEBUG_PROV(PSTR("[ConfigHttpServer.handleProductConfigure()] No of devices: %d.\n"), size);

    DynamicJsonDocument doc(2048);
    JsonObject credentials = doc.createNestedObject(F("credentials"));
    credentials[F("appkey")] = appKey;
    credentials[F("appsecret")] = appSecret;
    
    JsonArray devices = doc.createNestedArray(F("devices")); // include id and credential in the response.
    for (int i = 0; i < size ; ++i) {
      JsonObject device = devices.createNestedObject();
      device[F("id")] = deviceIds[i].c_str();      
      DEBUG_PROV(PSTR("[ConfigHttpServer.handleProductConfigure()] Device id: %s.\n"), deviceIds[i].c_str());
    }    

    String body;
    serializeJson(doc, body); 
    m_authCredentialsCallback(body);

    m_apiService.clearAccessToken();

    DEBUG_PROV(PSTR("[ConfigHttpServer.handleProductConfigure()] Configuration finished!\n"));
    m_isConfigured = true;
  }
}

/**
 * @brief Handles create account API call from captive portal. 
 */
void ConfigHttpServer::handleCreateAccount() {
  DEBUG_PROV(PSTR("[ConfigHttpServer.handleCreateAccount()]: Creating account..\r\n"));
  
  String name = m_webServer.arg("name");
  String email = m_webServer.arg("email");
  String password = m_webServer.arg("password");
  String manufacturerId = m_webServer.arg("manufacturerId");
  String temperatureScale = m_webServer.arg("temperatureScale");
  String language = m_webServer.arg("language");
  String recaptcha = m_webServer.arg("recaptcha");
  String clientId = m_webServer.arg("clientId");
  String timeZone = m_webServer.arg("timeZone");
  
  HTTPResponse httpResponse = m_apiService.createAccount(name, email, password, manufacturerId, temperatureScale, language, recaptcha, clientId, timeZone);
  
  String res = "";  
  if(httpResponse.code == HTTP_CODE_OK) {
      res = FPSTR(str_success);
  }
  else if(httpResponse.code == 422 || httpResponse.code == 500) {
    extractServerError(httpResponse.body, res);
  }
  else {
    res = F("{\"success\":false, \"message\":\"");
    res += httpResponse.body;
    res += F("\"}");
  }

  DEBUG_PROV(PSTR("[ConfigHttpServer.handleCreateAccount()]: response: %s..\r\n"), res.c_str());  
  m_webServer.sendHeader(kAccessControlAllowOrigin, kAnyIP);
  m_webServer.send(HTTP_CODE_OK, kJson, res);
}
 

/**
 * @brief Handles device auto configure API call from captive portal. 
 */
void ConfigHttpServer::handleAutoConfigure() {
  if (!m_webServer.hasArg("email")) {
    return m_webServer.send(HTTP_CODE_OK, kJson, str_invalid_email);
  }
 
  String email = m_webServer.arg("email"); 
  String macAddress = ProvUtil::getMacAddress();  
  String secret = String(PROV_HTTP_REQUEST_SECRET);
  String manufacturerId = m_webServer.arg("manufacturerId"); 
  String password = m_webServer.arg("password"); 
  String productCode = String(PRODUCT_CODE);

  String payload("");
  payload.concat(email);
  payload.concat(macAddress);
  payload.concat(productCode);
  payload.concat(manufacturerId);
  payload.concat(password);
    
  String sign = ProvUtil::sign(payload, secret);
  
  DEBUG_PROV(PSTR("[ConfigHttpServer.handleAutoConfigure()]: email: %s, productCode: %s, macAddress: %s, manufacturerId: %s, password: %s, sign: %s\r\n"), 
    email.c_str(), productCode.c_str(), macAddress.c_str(), manufacturerId.c_str(), password.c_str(), sign.c_str());
  
  HTTPResponse httpResponse = m_apiService.autoConfigure(email, productCode, macAddress, manufacturerId, password, sign);
 
  String res = "";  
  
  if(httpResponse.code == HTTP_CODE_OK) {
      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, httpResponse.body);
      
      if (error) {
        DEBUG_PROV(PSTR("[ConfigHttpServer.handleAutoConfigure()] decode error: %s\r\n"), error.c_str());       
        DEBUG_PROV(PSTR("[ConfigHttpServer.handleAutoConfigure()] json :%s\n"), httpResponse.body.c_str());
        res = FPSTR(str_got_invalid_http_response_code);
      } else {
        bool success = doc[F("success")].as<bool>(); // true
  
        if(success) { 
          // Send the response to client.
          res = FPSTR(str_success);
          m_webServer.sendHeader(kAccessControlAllowOrigin, kAnyIP);
          m_webServer.send(HTTP_CODE_OK, kJson, res);

          if(doc.containsKey("success")) {
            doc.remove("success");
          }

          if(doc.containsKey("message")) {
            doc.remove("message");
          }
         
          // Save the config 
          String body;
          serializeJson(doc, body); 
          m_authCredentialsCallback(body);          

          DEBUG_PROV(PSTR("[ConfigHttpServer.handleAutoConfigure()] Configuration finished!\n"));
          if(success) ProvState::getInstance()->state = ProvState::MODE_PROV_DONE;
          m_isConfigured = true;
        } else {
          copyServerError(doc, res);
        }
      }
  }
  else if(httpResponse.code == 422 || httpResponse.code == 500) {
    extractServerError(httpResponse.body, res);
  }
  else {
    res = F("{\"success\":false, \"message\":\"");
    res += httpResponse.body;
    res += F("\"}");
  } 

  DEBUG_PROV(PSTR("[ConfigHttpServer.handleAutoConfigure()]: response: %s..\r\n"), res.c_str());  
  m_webServer.sendHeader(kAccessControlAllowOrigin, kAnyIP);
  m_webServer.send(HTTP_CODE_OK, kJson, res);
}

void ConfigHttpServer::copyServerError(const DynamicJsonDocument &doc, String &res) { 
    String msg = doc[F("message")].as<String>(); 
    msg.replace("\"", "'");
    res = F("{\"success\":false, \"message\":\"");
    res += msg;
    res += F("\"}");
}

void ConfigHttpServer::extractServerError(String &body, String &res) { 
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, body);
    if (error) {
        DEBUG_PROV(PSTR("[ConfigHttpServer.extractServerError()] decode error: %s\r\n"), error.c_str());       
        DEBUG_PROV(PSTR("[ConfigHttpServer.extractServerError()] json :%s\n"), body.c_str());
        res = FPSTR(str_got_invalid_http_response_code);
    } else {
      copyServerError(doc, res);
    }    
}



/**
 * @brief Handles WiFi state API call from app. 
 */
void ConfigHttpServer::handleWiFiStatus() {
  StaticJsonDocument<256> doc;
  doc["isConnected"] = WiFi.isConnected();
  doc["ssid"] = WiFi.isConnected() ? WiFi.SSID() : "";
  doc["localIP"] = WiFi.isConnected() ? WiFi.localIP().toString() : "";

  String res;
  serializeJson(doc, res); 

  m_webServer.sendHeader(kAccessControlAllowOrigin, kAnyIP);
  m_webServer.send(HTTP_CODE_OK, kJson, res);   
}

/**
 * @brief Handles WiFi configure API call from captive portal. 
 */
void ConfigHttpServer::handleWiFiConfigure() {
  if (!m_webServer.hasArg("ssid")) {
    return m_webServer.send(HTTP_CODE_OK, kJson, str_invalid_ssid);
  }

  if (!m_webServer.hasArg("pass")) {
    return m_webServer.send(HTTP_CODE_OK, kJson, str_invalid_password);
  }

  String ssid = m_webServer.arg("ssid");
  String password = m_webServer.arg("pass");   

  DEBUG_PROV(PSTR("[ConfigHttpServer.handleConfigure()]: Got SSID:%s, PWD: %s\n"), ssid.c_str(), password.c_str());
  
    
  if(WiFi.isConnected()) {
    if(WiFi.SSID() == ssid) {
      DEBUG_PROV(PSTR("[ConfigHttpServer.handleConfigure()]: Already connected to same WiFi...\n"));
      m_webServer.sendHeader(kAccessControlAllowOrigin, kAnyIP);
      m_webServer.send(HTTP_CODE_OK, kJson, str_success);
      return;    
    }
    else {
      DEBUG_PROV(PSTR("[ConfigHttpServer.handleConfigure()]: Already connected to WiFi... disconnecting..\n"));
      WiFi.disconnect();
    }  
  }

  ProvState::getInstance()->state = ProvState::MODE_CONNECTING_WIFI;
  
  bool success = ProvUtil::connectToWiFi(ssid.c_str(), password.c_str());

  if(success) {
     ProvState::getInstance()->state = ProvState::MODE_WIFI_CONNECTED_WAIT_APP_CREDENTIALS; 
  } else {
     ProvState::getInstance()->state = ProvState::MODE_WAIT_WIFI_CONFIG;
  }

  delay(500);
  
  m_webServer.sendHeader(kAccessControlAllowOrigin, kAnyIP);
  if (success) {
      m_webServer.send(HTTP_CODE_OK, kJson, str_success);
  } else {
      m_webServer.send(HTTP_CODE_OK, kJson, str_failed_to_connect);
  }
}

/**
* @brief Handle Http server requests.
*/ 
void ConfigHttpServer::handle() {
  DEBUG_PROV(PSTR("[ConfigHttpServer.handle()]: Handle incoming http requests ..\r\n"));
   
  while (!m_isConfigured) {
    if(m_dnsServer) m_dnsServer->processNextRequest();
    m_webServer.handleClient();
    yield();
  }

  DEBUG_PROV(PSTR("[ConfigHttpServer.handle()] Wait 2 seconds untill the client the response before we wrap up.\r\n"));
  unsigned long time_now = millis();
  while(millis() < time_now + 2000){
    m_webServer.handleClient();
    yield();
  }  

  DEBUG_PROV(PSTR("[ConfigHttpServer.handle()]: Exit ..\r\n"));
}

 

/**
* @brief Handle configuration data from the app.
*/ 
void ConfigHttpServer::onConfigure() {
  DEBUG_PROV(PSTR("[ConfigHttpServer.onConfigure()]: Receiving configuration\r\n"));

  if (m_webServer.hasArg("configuration"))
  {
    String configuration = m_webServer.arg("configuration");
    String key = String(ProvUtil::getChipId32(), HEX);
    String decoded = decodeStr(configuration, key);

    DynamicJsonDocument jsonConfig(2048);
    deserializeJson(jsonConfig, decoded);

    DEBUG_PROV(PSTR("[ConfigHttpServer.onConfigure()]: %s\r\n"), decoded.c_str());
    
    DEBUG_PROV(PSTR("[ConfigHttpServer.onConfigure()]: Remove unnecessory elements!\r\n"));
    for (JsonObject elem : jsonConfig["devices"].as<JsonArray>()) {
      if (elem["name"]) elem.remove("name");
      if (elem["code"]) elem.remove("code");
    }

    m_webServer.sendHeader(kAccessControlAllowOrigin, kAnyIP);
    
    if (m_configStore.saveJsonConfig(jsonConfig)) {
      m_webServer.send(HTTP_CODE_OK, kJson, str_success);      
      DEBUG_PROV(PSTR("[ConfigHttpServer.onConfigure()]: Configuration updated!\r\n"));

      ProvState::getInstance()->state = ProvState::MODE_PROV_DONE;
      
      // Wait until client gets the response before we wrap up.
      for(int i = 0; i < 3; i++) { delay(500); }      
      m_isConfigured = true;
    }
    else
    {
      DEBUG_PROV(PSTR("[ConfigHttpServer.onConfigure()]: Configuration failed!\r\n"));
      m_webServer.send(HTTP_CODE_OK, kJson, str_failed_to_save_configuration);
      yield();
      delay(1000);
      ESP.restart();
    }
  }
}


/**
 * @brief Start the SoftAP in WIFI_AP_STA mode.
 * @return bool Success or Failed
 */
bool ConfigHttpServer::startSoftAP() {
  DEBUG_PROV(PSTR("[ConfigHttpServer.startSoftAP()]: Setup SoftAP ..\r\n"));
  
  WiFi.disconnect();
  delay(1000); 
   
  WiFi.mode(WIFI_AP_STA);
  delay(100);

  #ifdef ESP8266 
    DEBUG_PROV(PSTR("[ConfigHttpServer.startSoftAP()]: Set WiFi sleep mode to non-sleep..\r\n"));
    WiFi.setSleepMode(WIFI_NONE_SLEEP); //https://arduino.stackexchange.com/questions/39957/esp8266-udp-multicast-doesnt-receive-packets
  #endif
  

  while (WiFi.getMode() != WIFI_AP_STA) {
    delay(50);
  }
    
  DEBUG_PROV(PSTR("[ConfigHttpServer.startSoftAP()]: Configuring softAP... "));

  // Setup static IP
  IPAddress apIP(STATIC_IP);
  IPAddress gateway(GATEWAY);
  IPAddress subnet(SUBNET);
  boolean result = WiFi.softAPConfig(apIP,gateway,subnet);
  Serial.println(result);

  // Set the AP name
  m_hostName += String(ProvUtil::getChipId32(), HEX);

  DEBUG_PROV(PSTR("[ConfigHttpServer.startSoftAP()]: Starting softAP with hostname: %s...\r\n"), m_hostName.c_str());  
  WiFi.hostname(m_hostName);
       
  result = WiFi.softAP(m_hostName.c_str());
  delay(500); // Wait 500 ms for AP_START...
 
  if(!result) { 
     DEBUG_PROV(PSTR("[ConfigHttpServer.startSoftAP()]: Starting AP failed.!\r\n"));
     return false; 
  }

  IPAddress myIP = WiFi.softAPIP();
  DEBUG_PROV(PSTR("[ConfigHttpServer.startSoftAP()]: SoftAP IP address: %s \r\n"), myIP.toString().c_str());
  

  /* Setup the DNS server redirecting all the domains to the apIP */
  DEBUG_PROV(PSTR("[ConfigHttpServer.startSoftAP()]: Setup DNS\r\n"));
  m_dnsServer = new DNSServer();
  m_dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  m_dnsServer->start(53, "*", myIP);
  
  DEBUG_PROV(PSTR("[ConfigHttpServer.startSoftAP()]: DNS Server started\r\n"));
  return true; 
}


/**
 * @brief Base64 decode and decrypt the input using key
 * @param input 
 * @param key 
 * @return String 
 */
String ConfigHttpServer::decodeStr(String input, String key)
{
  size_t input_len = input.length();
  size_t key_len = key.length();

  size_t dec_len = base64_dec_len((char *)input.c_str(), input_len);
  char *buf = new char[input_len];
  memset(buf, 0, dec_len);
  base64_decode(buf, (char *)input.c_str(), input_len);

  char *decrypted_buf = new char[dec_len + 1];
  memset(decrypted_buf, 0, dec_len);
  decryptBuf(decrypted_buf, buf, key.c_str(), dec_len, key_len);
  decrypted_buf[dec_len] = '\0';

  String result(decrypted_buf);

  delete[] decrypted_buf;
  delete[] buf;

  return result;
}


/**
 * @brief Decrypt the buffer
 */
void ConfigHttpServer::decryptBuf(char *buf, const char *input, const char *key, int input_len, int key_len)
{
  for (int i = 0; i < input_len; i++)
  {
    buf[i] = input[i] ^ key[i % key_len];
  }
}


/**
 * @brief configure device from mobile-app
 */
void ConfigHttpServer::handleSmartApConfigure() {
  if (!m_webServer.hasArg("configuration")) {
    return m_webServer.send(HTTP_CODE_OK, kJson, str_invalid_email);
  }

  String configuration = m_webServer.arg("configuration");
  String key = String(ProvUtil::getChipId32(), HEX);
  String decoded = decodeStr(configuration, key);

  DEBUG_PROV(PSTR("[ConfigHttpServer.handleSmartApConfigure()]: decoded: %s\r\n"), decoded.c_str());

  DynamicJsonDocument jsonConfig(2048);
  deserializeJson(jsonConfig, decoded);

  String appKey = ProvUtil::randomString(8) + "-" + ProvUtil::randomString(4) + "-" + ProvUtil::randomString(4) + "-" + ProvUtil::randomString(4) + "-" + ProvUtil::randomString(12);
  String appSecret = ProvUtil::randomString(8) + "-" + ProvUtil::randomString(4) + "-" + ProvUtil::randomString(4) + "-" + ProvUtil::randomString(4) + "-" + ProvUtil::randomString(12) + "-" + ProvUtil::randomString(8) + "-" + ProvUtil::randomString(4) + "-" + ProvUtil::randomString(4) + "-" + ProvUtil::randomString(4) + "-" + ProvUtil::randomString(12);
 
  jsonConfig[F("appKey")] = appKey;
  jsonConfig[F("appSecret")] = appSecret;
  jsonConfig[F("productCode")] = String(PRODUCT_CODE);
  jsonConfig[F("macAddress")] = ProvUtil::getMacAddress();
    
  String payload;
  serializeJson(jsonConfig, payload);  

  DEBUG_PROV(PSTR("[ConfigHttpServer.handleSmartApConfigure()]: payload: %s\r\n"), payload.c_str());

  String secret = String(PROV_HTTP_REQUEST_SECRET);

  #ifdef ESP32 
    String encrypedConfiguration = CryptoMbedTLS::aes128cbcEncrypt(payload);
  #else 
    String encrypedConfiguration = CryptoBearSSL::aes128cbcEncrypt(payload);  
  #endif

  
  String sign = ProvUtil::sign(encrypedConfiguration, secret);
    
  DEBUG_PROV(PSTR("[ConfigHttpServer.handleSmartApConfigure()]: encrypedConfiguration: %s, sign: %s \r\n"), encrypedConfiguration.c_str(), sign.c_str());
  
  HTTPResponse httpResponse = m_apiService.smartApConfigure(encrypedConfiguration, sign);

  if(httpResponse.code == HTTP_CODE_OK) {
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, httpResponse.body);
    String res = "";

    if (error) {
      DEBUG_PROV(PSTR("[ConfigHttpServer.handleSmartApConfigure()] Json decode error: %s\r\n"), error.c_str());
      DEBUG_PROV(PSTR("[ConfigHttpServer.handleSmartApConfigure()] json :%s\n"), httpResponse.body.c_str());
      res = FPSTR(str_got_invalid_json_response);  
    } else {
      bool success = doc[F("success")].as<bool>(); // true

      if(success) { 
        DynamicJsonDocument deviceConfig(2048);
        JsonObject credentials = deviceConfig.createNestedObject(F("credentials"));
        credentials[F("appkey")] = appKey;
        credentials[F("appsecret")] = appSecret;
        
        JsonArray createdDevices = jsonConfig["devices"].as<JsonArray>();
        JsonArray devices = deviceConfig.createNestedArray(F("devices"));

        for (JsonObject createdDevice : createdDevices) {
          JsonObject device = devices.createNestedObject();
          device[F("id")] = createdDevice["id"];      
        }    

        String body;
        serializeJson(deviceConfig, body); 

        DEBUG_PROV(PSTR("[ConfigHttpServer.handleSmartApConfigure()]: config: %s\r\n"), body.c_str());
        DEBUG_PROV(PSTR("[ConfigHttpServer.handleSmartApConfigure()] Configuration finished!\n"));

        res = FPSTR(str_success);  
        m_authCredentialsCallback(body);
        m_isConfigured = true;
        ProvState::getInstance()->state = ProvState::MODE_PROV_DONE;
      } else {          
        copyServerError(doc, res);
      }
    }
    
    m_webServer.sendHeader(kAccessControlAllowOrigin, kAnyIP);
    m_webServer.send(HTTP_CODE_OK, kJson, res);
    
  } else {
    String res = F("{\"success\":false, \"message\":\"");
    res += httpResponse.body;
    res += F("\"}");

    m_webServer.sendHeader(kAccessControlAllowOrigin, kAnyIP);
    m_webServer.send(HTTP_CODE_OK, kJson, res);
  }
}