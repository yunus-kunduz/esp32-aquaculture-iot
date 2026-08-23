// Toprak Nem Sensörü Modülü (Korozyona Dayanıklı / Kapasitif)
// 5V Mini Su Pompası (Dalgıç Pompa)
// Su Pompası İçin Akvaryum Hortumu
// 5V 1 Kanal Röle Modülü
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

// --- PİN TANIMLAMALARI ---
#define SOIL_SENSOR_PIN 34   // Kapasitif Toprak Nem Sensörü (Analog Giriş)
#define PUMP_RELAY_PIN 18    // Mini Su Pompası Rölesi (Dijital Çıkış)

// --- KALİBRASYON DEĞERLERİ ---
// Sensör tamamen havadayken (kuru) ve tamamen suyun içindeyken (ıslak) okunan analog değerler.
// Türkiye'de masada test ederken seri porttan bakıp bu sayıları hafifçe güncelleyebilirsiniz.
const int AIR_VALUE = 3200;   // Tamamen Kuru Toprak Değeri
const int WATER_VALUE = 1300; // Tamamen Islak Toprak Değeri

// Küresel Değişkenler
int soil_moisture_percent = 0;
bool manual_irrigation_command = false;
unsigned long pump_start_time = 0;
const unsigned long PUMP_DURATION = 3000; // 3 saniye (3000 milisaniye)

void setup() {
  Serial.begin(115200);
  
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  digitalWrite(PUMP_RELAY_PIN, LOW); // Başlangıçta pompa kapalı

  // Thinger.io Wi-Fi Bağlantısını Başlat
  thing.add_wifi(SSID, SSID_PASSWORD);

  // --- THINGER.IO TELEFON PANEL ENTEGRASYONU ---
  
  // 1. Telefondan Anlık Nem Yüzdesini İzleme Endpoint'i
  thing["plant_telemetry"] >> [](pson& out){
    out["moisture_level"] = soil_moisture_percent;
  };

  // 2. Telefondan "3 Saniyelik Can Suyu Ver" Buton Endpoint'i
  thing["remote_water_button"] << [](pson& in){
    if(in.is_empty()){
      in = manual_irrigation_command;
    } else {
      bool button_clicked = in;
      // Eğer telefondaki butona basıldıysa ve pompa şu an çalışmıyorsa
      if(button_clicked && !manual_irrigation_command && digitalRead(PUMP_RELAY_PIN) == LOW){
        manual_irrigation_command = true;
        pump_start_time = millis(); // Zamanlayıcıyı başlat
        digitalWrite(PUMP_RELAY_PIN, HIGH); // Pompayı çalıştır
        Serial.println("[TELEFON KOMUTU] Uzaktan 3 saniyelik sulama baslatildi!");
      }
    }
  };
}

void loop() {
  thing.handle();

  // --- 1. ADIM: TOPRAK NEMİNİ ÖLÇME VE YÜZDEYE ÇEVİRME ---
  static unsigned long last_sampling = 0;
  if (millis() - last_sampling > 2000) { // Her 2 saniyede bir oku
    last_sampling = millis();
    
    int raw_analog_value = analogRead(SOIL_SENSOR_PIN);
    
    // Analog değeri map fonksiyonu ile %0 ile %100 arasına dönüştürüyoruz
    soil_moisture_percent = map(raw_analog_value, AIR_VALUE, WATER_VALUE, 0, 100);
    
    // Sınır koruması (%0'ın altına veya %100'ün üzerine taşmaları engelle)
    if(soil_moisture_percent < 0) soil_moisture_percent = 0;
    if(soil_moisture_percent > 100) soil_moisture_percent = 100;

    Serial.print("Toprak Nem Orani: %");
    Serial.println(soil_moisture_percent);

    // --- 2. ADIM: OTONOM SULAMA MANTIĞI (EDGE LOGIC) ---
    // Eğer toprak %40'ın altına düşerse ve manuel mod aktif değilse otonom sula
    if (soil_moisture_percent < 40 && !manual_irrigation_command) {
      digitalWrite(PUMP_RELAY_PIN, HIGH);
      Serial.println("[OTONOM OTOMASYON] Toprak cok kuru! Pompa calisiyor...");
    } 
    // Toprak ideal nem seviyesi olan %70'e ulaştığında otonom pompayı kapat
    else if (soil_moisture_percent >= 70 && !manual_irrigation_command) {
      digitalWrite(PUMP_RELAY_PIN, LOW);
      Serial.println("[OTONOM OTOMASYON] Toprak ideal neme ulasti. Pompa durduruldu.");
    }
  }

  // --- 3. ADIM: TELEFONDAN GELEN MANUEL SULAMA ZAMANLAYICI KONTROLÜ ---
  if (manual_irrigation_command) {
    // Eğer 3 saniyelik süre dolduysa pompayı kapat ve durumu sıfırla
    if (millis() - pump_start_time >= PUMP_DURATION) {
      digitalWrite(PUMP_RELAY_PIN, LOW);
      manual_irrigation_command = false;
      
      // Thinger panelindeki butonu otomatik olarak tekrar "KAPALI" konumuna çek
      thing.stream(thing["remote_water_button"]); 
      Serial.println("[TELEFON KOMUTU] 3 saniyelik guvenli sulama suresi doldu. Pompa kapatildi.");
    }
  }
}
