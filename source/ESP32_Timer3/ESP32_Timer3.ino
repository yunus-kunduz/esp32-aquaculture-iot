#include <ThingerESP32.h>
#include <EEPROM.h>
#include <time.h> 

// --- THINGER.IO KİMLİK BİLGİLERİ ---
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

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800; 
const int   daylightOffset_sec = 0;

struct ConfigStruct 
{
  int tOn, tOff;                     
  int startH, startM, stopH, stopM;  
  bool days[7];                      
} cfg;

bool automationActive = false; 
bool scheduleActive = false;   
bool relay1RealState = false;  

bool state2 = false;
bool state3 = false;
bool state4 = false;

unsigned long previousMillis = 0; 
unsigned long lastStreamMillis = 0; 
bool lastPhys1 = LOW, lastPhys2 = LOW, lastPhys3 = LOW, lastPhys4 = LOW;

void saveConfig() 
{
  EEPROM.put(0, cfg);
  EEPROM.commit();
}

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

  EEPROM.begin(sizeof(ConfigStruct));
  EEPROM.get(0, cfg);
  
  if (cfg.tOn < 0 || cfg.tOn > 59) cfg.tOn = 5;
  if (cfg.tOff < 0 || cfg.tOff > 59) cfg.tOff = 5;
  if (cfg.startH < 0 || cfg.startH > 23) cfg.startH = 0;
  if (cfg.stopH < 0 || cfg.stopH > 23) cfg.stopH = 0;

  thing.add_wifi(SSID, SSID_PASSWORD);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); 

  thing["automation_switch"] << [](pson& in)
  { 
    if(in.is_empty()) in = automationActive; 
    else 
    {
       automationActive = in; 
       if(automationActive) 
       { 
        scheduleActive = false; 
        previousMillis = millis(); 
        relay1RealState = true; 
        digitalWrite(SW1_RELAY_PIN, HIGH); 
       } 
       else 
       { 
        relay1RealState = false; 
        digitalWrite(SW1_RELAY_PIN, LOW); 
       } 
    }
  };

  thing["schedule_mode"] << [](pson& in)
  { 
    if(in.is_empty()) in = scheduleActive; 
    else 
    { 
      scheduleActive = in; 
      if(scheduleActive) 
      { 
        automationActive = false; 
        relay1RealState = false; 
        digitalWrite(SW1_RELAY_PIN, LOW); 
      } 
      else 
      { 
        relay1RealState = false; 
        digitalWrite(SW1_RELAY_PIN, LOW); 
      } 
    }
  };

  thing["switch1"] << [](pson& in){ if(in.is_empty()) in = relay1RealState; else { if(!automationActive && !scheduleActive) { relay1RealState = in; digitalWrite(SW1_RELAY_PIN, relay1RealState ? HIGH : LOW); } } };
  thing["switch2"] << [](pson& in){ if(in.is_empty()) in = state2; else { state2 = in; digitalWrite(SW2_RELAY_PIN, state2 ? HIGH : LOW); } };
  thing["switch3"] << [](pson& in){ if(in.is_empty()) in = state3; else { state3 = in; digitalWrite(SW3_RELAY_PIN, state3 ? HIGH : LOW); } };
  thing["switch4"] << [](pson& in){ if(in.is_empty()) in = state4; else { state4 = in; digitalWrite(SW4_RELAY_PIN, state4 ? HIGH : LOW); } };

  thing["timer_on"] << [](pson& in){ if(in.is_empty()) in = cfg.tOn; else { cfg.tOn = in; saveConfig(); previousMillis = millis(); } };
  thing["timer_off"] << [](pson& in){ if(in.is_empty()) in = cfg.tOff; else { cfg.tOff = in; saveConfig(); previousMillis = millis(); } };

  thing["start_h"] << [](pson& in){ if(in.is_empty()) in = cfg.startH; else { cfg.startH = in; saveConfig(); } };
  thing["start_m"] << [](pson& in){ if(in.is_empty()) in = cfg.startM; else { cfg.startM = in; saveConfig(); } };
  thing["stop_h"] << [](pson& in){ if(in.is_empty()) in = cfg.stopH; else { cfg.stopH = in; saveConfig(); } };
  thing["stop_m"] << [](pson& in){ if(in.is_empty()) in = cfg.stopM; else { cfg.stopM = in; saveConfig(); } };

  thing["d_sun"] << [](pson& in){ if(in.is_empty()) in = cfg.days[0]; else { cfg.days[0] = in; saveConfig(); } }; 
  thing["d_mon"] << [](pson& in){ if(in.is_empty()) in = cfg.days[1]; else { cfg.days[1] = in; saveConfig(); } }; 
  thing["d_tue"] << [](pson& in){ if(in.is_empty()) in = cfg.days[2]; else { cfg.days[2] = in; saveConfig(); } }; 
  thing["d_wed"] << [](pson& in){ if(in.is_empty()) in = cfg.days[3]; else { cfg.days[3] = in; saveConfig(); } }; 
  thing["d_thu"] << [](pson& in){ if(in.is_empty()) in = cfg.days[4]; else { cfg.days[4] = in; saveConfig(); } }; 
  thing["d_fri"] << [](pson& in){ if(in.is_empty()) in = cfg.days[5]; else { cfg.days[5] = in; saveConfig(); } }; 
  thing["d_sat"] << [](pson& in){ if(in.is_empty()) in = cfg.days[6]; else { cfg.days[6] = in; saveConfig(); } }; 

  thing["kalan_sure"] >> [](pson& out)
  {
    if (automationActive) 
    {
      unsigned long current = millis();
      unsigned long interval = relay1RealState ? (cfg.tOn * 60000UL) : (cfg.tOff * 60000UL);
      unsigned long elapsed = current - previousMillis;
      if (interval > elapsed) 
      {
        int m = (interval - elapsed) / 1000 / 60;
        int s = ((interval - elapsed) / 1000) % 60;
        char buf[6]; sprintf(buf, "%02d:%02d", m, s);
        out = (const char*) buf; 
      } 
      else out = "00:00";
    } 
    else out = "00:00";
  };

  thing["guncel_saat"] >> [](pson& out)
  {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo))
    {
      out = "Saat Bekleniyor...";
    } 
    else 
    {
      char timeString[20];
      strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeinfo);
      out = (const char*) timeString;
    }
  };
}

