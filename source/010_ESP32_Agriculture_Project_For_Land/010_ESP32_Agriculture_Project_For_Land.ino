// Toprak Nem Sensörü Modülü (Kapasitif v1.2)
// Yağmur Sensörü (Rain Sensor)
// 12V Solenoid Vana (Su Vanası)
// 5V 1 Kanal Röle Modülü
// ESP32 NodeMCU
// Breadboard
// Jumper Kablolar

#include <WiFi.h>
#include <ThingerESP32.h>

// --- TAŞINABİLİR Wİ-Fİ ROUTER (MODEM) BİLGİLERİNİZ ---
#define SSID "TASINABILIR_WIFI_ADINIZ"
#define SSID_PASSWORD "TASINABILIR_WIFI_SIFRENIZ"

// --- THINGER.IO KİMLİK BİLGİLERİ ---
#define USERNAME "Yunus_Kunduz"               
#define DEVICE_ID "esp32_role_kartim"         
#define DEVICE_CREDENTIAL "esp32_role_kartim" 

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

// --- PİN TANIMLAMALARI ---
#define SOIL_SENSOR_PIN   34  // Kapasitif Toprak Nem Sensörü (Analog Giriş)
#define RAIN_SENSOR_PIN   25  // Yağmur Sensörü Dijital Çıkışı (DO -> Dijital Giriş)
#define VALVE_RELAY_PIN   18  // 12V Solenoid Vana Kontrol Rölesi (Dijital Çıkış)

// --- TOPRAK SENSÖRÜ KALİBRASYON DEĞERLERİ ---
const int AIR_VALUE = 3200;   // Tamamen Kuru Toprak (Havadaki Değer)
const int WATER_VALUE = 1300; // Tamamen Islak Toprak (Sudaki Değer)

// Küresel Durum Değişkenleri
int soil_moisture_percent = 0;
String rain_status = "Hava Açık / Yağmursuz";
String valve_status = "KAPALI";

void setup() 
{
  Serial.begin(115200);
  
  // Pin modlarının ayarlanması
  pinMode(VALVE_RELAY_PIN, OUTPUT);
  digitalWrite(VALVE_RELAY_PIN, LOW); // Başlangıçta vana kapalı (Güvenli mod)
  
  pinMode(RAIN_SENSOR_PIN, INPUT); // Yağmur sensörünü giriş olarak tanımla

  // Thinger.io Wi-Fi Bağlantısını Başlat
  thing.add_wifi(SSID, SSID_PASSWORD);

  // --- THINGER.IO BULUT PANEL ENTEGRASYONU ---
  // Tarladan uzaktayken telefon ekranınızda göreceğiniz tüm canlı veriler
  thing["field_telemetry"] >> [](pson& out)
  {
    out["soil_moisture"] = soil_moisture_percent; // Toprak nem yüzdesi (%0 - %100)
    out["weather_info"] = rain_status;           // Yağmur durumu (Yazı)
    out["irrigation_valve"] = valve_status;      // Vananın anlık durumu (Yazı)
  };
}

void loop() 
{
  // Arka planda taşınabilir modemi ve bulut sunucusunu dinler
  thing.handle();

  // Her 3 saniyede bir tarladaki sensörleri oku ve durumu analiz et
  static unsigned long last_check_time = 0;
  if (millis() - last_check_time > 3000) 
  {
    last_check_time = millis();

    // 1. ADIM: TOPRAK NEMİNİ OKUMA VE YÜZDEYE ÇEVİRME
    int raw_analog_value = analogRead(SOIL_SENSOR_PIN);
    soil_moisture_percent = map(raw_analog_value, AIR_VALUE, WATER_VALUE, 0, 100);
    
    // Sınır taşma koruması
    if(soil_moisture_percent < 0) soil_moisture_percent = 0;
    if(soil_moisture_percent > 100) soil_moisture_percent = 100;

    // 2. ADIM: YAĞMUR DURUMUNU OKUMA
    // Yağmur sensörleri dijital çıkışta genellikle TERS çalışır. 
    // Üzerine su damladığında LOW (0) verir, kuru olduğunda HIGH (1) verir.
    bool is_raining = (digitalRead(RAIN_SENSOR_PIN) == LOW); 
    
    if (is_raining) 
    {
      rain_status = "YAĞMUR YAĞIYOR!";
    } 
    else 
    {
      rain_status = "Hava Açık / Yağmursuz";
    }

    // 3. ADIM: AKILLI OTONOM SULAMA KARAR MEKANIZMASI (EDGE LOGIC)
    // Kural: Toprak %35'ten kuruysa VE yağmur yağmıyorsa vanayı aç
    if (soil_moisture_percent < 35 && !is_raining) 
    {
      digitalWrite(VALVE_RELAY_PIN, HIGH); // Röleyi tetikle, 12V vanayı aç
      valve_status = "SULAMA YAPIYOR (AÇIK)";
      Serial.println("[TARLA OTOMASYONU] Toprak kuru ve hava temiz. Vana açildi.");
    } 
    // Toprak %75 doygunluğa ulaştıysa VEYA o sırada yağmur başladıysa sulamayı derhal kes
    else if (soil_moisture_percent >= 75 || is_raining) 
    {
      digitalWrite(VALVE_RELAY_PIN, LOW); // Röleyi kapat, vanayı kilitle
      
      if(is_raining && soil_moisture_percent < 35) 
      {
        Serial.println("[TARLA OTOMASYONU] Toprak kuru ama yagmur basladi, su israfini onlemek icin vana kapatildi.");
      } 
      else 
      {
        Serial.println("[TARLA OTOMASYONU] Toprak neme doydu. Vana kapatildi.");
      } 
      
      valve_status = "KAPALI";
    }

    // Bilgisayardan yerel takip için seri porta yazdır
    Serial.printf("Nem: %d%% | Hava: %s | Vana: %s\n", soil_moisture_percent, rain_status.c_str(), valve_status.c_str());
  }
}

