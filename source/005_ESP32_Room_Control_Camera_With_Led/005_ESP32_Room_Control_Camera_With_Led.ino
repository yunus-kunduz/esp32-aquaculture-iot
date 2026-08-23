#include "esp_camera.h"
#include <WiFi.h>

// --- AYARLAR ---
const char* ssid = "EV_WIFI_ADINIZ";
const char* password = "EV_WIFI_SIFRENIZ";
#define RELAY_PIN 12 // Kamera ile uyumlu güvenli pin

// ... (Kamera pin tanımlamaları, AI-Thinker standart)
WiFiServer server(80);

void startCameraServer();

// ... (Kamera kurulum kodları, esp_camera_init() vb.)

void handleWebClient() {
  WiFiClient client = server.available();
  if (!client) return;
  String request = client.readStringUntil('\r');
  if (request.indexOf("/LIGHT=ON") != -1) digitalWrite(RELAY_PIN, HIGH);
  if (request.indexOf("/LIGHT=OFF") != -1) digitalWrite(RELAY_PIN, LOW);
  
  // Basit HTML arayüzü ile ışık kontrolü
  client.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n");
  client.println("<html><body><h1>ESP32-CAM Işık Kontrol</h1>");
  client.println("<a href=\"/LIGHT=ON\"><button>ON</button></a>");
  client.println("<a href=\"/LIGHT=OFF\"><button>OFF</button></a></body></html>");
}

void setup() {
  // ... (WIFI bağlantısı ve setup kodları)
  pinMode(RELAY_PIN, OUTPUT);
  startCameraServer(); // Video akışı
  server.begin();      // Buton kontrolü
}

void loop() {
  handleWebClient();
}
