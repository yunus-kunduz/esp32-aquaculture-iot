// ESP32 Geliştirme Kartı (ESP32 NodeMCU / ESP-WROOM-32)
// 5V 1 Kanal Röle Modülü
// Büyük Boy Breadboard (830 Delikli)
// Dişi-Erkek Jumper Kablo Seti
// Evdeki Wi-Fi Modemi (İnternet)

#include <WiFi.h>
#include "time.h" // ESP32'nin dahili zaman kütüphanesi

// --- EVİNİZİN Wİ-Fİ BİLGİLERİNİ YAZIN ---
const char* ssid     = "EV_WIFI_ADINIZ";
const char* password = "EV_WIFI_SIFRENIZ";

// --- RÖLE PİN TANIMLAMASI ---
#define RELAY_PIN 18 

// NTP Saat Sunucusu Ayarları
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3 * 3600; // Türkiye için UTC+3 saat dilimi ayarı (3 saat * 3600 saniye)
const int   daylightOffset_sec = 0;   // Türkiye'de sabit saat uygulaması olduğu için 0

void checkAndControlAutomation() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Kritik Hata: İnternetten saat verisi alinamadi!");
    return;
  }
  
  // Anlık saati ve tarihi seri port ekranına yazdır (Takip için)
  Serial.print("Mevcut Zaman: ");
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  int currentHour = timeinfo.tm_hour; // Anlık saati al (0-23 arası)

  // --- OTONOM ZAMAN KONTROLÜ ---
  // Saat 19:00 ile 23:00 arasındaysa lambayı (röleyi) aç
  // Saat tam 23:00 olduğunda kapanması için currentHour < 23 şartı eklenmiştir.
  if (currentHour >= 19 && currentHour < 23) {
    digitalWrite(RELAY_PIN, HIGH); // Röleyi AÇ (Lamba yanar)
    Serial.println("[OTONOM DURUM] Saat 19:00 - 23:00 araligindayiz. Lamba: AÇIK");
  } else {
    digitalWrite(RELAY_PIN, LOW);  // Röleyi KAPAT (Lamba söner)
    Serial.println("[OTONOM DURUM] Belirlenen zaman dilimi disindayiz. Lamba: KAPALI");
  }
}

void setup() {
  Serial.begin(115200);
  
  // Röle pini çıkış olarak ayarlanıyor
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // İlk başlangıçta güvenli mod: Kapalı

  // İnternet (Wi-Fi) Bağlantısı Başlatılıyor
  Serial.printf("Baglanti kuruluyor: %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Baglantisi Basarili! Cihaz internete bağlandi.");

  // İnternet üzerinden atomik saat sunucusuyla senkronizasyon başlatılıyor
  Serial.println("NTP sunucusundan saat senkronize ediliyor...");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  // Her 5 saniyede bir saati kontrol et ve röleyi yönet (Sistemi yormamak için)
  delay(5000);
  checkAndControlAutomation(); 
}
