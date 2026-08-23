#include "esp_camera.h"
#include <WiFi.h>

const char* ssid = "EV_WIFI_ADINIZ";
const char* password = "EV_WIFI_SIFRENIZ";

#define GREEN_LED 12
#define YELLOW_LED 16

WiFiServer server(80);
void startCameraServer(); // Standart kamera yayını arka planda başlar (Port: 81)

void setup() {
  Serial.begin(115200);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  
  // Başlangıçta sarı yansın (Odada henüz insan tespiti yok)
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, HIGH);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  
  startCameraServer(); // Video yayını http://IP_ADRESINIZ:81 üzerinden akacak
  server.begin();      // Komut alma sunucusu (Port: 80)
}

void loop() {
  WiFiClient client = server.available();
  if (!client) return;
  
  String request = client.readStringUntil('\r');
  
  // Python yapay zekasından gelen komutları işle
  if (request.indexOf("/HUMAN=YES") != -1) {
    digitalWrite(GREEN_LED, HIGH);  // İnsan var -> Yeşil Aç
    digitalWrite(YELLOW_LED, LOW);   // Sarı Kapat
  }
  if (request.indexOf("/HUMAN=NO") != -1) {
    digitalWrite(GREEN_LED, LOW);   // İnsan yok -> Yeşil Kapat
    digitalWrite(YELLOW_LED, HIGH);  // Sarı Aç
  }
  client.println("HTTP/1.1 200 OK\n\n");
}