void loop() 
{
  thing.handle();
  unsigned long currentMillis = millis();

  if (automationActive) 
  {
    unsigned long interval = relay1RealState ? (cfg.tOn * 60000UL) : (cfg.tOff * 60000UL);
    if (currentMillis - previousMillis >= interval) 
    {
      previousMillis = currentMillis; 
      relay1RealState = !relay1RealState; 
      digitalWrite(SW1_RELAY_PIN, relay1RealState ? HIGH : LOW);
    }
  }

  if (scheduleActive) 
  {
    struct tm timeinfo;
    if(getLocalTime(&timeinfo)) 
    { 
      int current_wday = timeinfo.tm_wday; 
      
      if (cfg.days[current_wday] == true) 
      {
        int current_total_mins = timeinfo.tm_hour * 60 + timeinfo.tm_min;
        int start_total_mins = cfg.startH * 60 + cfg.startM;
        int stop_total_mins = cfg.stopH * 60 + cfg.stopM;

        if (start_total_mins < stop_total_mins) 
        {
          if (current_total_mins >= start_total_mins && current_total_mins < stop_total_mins) relay1RealState = true;
          else relay1RealState = false;
        } 
        else 
        {
          if (current_total_mins >= start_total_mins || current_total_mins < stop_total_mins) relay1RealState = true;
          else relay1RealState = false;
        }
      } 
      else 
      {
        relay1RealState = false; 
      }
      digitalWrite(SW1_RELAY_PIN, relay1RealState ? HIGH : LOW);
    }
  }

  if (currentMillis - lastStreamMillis >= 1000) 
  {
    lastStreamMillis = currentMillis;
    if(automationActive) thing.stream("kalan_sure");
    thing.stream("guncel_saat");
  }

  bool phys1 = digitalRead(button1);
  if (phys1 == HIGH && lastPhys1 == LOW) 
  { 
    if(!automationActive) 
    {
      automationActive = true; 
      scheduleActive = false; 
      relay1RealState = true; 
      previousMillis = millis(); 
      digitalWrite(SW1_RELAY_PIN, HIGH);
    } 
    else 
    {
      automationActive = false; 
      relay1RealState = false; 
      digitalWrite(SW1_RELAY_PIN, LOW);
    }
    thing.stream("automation_switch"); 
    thing.stream("schedule_mode");
    delay(50); 
  }
  lastPhys1 = phys1;

  bool phys2 = digitalRead(button2); 
  if (phys2 == HIGH && lastPhys2 == LOW) 
  { 
    state2 = !state2; 
    digitalWrite(SW2_RELAY_PIN, state2 ? HIGH : LOW); 
    thing.stream("switch2"); 
    delay(50); 
  } 
  lastPhys2 = phys2;

  bool phys3 = digitalRead(button3); 
  if (phys3 == HIGH && lastPhys3 == LOW) 
  { 
    state3 = !state3; 
    digitalWrite(SW3_RELAY_PIN, state3 ? HIGH : LOW); 
    thing.stream("switch3"); 
    delay(50); 
  } 
  lastPhys3 = phys3;

  bool phys4 = digitalRead(button4); 
  if (phys4 == HIGH && lastPhys4 == LOW) 
  { 
    state4 = !state4; 
    digitalWrite(SW4_RELAY_PIN, state4 ? HIGH : LOW); 
    thing.stream("switch4"); 
    delay(50); 
  } 
  lastPhys4 = phys4;
}