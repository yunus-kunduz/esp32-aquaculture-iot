#include <ThingerESP32.h>
#include <EEPROM.h>

// --- THINGER.IO KİMLİK BİLGİLERİ ---
#define USERNAME "*********"               
#define DEVICE_ID "*********"         
#define DEVICE_CREDENTIAL "*********" 

// --- AĞ BİLGİLERİ ---
#define SSID "*********"
#define SSID_PASSWORD "*********"

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

// --- RÖLE VE BUTON PİNLERİ ---
#define SW1_RELAY_PIN 18  
#define SW2_RELAY_PIN 23  
#define SW3_RELAY_PIN 13  
#define SW4_RELAY_PIN 33  

#define button1 15
#define button2 5
#define button3 14
#define button4 25

// --- EEPROM (KALICI HAFIZA) AYARLARI ---
#define EEPROM_SIZE 1 
int timerMinutes = 5; 

// --- SCHEDULING (ZAMANLAMA) DEĞİŞKENLERİ ---
bool scheduleActive = false;        
bool relayState = false;            
unsigned long previousMillis = 0;   
unsigned long lastStreamMillis = 0; 

// EKSİK OLAN VE HATA VERDİREN DEĞİŞKEN EKLENDİ
bool lastPhys1 = LOW; 

void setup() {
  Serial.begin(115200);

  pinMode(SW1_RELAY_PIN, OUTPUT);
  pinMode(SW2_RELAY_PIN, OUTPUT);
  pinMode(SW3_RELAY_PIN, OUTPUT);
  pinMode(SW4_RELAY_PIN, OUTPUT);

  pinMode(button1, INPUT_PULLDOWN);
  pinMode(button2, INPUT_PULLDOWN);
  pinMode(button3, INPUT_PULLDOWN);
  pinMode(button4, INPUT_PULLDOWN);

  digitalWrite(SW1_RELAY_PIN, LOW);
  digitalWrite(SW2_RELAY_PIN, LOW);
  digitalWrite(SW3_RELAY_PIN, LOW);
  digitalWrite(SW4_RELAY_PIN, LOW);

  // --- EEPROM BAŞLATMA VE OKUMA ---
  EEPROM.begin(EEPROM_SIZE);
  int savedTime = EEPROM.read(0); 
  
  if (savedTime >= 0 && savedTime <= 59) {
    timerMinutes = savedTime; 
  }

  thing.add_wifi(SSID, SSID_PASSWORD);

  // --- ANAHTAR 1: ZAMANLAYICIYI BAŞLAT/DURDUR ---
  thing["switch1"] << [](pson& in){ 
    if(in.is_empty()) in = scheduleActive; 
    else { 
      scheduleActive = in; 
      if(scheduleActive) {
        relayState = true; 
        digitalWrite(SW1_RELAY_PIN, HIGH);
        previousMillis = millis(); 
      } else {
        relayState = false; 
        digitalWrite(SW1_RELAY_PIN, LOW);
      }
    } 
  };

  thing["switch2"] << [](pson& in){ if(!in.is_empty()) digitalWrite(SW2_RELAY_PIN, in ? HIGH : LOW); };
  thing["switch3"] << [](pson& in){ if(!in.is_empty()) digitalWrite(SW3_RELAY_PIN, in ? HIGH : LOW); };
  thing["switch4"] << [](pson& in){ if(!in.is_empty()) digitalWrite(SW4_RELAY_PIN, in ? HIGH : LOW); };

  // --- THINGER.IO SÜRE AYARI (SLIDER) & EEPROM ---
  thing["timer_slider"] << [](pson& in){
    if(in.is_empty()) {
      in = timerMinutes; 
    } else {
      timerMinutes = in; 
      EEPROM.write(0, timerMinutes);
      EEPROM.commit(); 
      previousMillis = millis(); 
    }
  };

  // --- THINGER.IO KALAN SÜRE GÖSTERGESİ ---
  thing["kalan_sure"] >> [](pson& out){
    if (scheduleActive && timerMinutes > 0) {
      unsigned long current = millis();
      unsigned long interval = timerMinutes * 60000UL;
      unsigned long elapsed = current - previousMillis;
      
      if (interval > elapsed) out = (interval - elapsed) / 1000;
      else out = 0;
    } else {
      out = 0;
    }
  };
}

void loop() {
  thing.handle();
  
  unsigned long currentMillis = millis();
  unsigned long interval = timerMinutes * 60000UL; 

  // --- SCHEDULING (ZAMANLAMA DÖNGÜSÜ) ---
  if (scheduleActive && timerMinutes > 0) {
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis; 
      relayState = !relayState;       
      digitalWrite(SW1_RELAY_PIN, relayState ? HIGH : LOW);
    }
    
    if (currentMillis - lastStreamMillis >= 1000) {
      lastStreamMillis = currentMillis;
      thing.stream("kalan_sure");
    }
  }

  // --- FİZİKSEL BUTON 1 ---
  bool phys1 = digitalRead(button1);
  if (phys1 == HIGH && lastPhys1 == LOW) { 
    scheduleActive = !scheduleActive; 
    if(scheduleActive) {
      relayState = true;
      digitalWrite(SW1_RELAY_PIN, HIGH);
      previousMillis = millis();
    } else {
      relayState = false;
      digitalWrite(SW1_RELAY_PIN, LOW);
    }
    thing.stream("switch1"); 
    delay(50); 
  }
  lastPhys1 = phys1;
}