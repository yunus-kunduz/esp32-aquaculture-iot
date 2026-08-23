// Nextion Ekran (Nextion 2.4" Basic NX3224T024)
// Jumper Kablo (Erkek-Erkek)
// ESP32 NodeMCU
// Büyük Boy Breadboard

#include <WiFi.h>
#include <HTTPClient.h>
#include "EasyNextionLibrary.h" // Nextion ekranı kolayca okumak için kütüphane

// --- EVİNİZİN Wİ-Fİ BİLGİLERİNİ YAZIN ---
const char* ssid     = "EV_WIFI_ADINIZ";
const char* password = "EV_WIFI_SIFRENIZ";

// --- IFTTT WEBHOOKS BİLGİLERİNİZ ---
// (IFTTT sitesinden ücretsiz alacağımız gizli anahtarlar)
const char* event_name = "sos_alarm";
const char* ifttt_key  = "GIZLI_IFTTT_ANAHTARINIZ_BURAYA";

// Nextion nesnesini oluşturuyoruz. 
// ESP32'nin Donanımsal Seri Port 2'sini (Serial2) kullanıyoruz (Pins 16 ve 17)
EasyNex myNex(Serial2); 

void setup() 
{
  Serial.begin(115200);
  
  // Nextion ekran haberleşmesini 9600 hızında başlatıyoruz (Ekranların varsayılan hızıdır)
  myNex.begin(9600);

  // Wi-Fi Bağlantısını Başlat
  Serial.printf("Baglaniyor: %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Baglantisi Basarili! IoT Telefon Çevrimiçi.");
}

void loop() 
{
  // Arka planda Nextion ekrandan gelen dokunmatik verileri sürekli dinler
  myNex.NextionListen(); 
}

// --- TELEFONDAN ÇAĞRI/BİLDİRİM ATAN İNTERNET FONKSİYONU ---
void sendSosNotification() 
{
  if (WiFi.status() == WL_CONNECTED) 
  {
    HTTPClient http;
    
    // IFTTT bulut servisine istek atacağımız özel URL adresi
    String url = "http://ifttt.com" + String(event_name) + "/with/key/" + String(ifttt_key);
    
    Serial.println("[IoT TELEFON] Bulut sunucusuna SOS sinyali gönderiliyor...");
    http.begin(url);
    
    int httpResponseCode = http.GET(); // İnternet isteğini fırlat
    
    if (httpResponseCode > 0) 
    {
      Serial.print("[BAŞARILI] Sinyal ulaştı. Gerçek telefonunuz çalıyor! Kod: ");
      Serial.println(httpResponseCode);
    } 
    else 
    {
      Serial.print("[HATA] Sinyal gönderilemedi. Hata kodu: ");
      Serial.println(httpResponseCode);
    }
    http.end(); // Bağlantıyı güvenli şekilde kapat
  } 
  else 
  {
    Serial.println("[HATA] Wi-Fi bağlantısı koptuğu için arama yapılamadı!");
  }
}

// --- NEXTION TETİKLEME TETİĞİ (KRİTİK KISIM) ---
// Bir sonraki adımda Nextion Editor programında çizeceğimiz SOS butonuna 
// "trigger 50" (50 numaralı tetiği fırlat) emrini yazacağız. 
// Ekrandan o emir geldiğinde ESP32 otomatik olarak aşağıdaki bu fonksiyonu çalıştırır.
void trigger50() 
{
  Serial.println("\n[EKRAN UYARISI] Dokunmatik ekrandaki SOS butonuna basildi!");
  sendSosNotification(); // İnternet üzerinden gerçek telefonu ara/bildir
}
