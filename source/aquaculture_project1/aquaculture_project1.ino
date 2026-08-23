#include <WiFi.h>
#include <ThingerESP32.h>
#include <OneWire.h>           // Required for DS18B20 temperature sensor
#include <DallasTemperature.h> // Temperature sensor library

// --- THINGER.IO CREDENTIALS ---
#define USERNAME "*******"               
#define DEVICE_ID "*******"         
#define DEVICE_CREDENTIAL "******" 

// --- NETWORK CONFIGURATION ---
#define SSID "********"
#define SSID_PASSWORD "*********"

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);


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


float tank_temperature = 0.0;
int tank_turbidity = 0;

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

  
  thing["filtration_pump"] << [](pson& in)
  {
    if(in.is_empty()) in = (digitalRead(SW1_RELAY_PIN) == HIGH);
    else digitalWrite(SW1_RELAY_PIN, in ? HIGH : LOW);
  };

  
  thing["fish_feeder"] << [](pson& in){
    if(in.is_empty()) in = (digitalRead(SW4_RELAY_PIN) == HIGH);
    else digitalWrite(SW4_RELAY_PIN, in ? HIGH : LOW);
  };
}

void loop() 
{
  thing.handle();

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
    }
  }

  
  if (digitalRead(BUTTON_1) == LOW) 
  { 
    digitalWrite(SW1_RELAY_PIN, !digitalRead(SW1_RELAY_PIN)); 
    delay(300);
  }
}
