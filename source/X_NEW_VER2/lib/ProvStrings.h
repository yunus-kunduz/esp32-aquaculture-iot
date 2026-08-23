/* 
 Copyright (c) 2019-2021 Sinric Pro

 The copyright in these pages (including without limitation all text, graphics and computer code relating thereto or associated therewith) 
 and all other intellectual property and proprietary rights therein belongs to Sinric Pro and all rights are reserved.

 Permission is given for the downloading and temporary storage of one or more of these pages for the sole purpose of viewing them on a stand-alone personal computer or monitor. 
 Permanent copying or redistribution, or reproduction of all or part of this library in any form is strictly prohibited without express permission of Sinric Pro.
*/

#pragma once 

// URLs

static const char str_users_register_url[]        PROGMEM = "/users/register";   
static const char str_devices_url[]               PROGMEM = "/devices";   
static const char str_rooms_url[]                 PROGMEM = "/rooms?fields=id,name";
static const char str_devices_by_mac_url[]        PROGMEM = "/devices?macAddress=";
static const char str_auth_endpoint_url[]         PROGMEM = "/auth";   
static const char str_prov_autoconfigure_url[]    PROGMEM = "/devices/prov/autoconfigure";   
static const char str_prov_smartapconfigure_url[]    PROGMEM = "/devices/prov/smartapconfigure";  

// Authentication

static const char str_auth_request_body[]               PROGMEM = "{\"client_id\":\"mobile-app\", \"fields\":[\"success\", \"message\", \"accessToken\"]}";
static const char str_incorrect_username_or_password[]  PROGMEM = "{\"success\":false, \"message\":\"Incorrect username or password\"}";
static const char str_got_invalid_http_response_code[]  PROGMEM = "{\"success\":false, \"message\":\"Got invalid HTTP response code!\"}";
static const char str_invalid_login_token[]             PROGMEM = "{\"success\":false, \"message\":\"Invaid login token\"}";
static const char str_got_invalid_json_response[]       PROGMEM = "{\"success\":false, \"message\":\"Got invalid json response!\"}";

//WIFI

static const char str_failed_to_connect[]   PROGMEM = "{\"success\":false, \"message\":\"Failed to connect!\"}";
static const char str_invalid_ssid[]        PROGMEM = "{\"success\":false, \"message\":\"Invalid SSID!\"}";
static const char str_invalid_password[]    PROGMEM = "{\"success\":false, \"message\":\"Invalid Password!\"}";
static const char str_invalid_email[]        PROGMEM = "{\"success\":false, \"message\":\"Invalid Email!\"}";

// Configuration

static const char str_failed_to_save_configuration[]  PROGMEM = "{\"success\":false, \"message\":\"Failed to save configuration!\"}";

// Common

static const char str_success[] PROGMEM = "{\"success\":true, \"message\":\"Success!\"}";

static const char kJson[] PROGMEM = "application/json";
static const char kTxt[] PROGMEM = "text/plain";
static const char kAccessControlAllowOrigin[] PROGMEM = "Access-Control-Allow-Origin";
static const char kAnyIP[] PROGMEM = "*";
