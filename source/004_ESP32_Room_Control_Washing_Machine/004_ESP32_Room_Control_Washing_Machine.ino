// SCT-013 Akım Sensörü (30A Veya 100A)
// SCT-013 Kulaklık Jakı Dönüştürücü Kartı
// Dişi-Erkek Jumper Kablo
// ESP32 NodeMCU ve Breadboard
// 1 metrelik uzatma kablosu (sct-013 sensörü kelepçe bağlantısı için prize giden kabloyu kesip içinden tek bir hatta takmak gerekiyor. Örn: faz)
// Thinger.io + kütüphaneleri indir.

#include <WiFi.h>
#include <ThingerESP32.h>
#include "EmonLib.h" // Akım sensörü hesaplama kütüphanesi

// --- EVİNİZİN Wİ-Fİ BİLGİLERİNİ YAZIN ---
#define SSID "EV_WIFI_ADINIZ"
#define SSID_PASSWORD "EV_WIFI_SIFRENIZ"

// --- THINGER.IO KİMLİK BİLGİLERİ ---
#define USERNAME "Yunus_Kunduz"               
#define DEVICE_ID "esp32_role_kartim"         
#define DEVICE_CREDENTIAL "esp32_role_kartim" 

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);
EnergyMonitor emon1; // Akım izleme nesnesi oluşturuluyor

// --- AKIM SENSÖRÜ PİN TANIMLAMASI ---
#define CURRENT_SENSOR_PIN 34 

// Makine durumunu tutan değişken (Telefonda görünecek yazı)
String washing_machine_status = "OFFLINE"; 

void setup() {
  Serial.begin(115200);

  // Thinger.io Wi-Fi Bağlantısı
  thing.add_wifi(SSID, SSID_PASSWORD);

  // SCT-013 Sensör Kalibrasyonu 
  // (34: Analog pin numarası, 30: 30A'lik sensör için kalibrasyon katsayısıdır)
  emon1.current(CURRENT_SENSOR_PIN, 30.0); 

  // --- THINGER.IO TELEMETRİ BAĞLANTISI ---
  // Telefonunuzdaki Thinger panelinde görünecek veri etiketleri
  thing["appliance_monitor"] >> [](pson& out){
    out["machine_status"] = washing_machine_status;
  };
}

void loop() {
  thing.handle();

  // Her 3 saniyede bir akım ölçümü yap (Sistemi yormamak için)
  static unsigned long last_measurement = 0;
  if (millis() - last_measurement > 3000) {
    last_measurement = millis();

    // 1480 örnek alarak Irms (Etkin Akım) değerini hesapla
    double Amps = emon1.calcIrms(1480); 
    
    // Güvenlik ve gürültü filtresi: 0.15 Amper altındaki parazitleri sıfır kabul et
    if (Amps < 0.15) {
      Amps = 0;
    }

    Serial.print("Çekilen Anlık Akım: ");
    Serial.print(Amps);
    Serial.println(" Amper");

    // --- OTONOM DURUM ANALİZİ ---
    if (Amps > 0.8) {
      // Makine motoru veya rezistansı devredeyse
      washing_machine_status = "RUNNING"; 
    } 
    else if (Amps == 0 && washing_machine_status == "RUNNING") {
      // Makine daha önce çalışıyordu ama artık akım sıfıra düştüyse yıkama bitmiştir
      washing_machine_status = "FINISHED";
    }
    else if (Amps == 0) {
      // Makine tamamen kapalıysa
      washing_machine_status = "IDLE / READY";
    }
  }
}
