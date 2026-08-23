#include <ThingerESP32.h>

#define USERNAME "*********"
#define DEVICE_ID "*********"
#define DEVICE_CREDENTIAL "*********"

#define SSID "*********"
#define SSID_PASSWORD "*********"

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

#define SW1_RELAY_PIN 18
#define SW2_RELAY_PIN 23
#define SW3_RELAY_PIN 13
#define SW4_RELAY_PIN 33

#define button1 15
#define button2 5
#define button3 14
#define button4 25

bool state1 = false;
bool state2 = false;
bool state3 = false;
bool state4 = false;

bool lastPhys1 = LOW;
bool lastPhys2 = LOW;
bool lastPhys3 = LOW;
bool lastPhys4 = LOW;

void setup() 
{
  Serial.begin(115200);

  pinMode(SW1_RELAY_PIN, OUTPUT);
  pinMode(SW2_RELAY_PIN, OUTPUT);
  pinMode(SW3_RELAY_PIN, OUTPUT);
  pinMode(SW4_RELAY_PIN, OUTPUT);

  digitalWrite(SW1_RELAY_PIN, LOW);
  digitalWrite(SW2_RELAY_PIN, LOW);
  digitalWrite(SW3_RELAY_PIN, LOW);
  digitalWrite(SW4_RELAY_PIN, LOW);

  pinMode(button1, INPUT_PULLDOWN);
  pinMode(button2, INPUT_PULLDOWN);
  pinMode(button3, INPUT_PULLDOWN);
  pinMode(button4, INPUT_PULLDOWN);

  thing.add_wifi(SSID, SSID_PASSWORD);

  thing["switch1"] << [](pson& in) 
  {
    if(in.is_empty()) 
    {
      in = state1;
    } else {
      state1 = in;
      digitalWrite(SW1_RELAY_PIN, state1 ? HIGH : LOW);
    }
  };

  thing["switch2"] << [](pson& in) 
  {
    if(in.is_empty()) 
    {
      in = state2;
    } else {
      state2 = in;
      digitalWrite(SW2_RELAY_PIN, state2 ? HIGH : LOW);
    }
  };

  thing["switch3"] << [](pson& in) 
  {
    if(in.is_empty()) 
    {
      in = state3;
    } else {
      state3 = in;
      digitalWrite(SW3_RELAY_PIN, state3 ? HIGH : LOW);
    }
  };

  thing["switch4"] << [](pson& in) 
  {
    if(in.is_empty()) 
    {
      in = state4;
    } else {
      state4 = in;
      digitalWrite(SW4_RELAY_PIN, state4 ? HIGH : LOW);
    }
  };
}

void loop() 
{
  thing.handle();

  bool phys1 = digitalRead(button1);
  bool phys2 = digitalRead(button2);
  bool phys3 = digitalRead(button3);
  bool phys4 = digitalRead(button4);

  if (phys1 == HIGH && lastPhys1 == LOW) 
  {
    state1 = !state1;
    digitalWrite(SW1_RELAY_PIN, state1 ? HIGH : LOW);
    thing.stream("switch1");
    delay(200);
  }
  lastPhys1 = phys1;

  if (phys2 == HIGH && lastPhys2 == LOW) 
  {
    state2 = !state2;
    digitalWrite(SW2_RELAY_PIN, state2 ? HIGH : LOW);
    thing.stream("switch2");
    delay(200);
  }
  lastPhys2 = phys2;

  if (phys3 == HIGH && lastPhys3 == LOW) 
  {
    state3 = !state3;
    digitalWrite(SW3_RELAY_PIN, state3 ? HIGH : LOW);
    thing.stream("switch3");
    delay(200);
  }
  
  lastPhys3 = phys3;

  if (phys4 == HIGH && lastPhys4 == LOW) 
  {
    state4 = !state4;
    digitalWrite(SW4_RELAY_PIN, state4 ? HIGH : LOW);
    thing.stream("switch4");
    delay(200);
  }
  lastPhys4 = phys4;
}