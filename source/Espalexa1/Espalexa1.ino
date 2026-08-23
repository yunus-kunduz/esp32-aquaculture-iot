#include <WiFi.h>
#include <Espalexa.h>

// --- AĞ BİLGİLERİ ---
#define SSID "************"
#define SSID_PASSWORD "**********"

// --- RÖLE PİNLERİ ---
#define SW1_RELAY_PIN 18 
#define SW2_RELAY_PIN 23 
#define SW3_RELAY_PIN 13 
#define SW4_RELAY_PIN 33 

// --- BUTON PİNLERİ ---
#define button1 15
#define button2 5
#define button3 14
#define button4 25

Espalexa espalexa;


bool relay1State = false;
bool relay2State = false;
bool relay3State = false;
bool relay4State = false;


bool lastButton1 = LOW;
bool lastButton2 = LOW;
bool lastButton3 = LOW;
bool lastButton4 = LOW;

void setup() 
{
  Serial.begin(115200);

  pinMode(SW1_RELAY_PIN, OUTPUT);
  pinMode(SW2_RELAY_PIN, OUTPUT);
  pinMode(SW3_RELAY_PIN, OUTPUT);
  pinMode(SW4_RELAY_PIN, OUTPUT);

  pinMode(button1, INPUT_PULLDOWN);
  pinMode(button2, INPUT_PULLDOWN);
  pinMode(button3, INPUT_PULLDOWN);
  pinMode(button4, INPUT_PULLDOWN);

  
  digitalWrite(SW1_RELAY_PIN, LOW);
  digitalWrite(SW2_RELAY_PIN, LOW);
  digitalWrite(SW3_RELAY_PIN, LOW);
  digitalWrite(SW4_RELAY_PIN, LOW);

  
  WiFi.begin(SSID, SSID_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.println("WiFi'ye bağlanıyor...");
  }
  Serial.println("WiFi bağlandı!");
  Serial.println("IP: " + WiFi.localIP().toString());

  
  espalexa.addDevice("Birinci Röle", relay1Control);
  espalexa.addDevice("İkinci Röle", relay2Control);
  espalexa.addDevice("Üçüncü Röle", relay3Control);
  espalexa.addDevice("Dördüncü Röle", relay4Control);

  espalexa.begin();
  Serial.println("Alexa cihazları hazır!");
}


void relay1Control(uint8_t brightness) 
{
  relay1State = (brightness > 0);
  digitalWrite(SW1_RELAY_PIN, relay1State ?  HIGH : LOW);
  Serial.println("Röle 1: " + String(relay1State ?  "AÇIK" : "KAPALI"));
}

void relay2Control(uint8_t brightness) 
{
  relay2State = (brightness > 0);
  digitalWrite(SW2_RELAY_PIN, relay2State ?  HIGH : LOW);
  Serial.println("Röle 2: " + String(relay2State ?  "AÇIK" : "KAPALI"));
}

void relay3Control(uint8_t brightness) 
{
  relay3State = (brightness > 0);
  digitalWrite(SW3_RELAY_PIN, relay3State ?  HIGH : LOW);
  Serial.println("Röle 3: " + String(relay3State ?  "AÇIK" : "KAPALI"));
}

void relay4Control(uint8_t brightness) 
{
  relay4State = (brightness > 0);
  digitalWrite(SW4_RELAY_PIN, relay4State ?  HIGH : LOW);
  Serial.println("Röle 4: " + String(relay4State ?  "AÇIK" : "KAPALI"));
}

void loop() 
{
  espalexa.loop();

  bool button1Now = digitalRead(button1);
  if (button1Now == HIGH && lastButton1 == LOW) 
  {
    relay1State = !relay1State;
    digitalWrite(SW1_RELAY_PIN, relay1State ?  HIGH : LOW);
    Serial.println("Buton 1 - Röle 1: " + String(relay1State ?  "AÇIK" : "KAPALI"));
    delay(50);
  }
  lastButton1 = button1Now;

  bool button2Now = digitalRead(button2);
  if (button2Now == HIGH && lastButton2 == LOW) 
  {
    relay2State = !relay2State;
    digitalWrite(SW2_RELAY_PIN, relay2State ?  HIGH : LOW);
    Serial.println("Buton 2 - Röle 2: " + String(relay2State ?  "AÇIK" : "KAPALI"));
    delay(50);
  }
  lastButton2 = button2Now;

  bool button3Now = digitalRead(button3);
  if (button3Now == HIGH && lastButton3 == LOW) 
  {
    relay3State = !relay3State;
    digitalWrite(SW3_RELAY_PIN, relay3State ?  HIGH : LOW);
    Serial.println("Buton 3 - Röle 3: " + String(relay3State ?  "AÇIK" : "KAPALI"));
    delay(50);
  }
  lastButton3 = button3Now;

  bool button4Now = digitalRead(button4);
  if (button4Now == HIGH && lastButton4 == LOW) 
  {
    relay4State = !relay4State;
    digitalWrite(SW4_RELAY_PIN, relay4State ?  HIGH : LOW);
    Serial.println("Buton 4 - Röle 4: " + String(relay4State ?  "AÇIK" : "KAPALI"));
    delay(50);
  }
  lastButton4 = button4Now;
}