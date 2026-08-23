/* 
 Copyright (c) 2020-2021 Sinric Pro

 The copyright in these pages (including without limitation all text, graphics and computer code relating thereto or associated therewith) 
 and all other intellectual property and proprietary rights therein belongs to Sinric Pro and all rights are reserved.

 Permission is given for the downloading and temporary storage of one or more of these pages for the sole purpose of viewing them on a stand-alone personal computer or monitor. 
 Permanent copying or redistribution, or reproduction of all or part of this library in any form is strictly prohibited without express permission of Sinric Pro.
*/

#pragma once 

#include <Arduino.h> 
#include "HTTPRequest.h"
#include <ArduinoJson.h>
#include "ProvUtil.h"
#include "ProvStrings.h"
#include "ProvDebug.h"

class APIService {
  public:
    APIService();
    ~APIService();
    
    HTTPResponse authenticate(const char* token); 
    HTTPResponse getRooms();
    HTTPResponse createDevice(const String &name, const String &description, const String &ip, const String &macAddress, const String &productCode, const String &roomId, const String &accessKeyId);
    HTTPResponse createAccount(const String &name, const String &email, const String &password, const String &manufacturerId, const String &temperatureScale, const String &language, const String &recaptcha, const String &clientId, const String &timeZone);
    HTTPResponse autoConfigure(const String &email, const String &productCode, const String &macAddress, const String &manufacturerId, const String&password, const String &sign);
    HTTPResponse smartApConfigure(const String &configuration, const String &sign);

    void deleteDevicesByMacAddress(const char* bssid);
    void setAccessToken(const char* accessToken);
    void clearAccessToken();
  private: 
    const char* formatUrl(const char* path);
    const char* formatHttpHeader(const __FlashStringHelper* header, const char* value);

    char* m_accessToken = NULL;
    HTTPRequest m_httpRequest;
};

APIService::APIService(){ }

APIService::~APIService(){ } 

/**
* @brief Set server access token
*/
void APIService::setAccessToken(const char* token) {   
  clearAccessToken();
  m_accessToken = strdup(token);
}

/**
* @brief Clear memory 
*/
void APIService::clearAccessToken() {   
  if (m_accessToken != NULL) {
    DEBUG_PROV(PSTR("[APIService.clearAccessToken()]: Clear existing access token\n"));
    free(m_accessToken);
  }
}

/**
* @brief Format URL
*/
const char* APIService::formatUrl(const char* path) { 
  static char buf[512] = {0};
     
  #if defined(UNSECURE_API)
    snprintf_P(buf, sizeof(buf), PSTR("http://%s:%u/api/v1%s"), API_HOST, API_UNSECURE_PORT, path);
  #else
    snprintf_P(buf, sizeof(buf), PSTR("https://%s/api/v1%s"), API_HOST, path);
  #endif
       
  return buf;
}

const char* APIService::formatHttpHeader(const __FlashStringHelper* header, const char* value) {
  static char buf[512] = {0};
  snprintf_P(buf, sizeof(buf), PSTR("%s %s"), String(header).c_str(), value);
  return buf;
}

/**
* @brief Register for an account in the server
*/
HTTPResponse APIService::createAccount(const String &name, const String &email, const String &password, const String &manufacturerId, const String &temperatureScale, const String &language, 
                                        const String &recaptcha, const String &clientId, const String &timeZone) {    
  
  DynamicJsonDocument doc(2048);    
  doc[F("name")] = name;
  doc[F("email")] = email;
  doc[F("password")] = password;
  doc[F("manufacturerId")] = manufacturerId;
  doc[F("temperatureScale")] = temperatureScale;
  doc[F("language")] = language;
  doc[F("clientId")] = clientId;
  doc[F("timeZone")] = timeZone;
  doc[F("recaptcha")] = recaptcha; 
  
  String body;
  serializeJson(doc, body);
   
  const char* endpointUrl = formatUrl(str_users_register_url);  
  String response;
  int responseCode = m_httpRequest.POST(endpointUrl, body.c_str(), &response);  
  DEBUG_PROV(PSTR("[APIService.createAccount()]: HTTP response code: %u, size:%u, body:%s\n"), responseCode, response.length(), response.c_str());
  HTTPResponse res(responseCode, response.c_str());
  return res;   
}
 
/**
* @brief Authenticate an account in the server
*/
HTTPResponse APIService::authenticate(const char* token) {
  const char* httpHeader = formatHttpHeader(F("Basic"), token);
  m_httpRequest.setAuthorizationHeader(httpHeader);  
  
  String response;
  const char* endpointUrl = formatUrl(str_auth_endpoint_url);
  int responseCode = m_httpRequest.POST(endpointUrl, str_auth_request_body, &response);  

  DEBUG_PROV(PSTR("[APIService.authenticate()]: HTTP response code: %u, size:%u, body:%s\n"), responseCode, response.length(), response.c_str());
  HTTPResponse res(responseCode, response);
  return res;   
}

