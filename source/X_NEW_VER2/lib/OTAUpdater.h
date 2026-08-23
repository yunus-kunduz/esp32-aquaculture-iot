/* 
 Copyright (c) 2019-2021 Sinric Pro

 The copyright in these pages (including without limitation all text, graphics and computer code relating thereto or associated therewith) 
 and all other intellectual property and proprietary rights therein belongs to Sinric Pro and all rights are reserved.

 Permission is given for the downloading and temporary storage of one or more of these pages for the sole purpose of viewing them on a stand-alone personal computer or monitor. 
 Permanent copying or redistribution, or reproduction of all or part of this library in any form is strictly prohibited without express permission of Sinric Pro.
*/

#pragma once 

#include <ArduinoJson.h>
#include <SinricProConfig.h>
#include <time.h>
#include "ProvUtil.h" 
#include "ProvDebug.h" 

#ifdef ESP8266 
  #include <ESP8266WiFi.h>
  #include <ESP8266httpUpdate.h>
  #include <ESP8266HTTPClient.h>
#endif 
#ifdef ESP32   
  #include <WiFi.h>
  #include <HTTPClient.h>
  #include <HTTPUpdate.h>
#endif  
  
class OTAUpdater {
  public:
    OTAUpdater();
    ~OTAUpdater();
    void setup();
    void handleCheckAndUpdate();

  private:        
    unsigned long m_lastOTACheckMillis; 
    String getOtaChallengeUrl();
    void getOtaUpdateUrl(String &url, const String &answer);
    void update(const String &token); 
    bool authenticate(String &token);
};

/**
* @brief Authenticate (CRAM) and get a access token
* @param token Reference to get the authentication token
*/ 
bool OTAUpdater::authenticate(String &token) {
  bool success = false;

  // setup WiFiClient client
  #ifndef SECURE_OTA
    WiFiClient *client = new WiFiClient;
  #else        
    #if defined(ESP8266) 
      BearSSL::WiFiClientSecure *client = new BearSSL::WiFiClientSecure;
    
      if(client) { 
        bool mfln = client->probeMaxFragmentLength(OTA_HOST, 443, 1024);
        if (mfln) {
          client->setBufferSizes(1024, 1024);
        } else {
          DEBUG_PROV(PSTR("[OTAUpdater.authenticate()]: Warning: probeMaxFragmentLength not supported!\n"));
        }        
      }      
    #elif defined(ESP32) 
      WiFiClientSecure *client = new WiFiClientSecure;   
      client->setInsecure(); //skip verification
    #endif
  #endif 

  if(client) {
    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
      HTTPClient https;
      const char *headerkeys[] = { "Set-Cookie", "Cookie" };
      size_t headerkeyssize = sizeof(headerkeys) / sizeof(char *); 

      // Get OTA challenge URL
      String otaChallengeUrl = getOtaChallengeUrl();
      DEBUG_PROV(PSTR("[OTAUpdater.authenticate()]: Ota challenge url :%s\n"),  otaChallengeUrl.c_str());  
      
      if (https.begin(*client, otaChallengeUrl)) {
        https.setTimeout(OTA_HTTP_TIMEOUT);
        https.setReuse(true);
        https.collectHeaders(headerkeys, headerkeyssize); 
        int httpCode = https.GET();     
           
        if (httpCode == 200) {
          String sessionCookie = https.header("Set-Cookie");
          String challenge = https.getString();
          https.end();

          // Prepre answer to challenge
          String secret = String(OTA_SECRET);
          String payload = "";  
          payload.concat(challenge);
          payload.concat(":");
          payload.concat(secret);  

          String answer = ProvUtil::sign(payload, secret);           
          DynamicJsonDocument doc(512);    
          doc[F("answer")] = answer; 
          String body;
          serializeJson(doc, body); 

          DEBUG_PROV(PSTR("[OTAUpdater.authenticate()]: OTA secret :%s, challenge: %s answer: %s\n"),  secret.c_str(), challenge.c_str(), answer.c_str());
          
          // Send the answer.
          https.begin(*client, otaChallengeUrl);
          https.addHeader("Cookie", sessionCookie);
          https.addHeader("Content-Type", "application/json"); 
          https.setTimeout(OTA_HTTP_TIMEOUT);
          int httpCode = https.POST(body);

          if (httpCode == 200) {
            token = https.getString();
            success = true;
            DEBUG_PROV(PSTR("[OTAUpdater.authenticate()]: Answer success. Got Access token :%s\n"),  token.c_str());            
          }
          else {
            DEBUG_PROV(PSTR("[OTAUpdater.authenticate()]: POST answer failed, error: %s\n"), https.errorToString(httpCode).c_str());
          }
        } else {
          DEBUG_PROV(PSTR("[OTAUpdater.authenticate()]: GET challenge failed, error: %s\n"), https.errorToString(httpCode).c_str());
        }

        https.end();
      } else {
        https.end();
      }

      // End extra scoping block
    }
  
    delete client;
  }

  return success;
}
 
