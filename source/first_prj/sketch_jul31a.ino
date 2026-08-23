#include <WiFi.h>
#include <ThingerESP32.h>

// --- THINGER.IO KİMLİK BİLGİLERİ ---
#define USERNAME "***********"               
#define DEVICE_ID "*********"         
#define DEVICE_CREDENTIAL "**********" 

// --- AĞ BİLGİLERİ (Telefonunuzun Dağıttığı İnternet) ---
#define SSID "************"
#define SSID_PASSWORD "**********"

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

// --- RÖLE PİNLERİ ---
#define SW1_RELAY_PIN 18  

void setup() 
{
  Serial.begin(115200);

  
  pinMode(SW1_RELAY_PIN, OUTPUT);
  digitalWrite(SW1_RELAY_PIN, LOW); 

  
  thing.add_wifi(SSID, SSID_PASSWORD);

  
  thing["lamba"] << [](pson& in)
  {
    if(in.is_empty())
    {
      in = (digitalRead(SW1_RELAY_PIN) == HIGH); 
    }
    else
    {
      if(in)
      {
        digitalWrite(SW1_RELAY_PIN, HIGH);
        Serial.println("Röle AÇILDI");
      }
      else
      {
        digitalWrite(SW1_RELAY_PIN, LOW);
        Serial.println("Röle KAPATILDI");
      }
    }
  };
}

void loop() 
{
  thing.handle();
}
