/* 
 Copyright (c) 2020-2021 Sinric Pro

 The copyright in these pages (including without limitation all text, graphics and computer code relating thereto or associated therewith) 
 and all other intellectual property and proprietary rights therein belongs to Sinric Pro and all rights are reserved.

 Permission is given for the downloading and temporary storage of one or more of these pages for the sole purpose of viewing them on a stand-alone personal computer or monitor. 
 Permanent copying or redistribution, or reproduction of all or part of this library in any form is strictly prohibited without express permission of Sinric Pro.
*/

#pragma once 

#include <Arduino.h> 
#include <WiFiClient.h> 
#include <WiFiClientSecure.h> 
#include "ProvDebug.h"

#ifdef ESP32
  #include <HTTPClient.h>
#else
  #include <ESP8266HTTPClient.h>
#endif 
 
struct HTTPResponse {
  int code;
  String body;
  explicit HTTPResponse(int _code, String _body) : code(_code), body(_body) {}
}; 

class HTTPRequest
{
public:
    void setAuthorizationHeader(const char* value);   
    int GET(const char * url, String *response); 
    int POST(const char * url, const char * body, String *response); 
    int DELETE(const char * url); 
    
private : 
    static unsigned char h2int(char c);         
    const char *authorization; 
    bool _clock = false;
};

/**
 * @brief Set the HTTP authorization header
 * @param value
 *      Header value
 */
void HTTPRequest::setAuthorizationHeader(const char* value){
   authorization =  value;
}

/**
 * @brief Send a HTTP GET request
 * @param url
 *      URL path
 * @param response
 *      Referece to the response.
 * @return
 *      Http code
 */
int HTTPRequest::GET(const char * url, String *response) {
  int httpCode = -1;
    
  DEBUG_PROV(PSTR("[HTTPRequest.GET()]: url: %s...\r\n"), url);

  #if defined(UNSECURE_API)
    WiFiClient *client = new WiFiClient;
  #else    
    #if defined(ESP8266) 
      BearSSL::WiFiClientSecure *client = new BearSSL::WiFiClientSecure;
      
      if(client) { 
        bool mfln = client->probeMaxFragmentLength(API_HOST, 443, 1024);
        if (mfln) {
          client->setBufferSizes(1024, 1024);
        } else {
          DEBUG_PROV(PSTR("[HTTPRequest.GET()]: Warning: probeMaxFragmentLength not supported!\n"));
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
      
      if (https.begin(*client, url)) {
        https.addHeader("Content-Type", "application/json");
        https.addHeader("Authorization", authorization);
        https.setTimeout(10000);
        https.setReuse(false);
        https.useHTTP10(true);
        
        httpCode = https.GET();
  
        if (httpCode > 0) {
          response->concat(https.getString());
        } else {
          response->concat(https.errorToString(httpCode));
        }
        https.end();
      } else {
        response->concat("Unable to connect");
      }

      // End extra scoping block
    }
  
    delete client;
  } else {
    response->concat("Unable to create HTTPClient");
  } 
  return httpCode;
}

/**
 * @brief Send a HTTP POST request
 * @param url
 *      URL path
 * @param body
 *      request body
 * @param response
 *      Referece to the response.
 * @return
 *      Http code
 */
int HTTPRequest::POST(const char * url, const char * body, String *response) {
  int httpCode = -1;
    
  DEBUG_PROV(PSTR("[HTTPRequest.POST()]: url: %s...\r\n"), url);

  #if defined(UNSECURE_API)
    WiFiClient *client = new WiFiClient;
  #else    
    #if defined(ESP8266) 
      BearSSL::WiFiClientSecure *client = new BearSSL::WiFiClientSecure;
      if(client) { 
        bool mfln = client->probeMaxFragmentLength(API_HOST, 443, 1024);
        if (mfln) {
          client->setBufferSizes(1024, 1024);
        } else {
          DEBUG_PROV(PSTR("[HTTPRequest.POST()]: Warning: probeMaxFragmentLength not supported!\n"));
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
      
      if (https.begin(*client, url)) {
        https.addHeader("Content-Type", "application/json");
        https.addHeader("Authorization", authorization);
        https.setTimeout(10000);
        https.setReuse(false);
        https.useHTTP10(true);
        
        httpCode = https.POST(body);
  
        if (httpCode > 0) {
          response->concat(https.getString());
        } else {
          response->concat(https.errorToString(httpCode));
        }
        https.end();
      } else {
        response->concat("Unable to connect");
      }

      // End extra scoping block
    }
  
    delete client;
  } else {
    response->concat("Unable to create HTTPClient");
  } 
  return httpCode;
}

/**
 * @brief Send a HTTP DELETE request
 * @param url
 *      URL path
 * @return
 *      Http code
 */
int HTTPRequest::DELETE(const char * url) {
 int httpCode = -1;

 DEBUG_PROV(PSTR("[HTTPRequest.DELETE()]: url: %s...\r\n"), url);

  #if defined(UNSECURE_API)
    WiFiClient *client = new WiFiClient;
  #else
    #if defined(ESP8266) 
      BearSSL::WiFiClientSecure *client = new BearSSL::WiFiClientSecure;      
      if(client) { 
        bool mfln = client->probeMaxFragmentLength(API_HOST, 443, 1024);
        if (mfln) {
          client->setBufferSizes(1024, 1024);
        } else {
          DEBUG_PROV(PSTR("[HTTPRequest.GET()]: Warning: probeMaxFragmentLength not supported!\n"));
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
      
      if (https.begin(*client, url)) {
        https.addHeader("Content-Type", "application/json");
        https.addHeader("Authorization", authorization);
        https.setTimeout(10000);
        https.useHTTP10(true);
        https.setReuse(false);
        
        httpCode = https.sendRequest("DELETE"); 
        https.end();
      }  

      // End extra scoping block
    }
  
    delete client;
  }  
  return httpCode;
}
 
