/* 
 Copyright (c) 2020-2021 Sinric Pro

 The copyright in these pages (including without limitation all text, graphics and computer code relating thereto or associated therewith) 
 and all other intellectual property and proprietary rights therein belongs to Sinric Pro and all rights are reserved.

 Permission is given for the downloading and temporary storage of one or more of these pages for the sole purpose of viewing them on a stand-alone personal computer or monitor. 
 Permanent copying or redistribution, or reproduction of all or part of this library in any form is strictly prohibited without express permission of Sinric Pro.
*/

#pragma once 

#ifdef ESP8266
 
#include "ProvDebug.h"
#include "ProvBase64.h"

class CryptoBearSSL {
public:
    CryptoBearSSL();
    ~CryptoBearSSL();

    // aes-128-cbc
    static String aes128cbcEncrypt(String plain_data);
};

CryptoBearSSL::CryptoBearSSL() {}

CryptoBearSSL::~CryptoBearSSL() {}
 
String CryptoBearSSL::aes128cbcEncrypt(String plain_data) {
  int i;
  // PKCS#7 Padding (Encryption), Block Size : 16
  int len = plain_data.length();
  int n_blocks = len / 16 + 1;
  uint8_t n_padding = n_blocks * 16 - len;
  uint8_t data[n_blocks*16];
  memcpy(data, plain_data.c_str(), len);
  for(i = len; i < n_blocks * 16; i++){
    data[i] = n_padding;
  }
  
  uint8_t key[16], iv[16];
  memcpy(key, SMARTAP_CIPHER_KEY, 16);
  memcpy(iv, SMARTAP_CIPHER_IV, 16);

  // encryption context
  br_aes_big_cbcenc_keys encCtx;

  // reset the encryption context and encrypt the data
  br_aes_big_cbcenc_init(&encCtx, key, 16);
  br_aes_big_cbcenc_run( &encCtx, iv, data, n_blocks*16 );

  // Base64 encode
  len = n_blocks*16;
  char encoded_data[ base64_enc_len(len) ];
  base64_encode(encoded_data, (char *)data, len);
  
  return String(encoded_data);
}


#endif
