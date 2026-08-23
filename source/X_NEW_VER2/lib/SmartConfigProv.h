/* 
 Copyright (c) 2020-2021 Sinric Pro

 The copyright in these pages (including without limitation all text, graphics and computer code relating thereto or associated therewith) 
 and all other intellectual property and proprietary rights therein belongs to Sinric Pro and all rights are reserved.

 Permission is given for the downloading and temporary storage of one or more of these pages for the sole purpose of viewing them on a stand-alone personal computer or monitor. 
 Permanent copying or redistribution, or reproduction of all or part of this library in any form is strictly prohibited without express permission of Sinric Pro.
*/
 
#pragma once

#ifdef ESP8266

#include "ESP8266WiFi.h"  
#include "smartconfig.h"
#include "ProvBase64.h"
#include "ProvDebug.h"

class SmartConfigProv
{  
public: 
  bool beginSmartConfig();
  bool stopSmartConfig();
  bool smartConfigDone();

  static bool s_smartConfigDone;
protected:
  static bool s_smartConfigStarted;
  static void s_smartConfigCallback(uint32_t status, void* result); 
};
 

bool SmartConfigProv::s_smartConfigStarted = false;
bool SmartConfigProv::s_smartConfigDone = false;

/**
 * Start SmartConfig
 */
bool SmartConfigProv::beginSmartConfig() {
  int DEBUG = 1;
  
  if (s_smartConfigStarted) {
      return false;
  }

  if(!WiFi.enableSTA(true)) {
      // enable STA failed
      return false;
  }
  
  if(smartconfig_start(reinterpret_cast<sc_callback_t>(&SmartConfigProv::s_smartConfigCallback), DEBUG)) {
      s_smartConfigStarted = true;
      s_smartConfigDone = false;
      return true;
  }
  
  return false; 
}

/**
 *  Stop SmartConfig
 */
bool SmartConfigProv::stopSmartConfig() {
  if (!s_smartConfigStarted) {
      return true;
  }

  if (smartconfig_stop()) {
      s_smartConfigStarted = false;
      return true;
  }

  return false;
}

/**
 * Query SmartConfig status, to decide when stop config
 * @return smartConfig Done
 */
bool SmartConfigProv::smartConfigDone() {
  if (!s_smartConfigStarted) {
      return false;
  }

  return s_smartConfigDone;
}

/**
 * s_smartConfigCallback
 * @param st
 * @param result
 */
void SmartConfigProv::s_smartConfigCallback(uint32_t st, void* result) {
    sc_status status = (sc_status) st;
 
    if(status == SC_STATUS_WAIT) {
      DEBUG_PROV(PSTR("SmartConfigProv.s_smartConfigCallback(): SC_STATUS_WAIT\n"));
    }
    else if(status == SC_STATUS_FIND_CHANNEL) {
      DEBUG_PROV(PSTR("SmartConfigProv.s_smartConfigCallback(): SC_STATUS_FIND_CHANNEL\n"));
    }
    else if(status == SC_STATUS_GETTING_SSID_PSWD) {
      DEBUG_PROV(PSTR("SmartConfigProv.s_smartConfigCallback(): SC_STATUS_GETTING_SSID_PSWD\n"));
    }  
    else if(status == SC_STATUS_LINK) {
      DEBUG_PROV(PSTR("SmartConfigProv.s_smartConfigCallback(): SC_STATUS_LINK\n"));
      
      ProvState::getInstance()->state = ProvState::MODE_CONNECTING_WIFI;
        
      station_config* sta_conf = reinterpret_cast<station_config*>(result);
      int encoded_len = strlen((char*)sta_conf->password);
      char *encoded_data = (char*)&sta_conf->password[0];
  
      // base64 decode
      int len = base64_dec_len(encoded_data, encoded_len);
      uint8_t data[ len ];
      base64_decode((char *)data, encoded_data, encoded_len);

      // make a local copy of the key, iv
      uint8_t key[16], iv[16];
      memcpy(key, SMARTCONFIG_CIPHER_KEY, 16);
      memcpy(iv,  SMARTCONFIG_CIPHER_IV, 16);

      // aes cbc decrypt
      int n_blocks = len / 16;      
      br_aes_big_cbcdec_keys decCtx;      
      br_aes_big_cbcdec_init(&decCtx, key, 16);
      br_aes_big_cbcdec_run( &decCtx, iv, data, n_blocks*16 ); 
    
      // PKCS#7 Padding
      uint8_t n_padding = data[n_blocks*16-1];
      len = n_blocks*16 - n_padding;
      int plain_data_len = len + 1;
      char plain_data[plain_data_len];
      memcpy(plain_data, data, len);
      plain_data[len] = '\0'; 

      // copy the decrypted password
      memcpy(sta_conf->password, plain_data, plain_data_len);

      DEBUG_PROV(PSTR("SmartConfigProv.s_smartConfigCallback(): Connecting to ssid:%s, pass:%s\n"), (char *)sta_conf->ssid, (char *)sta_conf->password);        

      // connect to wifi 
      wifi_station_set_config(sta_conf);
      wifi_station_disconnect();
      wifi_station_connect();
      
    } else if(status == SC_STATUS_LINK_OVER) {
      DEBUG_PROV(PSTR("SmartConfigProv.s_smartConfigCallback(): SC_STATUS_LINK_OVER\n"));
      ProvState::getInstance()->state = ProvState::MODE_WIFI_CONNECTED_WAIT_APP_CREDENTIALS;
      
      //if(result){
      //    ip4_addr_t * ip = (ip4_addr_t *)result;
      //    Serial.printf("Sender IP: " IPSTR, IP2STR(ip));
      //}        
      WiFi.stopSmartConfig();
      s_smartConfigDone = true;
    }
} 
#endif
