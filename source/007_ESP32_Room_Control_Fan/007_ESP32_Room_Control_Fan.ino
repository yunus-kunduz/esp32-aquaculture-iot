// 5V 1 kanal röle kartı
// El Vantilatörü
// ESP32 NodeMCU
// Breadboard
// Jumper Kablolar

#include <WiFi.h>
#include <ThingerESP32.h>

// --- EVİNİZİN Wİ-Fİ BİLGİLERİNİ YAZIN ---
#define SSID "EV_WIFI_ADINIZ"
#define SSID_PASSWORD "EV_WIFI_SIFRENIZ"

// --- THINGER.IO KİMLİK BİLGİLERİ ---
#define USERNAME "Yunus_Kunduz"               
#define DEVICE_ID "esp32_role_kartim"         
#define DEVICE_CREDENTIAL "esp32_role_kartim" 

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

// --- FAN KONTROL PİNİ ---
#define FAN_RELAY_PIN 18 

void setup() 
{
  Serial.begin(115200);
  
  // Röle pinini çıkış olarak tanımla ve başlangıçta kapat
  pinMode(FAN_RELAY_PIN, OUTPUT);
  digitalWrite(FAN_RELAY_PIN, LOW); 

  // Thinger.io Wi-Fi Bağlantısını Başlat
  thing.add_wifi(SSID, SSID_PASSWORD);

  // --- THINGER.IO TELEFON KONTROL API'Sİ ---
  // Bu blok, telefondaki buton widget'ı ile röle pini arasında doğrudan köprü kurar.
  thing["vantilator_kontrol"] << [](pson& in)
  {
    if(in.is_empty())
    {
      // Telefon durum sorguladığında fanın anlık durumunu gönderir
      in = (digitalRead(FAN_RELAY_PIN) == HIGH); 
    }
    else
    {
      // Telefondaki butona basıldığında fanı aç veya kapat
      digitalWrite(FAN_RELAY_PIN, in ? HIGH : LOW);
      
      if(in) 
      {
        Serial.println("Telefondan Komut Geldi: Vantilatör ÇALISIYOR");
      } 
      else 
      {
        Serial.println("Telefondan Komut Geldi: Vantilatör DURDURULDU");
      }
    }
  };
}

void loop() {
  // Arka planda Thinger.io bulutunu sürekli dinler
  thing.handle();
}


