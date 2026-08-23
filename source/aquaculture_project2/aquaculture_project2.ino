#include <WiFi.h>
#include <ThingerESP32.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>
#include <OneWire.h>           // Required for DS18B20 temperature sensor
#include <DallasTemperature.h> // Temperature sensor library

// --- NETWORK CONFIGURATION ---
#define SSID "*********"
#define SSID_PASSWORD "*********"

// --- THINGER.IO CREDENTIALS ---
#define THINGER_USERNAME "*********"               
#define THINGER_DEVICE_ID "*********"         
#define THINGER_CREDENTIAL "*********" 


#define APP_KEY           "bc5f5461-4191-427a-aXXXXXXXXXXXXXXXX"
#define APP_SECRET        "5b429379-1377-4409-bf94-88ab52e960c4-faf8d4a8-c893-49e9-aXXXXXXXXXXXXXXXX"
#define FILTRATION_ID     "6a71a8e029c6be334287f228"
#define FEEDER_ID         "6a71a93e29c6be334287f29d"


#define SW1_RELAY_PIN 18  // Relay 1: Water Filtration Pump
#define SW2_RELAY_PIN 23  // Relay 2: Aerator / Oxygen Pump
#define SW3_RELAY_PIN 13  // Relay 3: Water Heater
#define SW4_RELAY_PIN 33  // Relay 4: Automated Fish Feeder

#define BUTTON_1 15
#define BUTTON_2 5
#define BUTTON_3 14
#define BUTTON_4 25


#define TURBIDITY_PIN 34   // Analog Input for Turbidity Sensor
#define TEMPERATURE_PIN 4  // Digital Input for DS18B20 Temperature Sensor

OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature sensors(&oneWire);

ThingerESP32 thing(THINGER_USERNAME, THINGER_DEVICE_ID, THINGER_CREDENTIAL);


float tank_temperature = 0.0;
int tank_turbidity = 0;


bool onFiltrationPumpState(const String &deviceId, bool &state) 
{
  digitalWrite(SW1_RELAY_PIN, state ? HIGH : LOW);
  Serial.printf("Alexa Event: Filtration Pump set to %s\r\n", state ? "ON" : "OFF");
  return true; 
}

bool onFishFeederState(const String &deviceId, bool &state) 
{
  digitalWrite(SW4_RELAY_PIN, state ? HIGH : LOW);
  Serial.printf("Alexa Event: Fish Feeder set to %s\r\n", state ? "ON" : "OFF");
  return true; 
}

void setup() 
{
  Serial.begin(115200);
  sensors.begin(); 

  pinMode(SW1_RELAY_PIN, OUTPUT); digitalWrite(SW1_RELAY_PIN, LOW);
  pinMode(SW2_RELAY_PIN, OUTPUT); digitalWrite(SW2_RELAY_PIN, LOW);
  pinMode(SW3_RELAY_PIN, OUTPUT); digitalWrite(SW3_RELAY_PIN, LOW);
  pinMode(SW4_RELAY_PIN, OUTPUT); digitalWrite(SW4_RELAY_PIN, LOW);

  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  pinMode(BUTTON_3, INPUT_PULLUP);
  pinMode(BUTTON_4, INPUT_PULLUP);

  
  thing.add_wifi(SSID, SSID_PASSWORD);
  
  
  thing["aquaculture_telemetry"] >> [](pson& out)
  {
    out["temperature"] = tank_temperature;
    out["turbidity"] = tank_turbidity;
  };

  
  SinricProSwitch& filtrationSwitch = SinricPro[FILTRATION_ID];
  filtrationSwitch.onPowerState(onFiltrationPumpState);

  SinricProSwitch& feederSwitch = SinricPro[FEEDER_ID];
  feederSwitch.onPowerState(onFishFeederState);

  SinricPro.begin(APP_KEY, APP_SECRET);
}

void loop() 
{
  
  thing.handle();
  SinricPro.handle();

  static unsigned long last_sampling_time = 0;
  if (millis() - last_sampling_time > 2000) 
  {
    last_sampling_time = millis();
    
    sensors.requestTemperatures(); 
    tank_temperature = sensors.getTempCByIndex(0);
    tank_turbidity = analogRead(TURBIDITY_PIN); 

    
    if (tank_turbidity > 2500) 
    { 
      digitalWrite(SW1_RELAY_PIN, HIGH);
      
      SinricProSwitch& filtrationSwitch = SinricPro[FILTRATION_ID];
      filtrationSwitch.sendPowerStateEvent(true);
    }
  }

  if (digitalRead(BUTTON_1) == LOW) 
  { 
    bool currentState = digitalRead(SW1_RELAY_PIN);
    digitalWrite(SW1_RELAY_PIN, !currentState); 

    SinricProSwitch& filtrationSwitch = SinricPro[FILTRATION_ID];
    filtrationSwitch.sendPowerStateEvent(!currentState);
    
    delay(300);
  }
}
