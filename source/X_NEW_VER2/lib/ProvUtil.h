/* 
 Copyright (c) 2019-2021 Sinric Pro

 The copyright in these pages (including without limitation all text, graphics and computer code relating thereto or associated therewith) 
 and all other intellectual property and proprietary rights therein belongs to Sinric Pro and all rights are reserved.

 Permission is given for the downloading and temporary storage of one or more of these pages for the sole purpose of viewing them on a stand-alone personal computer or monitor. 
 Permanent copying or redistribution, or reproduction of all or part of this library in any form is strictly prohibited without express permission of Sinric Pro.
*/

#pragma once 
#include <sstream>
#include "ProvBase64.h"  
#include "ProvDebug.h"

#ifdef ESP32
  #include "esp_system.h"  
  #include <esp_wifi.h>
  #include <mbedtls/md.h>
#else
  #include <bearssl/bearssl_hmac.h>
#endif 
void touch1(void);
//#define STRHELPER(x) #x
//#define STR(x) STRHELPER(x) // stringifier

class ProvUtil {
  public:
      static bool setupWiFi();
      static bool connectToWiFi();
      static bool connectToWiFi(const char * wifi_ssid, const char * wifi_password);
      static std::string to_string(int a);
      static uint32_t getChipId32();
      static String getMacAddress();
      static String urlencode(String str);
      static void wait(int period);
      static String sign(String &payload, String &secret);
      static void setClock();
      static String randomString(uint32_t length);
  private:
      unsigned char h2int(char c);
};

 /**
 * @brief Connect to last connected WiFi
 * @param wifi_ssid
 *      WiFi SSID
 * @param wifi_password
 *      WiFi Password
 * @retval true
 *      Success
 * @retval false
 *      Failure
 */
bool ProvUtil::setupWiFi() {
  DEBUG_PROV(PSTR("[Util.setupWiFi()]: Connecting.."));

  #if defined(ESP8266)
    DEBUG_PROV(PSTR("[Util.setupWiFi()]: Set WiFi sleep mode to non-sleep..\r\n"));
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
  #endif  

  #if defined(ESP32)
    #if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 0, 0))
      WiFi.setMinSecurity(WIFI_AUTH_WEP); // https://github.com/espressif/arduino-esp32/blob/master/docs/source/troubleshooting.rst
    #endif    
  #endif
  
  WiFi.begin(); 
  
  uint8_t timeout = 40 * 2; // 20 seconds
    
  while (timeout && (WiFi.status() != WL_CONNECTED)) {
    delay(100);
    timeout--;
    DEBUG_PROV(".L");
    touch1();
  }

  bool isConnected = (WiFi.status() == WL_CONNECTED);

  #if defined(ESP8266)
    if (isConnected && !WiFi.localIP().isSet()) {
      DEBUG_PROV("[Util.setupWiFi()]: Invalid IP detected. Waiting for IP..\r\n");
  
      while (!WiFi.localIP().isSet()) {
        delay(500); 
      } 
    }  
  #else
    if (isConnected && WiFi.localIP().toString() == "0.0.0.0") {
      DEBUG_PROV("[Util.setupWiFi()]: Invalid IP detected. Waiting for IP..\r\n");
  
      while (WiFi.localIP().toString() != "0.0.0.0") {
        delay(500); 
      } 
    }  
  #endif

  if (isConnected) {
    WiFi.setAutoReconnect(true); 
    DEBUG_PROV(PSTR("done. IP is %s\r\n"), WiFi.localIP().toString().c_str());
    return true;
  } else {
    DEBUG_PROV(PSTR("failed!\r\n"));
    return false;
  }
}

bool ProvUtil::connectToWiFi() {
  return connectToWiFi("", "");  
}
  
 /**
 * @brief Connect to WiFi with ssid, password. If not provided, use the last connected
 * @param wifi_ssid
 *      WiFi SSID
 * @param wifi_password
 *      WiFi Password
 * @retval true
 *      Success
 * @retval false
 *      Failure
 */