/**
* @brief Get OTA challenge URL. 
*/ 
String OTAUpdater::getOtaChallengeUrl() {  
  static char otaChallengeUrl[512] = {'\0'};
    
  #if defined(SECURE_OTA)
    snprintf_P(otaChallengeUrl, sizeof(otaChallengeUrl), PSTR("https://%s/v2/challenge"), OTA_HOST);
  #else
    snprintf_P(otaChallengeUrl, sizeof(otaChallengeUrl), PSTR("http://%s:%u/v2/challenge"), OTA_HOST, OTA_SERVER_PORT);
  #endif

  return otaChallengeUrl;
} 

/**
* @brief Get OTA update URL. 
* @param token reference to get the OTA URL
* @param token authentication token
*/ 
void OTAUpdater::getOtaUpdateUrl(String &url, const String &token) {  
  String currentFWVersion = String(FIRMWARE_VERSION);
  DEBUG_PROV(PSTR("[OTAUpdater.getOtaUpdateUrl()]: Current FW version: %s\r\n"), currentFWVersion.c_str());    

  String mac = String(WiFi.macAddress());
  String params = String("currVersion=") + ProvUtil::urlencode(currentFWVersion)
          + "&productCode=" + ProvUtil::urlencode(String(PRODUCT_CODE))
          + "&mac=" + ProvUtil::urlencode(mac) 
          + "&token=" + ProvUtil::urlencode(token);  
    
  #ifdef SECURE_OTA   
    url.concat("https://");
  #else
    url.concat("http://"); 
  #endif

  url.concat(String(OTA_HOST));

  #ifndef SECURE_OTA   
    url.concat(":");
    url.concat(String(OTA_SERVER_PORT));
  #endif

  url.concat("/v2/check?");
  url.concat(params);  
}
 
/**
* @brief Authenticate and check if there's a newer version
*/ 
void OTAUpdater::handleCheckAndUpdate() {  
 if (millis() - m_lastOTACheckMillis > OTA_CHECK_INTERVAL) {
    m_lastOTACheckMillis = millis();

    DEBUG_PROV(PSTR("[OTAUpdater.handleCheckAndUpdate()]: Checking for OTA updates ..\r\n"));

    if (WiFi.status() != WL_CONNECTED) {
      DEBUG_PROV(PSTR("[OTAUpdater.handleCheckAndUpdate()] No internet  ..\r\n"));
      return;
    } 

    String token;

    if(authenticate(token)) {
      update(token);
    }
 }
}

/**
* @brief Check and update
*/ 
void OTAUpdater::update(const String &token) {  
  String url = "";
  getOtaUpdateUrl(url, token);
  DEBUG_PROV(PSTR("[OTAUpdater.update()] URL: %s ..\r\n"), url.c_str());
  
  #ifdef SECURE_OTA
    #ifdef ESP32
      WiFiClientSecure client;
      client.setInsecure(); //skip verification
      client.setTimeout(OTA_HTTP_TIMEOUT); // in seconds. because reading data over SSL may be slow.

      #if defined(OTA_LED_PIN)
        httpUpdate.setLedPin(BOARD_LED_PIN, HIGH);
      #endif
      
      t_httpUpdate_return ret = httpUpdate.update(client,url);
    #else
      #error "Not supported!"
    #endif      
  #else 
    WiFiClient client;

    #ifdef ESP32
      #if defined(OTA_LED_PIN)
        ESPhttpUpdate.setLedPin(BOARD_LED_PIN, HIGH);
      #endif

      t_httpUpdate_return ret = httpUpdate.update(client,url);
    #else       
      #if defined(OTA_LED_PIN)
        ESPhttpUpdate.setLedPin(BOARD_LED_PIN, HIGH);
      #endif
      
      t_httpUpdate_return ret = ESPhttpUpdate.update(client, url);
    #endif
  #endif
        
  switch(ret) {
      case HTTP_UPDATE_FAILED:
          #ifdef ESP32   
            DEBUG_PROV(PSTR("[OTAUpdater.update()]: Update failed (%d):%s\r\n"), httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
          #else  
            DEBUG_PROV(PSTR("[OTAUpdater.update()]: Update failed (%d): %s\r\n"), ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          #endif
          break;
      case HTTP_UPDATE_NO_UPDATES:
          DEBUG_PROV(PSTR("[OTAUpdater.update()]: Update no Update.\r\n"));
          break;
      case HTTP_UPDATE_OK:
          DEBUG_PROV(PSTR("[OTAUpdater.update()]: Update ok.\r\n")); // may not be called since we reboot the ESP
          break;
  }

  client.stop();
}
 
void OTAUpdater::setup() {
  m_lastOTACheckMillis = millis();
}

OTAUpdater::OTAUpdater() {    
}

OTAUpdater::~OTAUpdater(){ 
}