/**
* @brief Create a device in the server
*/
HTTPResponse APIService::createDevice(const String &name, const String &description, const String &ip, const String &macAddress, const String &productCode, const String &roomId, const String &accessKeyId) {
  if (!m_accessToken) {
    HTTPResponse res(401, "Unauthorized");
    return res;
  }

  DynamicJsonDocument doc(2048);
  doc[F("name")] = name;
  doc[F("description")] = description;
  doc[F("lastIpAddress")] = ip;
  doc[F("macAddress")] = macAddress;
  doc[F("productCode")] = productCode;
  doc[F("roomId")] = roomId;

  JsonArray includes = doc.createNestedArray(F("fields")); // include id and credential in the response.
  includes.add(F("id"));
  includes.add(F("credential")); 
     
  if(accessKeyId != nullptr) {
    doc[F("accessKeyId")] = accessKeyId; // link the first device access key id with next device 
  } 

  String body;
  serializeJson(doc, body);
  
  const char* httpHeader = formatHttpHeader(F("Bearer"), m_accessToken);  
  m_httpRequest.setAuthorizationHeader(httpHeader);  
    
  const char* endpointUrl = formatUrl(str_devices_url);  
  String response;
  int responseCode = m_httpRequest.POST(endpointUrl, body.c_str(), &response);   

  DEBUG_PROV(PSTR("[APIService.createDevice()]: HTTP response code: %u, size:%u, body:%s\n"), responseCode, response.length(), response.c_str());  
  HTTPResponse res(responseCode, response.c_str());
  return res; 
}

/**
* @brief Get user rooms 
*/
HTTPResponse APIService::getRooms() {
  if (!m_accessToken) {
    HTTPResponse res(401, "Unauthorized");
    return res;
  }

  const char* httpHeader = formatHttpHeader(F("Bearer"), m_accessToken);  
  m_httpRequest.setAuthorizationHeader(httpHeader);  
 
  const char* endpointUrl = formatUrl(str_rooms_url);
  String response;
  int responseCode = m_httpRequest.GET(endpointUrl, &response);    

  DEBUG_PROV(PSTR("[APIService.getRooms()]: HTTP response code: %u, size:%u, body:%s\n"), responseCode, response.length(), response.c_str());  
  HTTPResponse res(responseCode, response.c_str());
  return res;   
} 

/**
* @brief Delete existing devices by MAC address
*/
void APIService::deleteDevicesByMacAddress(const char* bssid) { 
  if (!m_accessToken) {
    return;
  }
  
  const char* httpHeader = formatHttpHeader(F("Bearer"), m_accessToken);  
  m_httpRequest.setAuthorizationHeader(httpHeader);  
  String urlPrefix(formatUrl(str_devices_by_mac_url));
  String endpointUrl =  urlPrefix + ProvUtil::urlencode(bssid);  
  m_httpRequest.DELETE(endpointUrl.c_str());  
}


/**
* @brief Auto configure the device
*/
HTTPResponse APIService::autoConfigure(const String &email, const String &productCode, const String &macAddress, const String &manufacturerId, const String &password, const String &sign) {   
  DynamicJsonDocument doc(1024);    
  doc[F("email")] = email;
  doc[F("productCode")] = productCode; 
  doc[F("macAddress")] = macAddress; 
  doc[F("manufacturerId")] = manufacturerId; 
  doc[F("password")] = password; 
  doc[F("sign")] = sign; 
  
  String body;
  serializeJson(doc, body);
   
  const char* endpointUrl = formatUrl(str_prov_autoconfigure_url);  
  String response;
  int responseCode = m_httpRequest.POST(endpointUrl, body.c_str(), &response);  
  DEBUG_PROV(PSTR("[APIService.autoConfigure()]: HTTP response code: %u, size:%u, body:%s\n"), responseCode, response.length(), response.c_str());
  HTTPResponse res(responseCode, response.c_str());
  return res;
}
 
HTTPResponse APIService::smartApConfigure(const String &configuration, const String &sign) {   
  DynamicJsonDocument doc(4096);    
  doc[F("configuration")] = configuration;
  doc[F("sign")] = sign;  

  String body;
  serializeJson(doc, body);
   
  const char* endpointUrl = formatUrl(str_prov_smartapconfigure_url);  
  String response;
  int responseCode = m_httpRequest.POST(endpointUrl, body.c_str(), &response);  
  DEBUG_PROV(PSTR("[APIService.smartApConfigure()]: HTTP response code: %u, size:%u, body:%s\n"), responseCode, response.length(), response.c_str());
  HTTPResponse res(responseCode, response.c_str());
  return res;
}