bool ProvUtil::connectToWiFi(const char * wifi_ssid, const char * wifi_password) {
    #if defined(ESP32)
      #if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 0, 0))
        WiFi.setMinSecurity(WIFI_AUTH_WEP); // https://github.com/espressif/arduino-esp32/blob/master/docs/source/troubleshooting.rst
      #endif    
    #endif

    if(strlen(wifi_ssid) > 0 && strlen(wifi_password) > 0) {
      DEBUG_PROV(PSTR("[Util.connectToWiFi()]: Got SSID:%s, PWD: %s\r\n"), wifi_ssid, wifi_password);
      WiFi.persistent(true);
      WiFi.begin(wifi_ssid, wifi_password);
    } else {
      DEBUG_PROV(PSTR("[Util.connectToWiFi()]: Connecting to WiFi...\r\n"));
      WiFi.begin(); // Use credentails in NVM    
    }

    WiFi.setAutoReconnect(true);   
    
    uint8_t timeout = 40 * 2; // 20 seconds
    
    while (timeout && (WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0,0,0,0))) {
      delay(150);
      timeout--;
      DEBUG_PROV(".L");
      touch1();
    }
    
    DEBUG_PROV("\r\n");  
  
    if (WiFi.status() == WL_CONNECTED) {
      DEBUG_PROV(PSTR("[Util.connectToWiFi()]: WiFi connected."));
      DEBUG_PROV(PSTR("IP: %s\r\n"),  WiFi.localIP().toString().c_str());       
      return true;     
    } else {
      DEBUG_PROV(PSTR("[Util.connectToWiFi()]: WiFi connection failed. Please reboot the device and try again!\r\n"));
      return false;     
    }
} 


/**
 * @brief Get the ESP32/8266 ChipId
 * @return uint32_t Chip Id
 */
uint32_t ProvUtil::getChipId32()
{
  uint32_t chipID = 0;

  #ifdef ESP32
    chipID = ESP.getEfuseMac() >> 16;
  #else
    chipID = ESP.getChipId();
  #endif 

  return chipID;
}

/**
 * @brief Returns the MAC address NIC
 * @return String Mac address eg "5C:CF:7F:30:D9:9"
 */
String ProvUtil::getMacAddress() {
  return String(WiFi.macAddress());  
}

/**
 * @brief Encode the URL
 * @return String Encoded url 
 */
String ProvUtil::urlencode(String str) {
    String encodedString="";
    char c;
    char code0;
    char code1;
    //char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        //code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
    
}

unsigned char ProvUtil::h2int(char c)
{
    if (c >= '0' && c <='9'){
        return((unsigned char)c - '0');
    }
    if (c >= 'a' && c <='f'){
        return((unsigned char)c - 'a' + 10);
    }
    if (c >= 'A' && c <='F'){
        return((unsigned char)c - 'A' + 10);
    }
    return(0);
} 

std::string ProvUtil::to_string(int a) {
   std::ostringstream ss;
   ss << a;
   return ss.str(); 
}

//wait approx. [period] ms
void ProvUtil::wait(int ms) {
  unsigned long start = millis();
  while (millis() - start < ms) delay(10);
}

String ProvUtil::sign(String &payload, String &secret) {
  char hmacResult[32];
  
  {
  #if defined(ESP32)
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;  
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    mbedtls_md_hmac_starts(&ctx, (unsigned char *) secret.c_str(), secret.length());
    mbedtls_md_hmac_update(&ctx, (unsigned char *) payload.c_str(), payload.length());
    mbedtls_md_hmac_finish(&ctx, (unsigned char *) hmacResult);
    mbedtls_md_free(&ctx);
  #else
    br_hmac_key_context keyCtx;
    br_hmac_key_init(&keyCtx, &br_sha256_vtable, secret.c_str(), secret.length());
    br_hmac_context hmacCtx;
    br_hmac_init(&hmacCtx, &keyCtx, 0);
    br_hmac_update(&hmacCtx, payload.c_str(), payload.length());
    br_hmac_out(&hmacCtx, hmacResult);
  #endif
  }
 
  int encodedLen = base64_enc_len(32);  
  char encoded[encodedLen];
  base64_encode((char*)encoded, hmacResult, 32);
  return String(encoded);
}

/**
* @brief Set time via NTP, as required for x.509 validation
*/ 
void ProvUtil::setClock() { 
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");  // UTC

  DEBUG_PROV(PSTR("ProvUtil:: setClock(): Waiting for NTP time sync. It takes about 20-30 seconds sometimes!"));
  
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    yield();
    delay(500);
    DEBUG_PROV(PSTR("."));
    now = time(nullptr);
  }

  DEBUG_PROV(PSTR("\r\n"));
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  DEBUG_PROV(PSTR("Current time: "));
  DEBUG_PROV(PSTR("%s\r\n"), asctime(&timeinfo));
}

String ProvUtil::randomString(uint32_t length) {
    String alphanumericals = "abcdefghijklmnopqrstuvwxyz1234567890";
    String randomString = "";
    for (int i = 0; i < length; i++) {
        int randomInt = random(alphanumericals.length() - 1L);
        randomString += alphanumericals.substring(randomInt, randomInt + 1);
    }
    return randomString;
}