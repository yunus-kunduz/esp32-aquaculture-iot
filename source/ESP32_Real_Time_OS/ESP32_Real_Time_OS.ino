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

#define EEPROM_SIZE 2 
int timeON = 5;  
int timeOFF = 5; 

// --- RÖLE DURUM DEĞİŞKENLERİ (DAHA STABİL KONTROL İÇİN) ---
bool automationActive = false; 
bool relay1RealState = false;  
bool state2 = false;
bool state3 = false;
bool state4 = false;

unsigned long previousMillis = 0; 
unsigned long lastStreamMillis = 0; 

bool lastPhys1 = LOW;
bool lastPhys2 = LOW;
bool lastPhys3 = LOW;
bool lastPhys4 = LOW;

void setup() 
{
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

  EEPROM.begin(EEPROM_SIZE);
  int savedON = EEPROM.read(0);
  int savedOFF = EEPROM.read(1);
  
  if (savedON >= 0 && savedON <= 59) timeON = savedON;
  if (savedOFF >= 0 && savedOFF <= 59) timeOFF = savedOFF;

  thing.add_wifi(SSID, SSID_PASSWORD);

  // --- THINGER.IO TANIMLAMALARI ---
  thing["automation_switch"] << [](pson& in) { 
    if(in.is_empty()) in = automationActive; 
    else { 
      automationActive = in; 
      if(!automationActive) {
        relay1RealState = false;
        digitalWrite(SW1_RELAY_PIN, LOW);
      } else {
        previousMillis = millis(); 
        relay1RealState = true;
        digitalWrite(SW1_RELAY_PIN, HIGH);
      }
    } 
  };

  thing["switch1"] << [](pson& in) { 
    if(in.is_empty()) in = relay1RealState; 
    else { 
      if(!automationActive) { 
        relay1RealState = in;
        digitalWrite(SW1_RELAY_PIN, relay1RealState ? HIGH : LOW);
      }
    } 
  };

  // Switch 2, 3 ve 4 için daha garantili state yöntemi
  thing["switch2"] << [](pson& in){ if(in.is_empty()) in = state2; else { state2 = in; digitalWrite(SW2_RELAY_PIN, state2 ? HIGH : LOW); } };
  thing["switch3"] << [](pson& in){ if(in.is_empty()) in = state3; else { state3 = in; digitalWrite(SW3_RELAY_PIN, state3 ? HIGH : LOW); } };
  thing["switch4"] << [](pson& in){ if(in.is_empty()) in = state4; else { state4 = in; digitalWrite(SW4_RELAY_PIN, state4 ? HIGH : LOW); } };
  
  thing["timer_on"] << [](pson& in) {
    if(in.is_empty()) in = timeON; 
    else { 
      timeON = in; 
      EEPROM.write(0, timeON);
      EEPROM.commit(); 
      previousMillis = millis(); 
    }
  };
  
  thing["timer_off"] << [](pson& in) {
    if(in.is_empty()) in = timeOFF; 
    else { 
      timeOFF = in; 
      EEPROM.write(1, timeOFF);
      EEPROM.commit(); 
      previousMillis = millis(); 
    }
  };
  
  thing["kalan_sure"] >> [](pson& out) {
    if (automationActive) {
      unsigned long current = millis();
      unsigned long currentInterval = relay1RealState ? (timeON * 60000UL) : (timeOFF * 60000UL);
      unsigned long elapsed = current - previousMillis;
      
      if (currentInterval > elapsed) {
        unsigned long totalSeconds = (currentInterval - elapsed) / 1000;
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;
        
        char timeString[6];
        sprintf(timeString, "%02d:%02d", minutes, seconds);
        out = (const char*) timeString; 
      } else {
        out = "00:00";
      }
    } else {
      out = "00:00";
    }
  };
}

void loop() 
{
  thing.handle();
  
  unsigned long currentMillis = millis();
  
  if (automationActive) {
    unsigned long activeInterval = relay1RealState ? (timeON * 60000UL) : (timeOFF * 60000UL);

    if (currentMillis - previousMillis >= activeInterval) {
      previousMillis = currentMillis; 
      relay1RealState = !relay1RealState; // Yardımcı fonksiyona gerek kalmadan direkt tersine çevrildi
      digitalWrite(SW1_RELAY_PIN, relay1RealState ? HIGH : LOW);
    }
    
    if (currentMillis - lastStreamMillis >= 1000) {
      lastStreamMillis = currentMillis;
      thing.stream("kalan_sure");
    }
  }
  
  bool phys1 = digitalRead(button1);
  if (phys1 == HIGH && lastPhys1 == LOW) { 
    automationActive = !automationActive;
    if(automationActive) {
      relay1RealState = true;
      digitalWrite(SW1_RELAY_PIN, HIGH);
      previousMillis = millis();
    } else {
      relay1RealState = false;
      digitalWrite(SW1_RELAY_PIN, LOW);
    }
    thing.stream("automation_switch");
    delay(50); 
  }
  lastPhys1 = phys1;

  bool phys2 = digitalRead(button2);
  if (phys2 == HIGH && lastPhys2 == LOW) {
    state2 = !state2;
    digitalWrite(SW2_RELAY_PIN, state2 ? HIGH : LOW);
    thing.stream("switch2");
    delay(50);
  }
  lastPhys2 = phys2;

  bool phys3 = digitalRead(button3);
  if (phys3 == HIGH && lastPhys3 == LOW) {
    state3 = !state3;
    digitalWrite(SW3_RELAY_PIN, state3 ? HIGH : LOW);
    thing.stream("switch3");
    delay(50);
  }
  lastPhys3 = phys3;

  bool phys4 = digitalRead(button4);
  if (phys4 == HIGH && lastPhys4 == LOW) {
    state4 = !state4;
    digitalWrite(SW4_RELAY_PIN, state4 ? HIGH : LOW);
    thing.stream("switch4");
    delay(50);
  }
  lastPhys4 = phys4;
}