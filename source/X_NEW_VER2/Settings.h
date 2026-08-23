/* 
 Copyright (c) 2019-2022 Sinric Pro

 The copyright in these pages (including without limitation all text, graphics and computer code relating thereto or associated therewith) 
 and all other intellectual property and proprietary rights therein belongs to Sinric Pro and all rights are reserved.

 Permission is given for the downloading and temporary storage of one or more of these pages for the sole purpose of viewing them on a stand-alone personal computer or monitor. 
 Permanent copying or redistribution, or reproduction of all or part of this library in any form is strictly prohibited without express permission of Sinric Pro.
*/

#pragma once 

#include "SDKSettings.h" 
 
// Adjust accordingly
#define FIRMWARE_VERSION                "13.1.1"                   // Firmware version.

#if defined(ESP8266)
  #define PRIMARY_PROV_MODE               PROV_MODE_SMARTCONFIG     // Can be PROV_MODE_BLE or PROV_MODE_SMARTCONFIG or PROV_MODE_SMART_AP
#elif defined(ESP32)
  #define PRIMARY_PROV_MODE               PROV_MODE_BLE     // Can be PROV_MODE_BLE or PROV_MODE_SMARTCONFIG or PROV_MODE_SMART_AP
#endif


#define FALLBACK_PROV_MODE              PROV_MODE_CAPTIVE_PORTAL  // Can be only PROV_MODE_CAPTIVE_PORTAL for now.
#define FAILOVER_TIMEOUT                60000 * 45                // How long should wait before taking over as the primary provisioing mode. Default 45 mins.

#define AP_HOSTNAME                     "PROV_"         // SoftAP perfix.
#define BLE_PRODUCT_PREFIX              ""              // BLE device prefix. (followed by PROV_)
#define OTA_ENABLE                                      // Enable OTA Updates
#define OTA_CHECK_INTERVAL              60000 * 60 * 1 // Default: Once a day!. Do not change to lower values unless for testing

#define ENABLE_LED_INDICATOR                          // Enable LED indicator. 

#if defined(ENABLE_LED_INDICATOR)
  #define BOARD_LED_PIN               22        // Set LED pin - if you have a single-color LED attached
  //#define BOARD_LED_PIN_R             21        // Set R,G,B pins - if your LED is PWM RGB 
  //#define BOARD_LED_PIN_G             22
  //#define BOARD_LED_PIN_B             23
  //#define BOARD_LED_PIN_WS2812          8         // Set if your LED is WS2812 RGB
  
  #define BOARD_LED_INVERSE             false     // true if LED is common anode, false if common cathode
  #define BOARD_LED_BRIGHTNESS          64        // 0..255 brightness control  
  #define BOARD_PWM_MAX                 1023

  #if defined(ESP32)
    #define LEDC_CHANNEL_1              1
    #define LEDC_CHANNEL_2              2
    #define LEDC_CHANNEL_3              3
    #define LEDC_TIMER_BITS             10
    #define LEDC_BASE_FREQ              12000
  #endif
  
  #define USE_TICKER // Use Ticker library for animations
#endif

//#define ENABLE_RESET_BUTTON                             // Enable Reset button during provisioning.

#if defined(ENABLE_RESET_BUTTON)
  #define BOARD_BUTTON_PIN              0     // Pin where user button is attached
  #define BOARD_BUTTON_ACTIVE_LOW       true  // true if button is "active-low"
  #define BUTTON_HOLD_TIME_INDICATION   3000 // hold-wait time to change the indicator. (going to take an action)
  #define BUTTON_HOLD_TIME_ACTION       5000 // hold time to take the action (since above indication). 
  #define BUTTON_INTERRUPT_PRESS_COUNT  3 // press 3 times to exit the current provisionig (SmartConfig or BLE) mode to enter AP mode. 
#endif

#if defined(ESP32)
  #if defined(OTA_ENABLE)
    #define SECURE_OTA // Enable HTTPS OTA updates on ESP32
  #endif
#elif defined(ESP8266)
  #define UNSECURE_API                true // AP mode API calls to server over http for ESP8266.
  #define API_UNSECURE_PORT           80   // HTTP API Port.
#endif

// Validate Settings.h settings.

#if !defined(PRIMARY_PROV_MODE)
  #error "PRIMARY_PROV_MODE not found in Settings.h!"
#endif

#if defined(ESP8266)
  #if (PRIMARY_PROV_MODE == PROV_MODE_BLE)
    #error "ESP8266 does not support BLE. Please check PRIMARY_PROV_MODE in Settings.h!"
  #endif
#endif
