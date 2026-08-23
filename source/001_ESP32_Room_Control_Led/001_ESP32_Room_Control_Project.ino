// esp32 geliştirme kartı
// 5v 1 kanal röle modülü
// breadboard
// jumper kablo seti
// push button
// micro usb kablo
/*1. ESP32 ve Buton Bağlantısı
ESP32'yi Yerleştirin: ESP32 kartınızı breadboard'un
tam ortasındaki uzun yarığa denk gelecek şekilde bastırarak oturtun.
Sol bacaklar sol tarafta, sağ bacaklar sağ tarafta kalmalı.
Butonu Takın: 4 bacaklı push butonu breadboard üzerinde boş bir yere
takın. Buton Kablosu 1: Bir ucu Erkek-Erkek olan jumper kablonun bir ucunu
butonun herhangi bir bacağına, diğer ucunu ESP32 üzerindeki GND (Toprak) pinine
bağlayın. Buton Kablosu 2: İkinci bir Erkek-Erkek kabloyu butonun tam karşısındaki
bacağına takın, diğer ucunu ise ESP32 üzerindeki GPIO 15 (D15) pinine bağlayın.

2. Röle Modülü BağlantısıSatın alacağınız 5V röle modülünün üzerinde genellikle 
3 adet erkek pin bulunur: VCC (veya +, V), GND (veya -, G) ve IN (veya S, Sinyal).
Bunları bağlamak için Dişi-Erkek jumper kablo kullanacağız:Güç Kablosu (VCC): Kablonun 
dişi ucunu rölenin VCC pinine, erkek ucunu ESP32'nin 5V (veya VIN) pinine bağlayın. 
(Bu sayede röle elektriğini bilgisayardan, yani ESP32 üzerinden alacak).
Toprak Kablosu (GND): Kablonun dişi ucunu rölenin GND pinine, erkek ucunu ESP32 üzerindeki
boşta kalan diğer bir GND pinine bağlayın.Sinyal Kablosu (IN): Kablonun dişi ucunu rölenin 
IN pinine, erkek ucunu ise ESP32'nin GPIO 18 (D18) pinine bağlayın.
*/

// --- TÜRKİYE'DEN ALINAN STANDART PARÇALARIN PİN TANIMLAMALARI ---
#define RELAY_PIN 18   // 5V 1 Kanal Röle Modülünün bağlı olduğu pin (IN)
#define BUTTON_PIN 15  // 4 Bacaklı Push Butonun bağlı olduğu pin

// Sistemin durumunu hafızada tutacak değişkenler
bool lastButtonState = HIGH; // Butonun bir önceki döngüdeki durumu
bool relayState = LOW;       // Rölenin anlık durumu (Başlangıçta kapalı/LOW)

void setup() {
  // Bilgisayarla haberleşmek için seri portu başlatıyoruz
  Serial.begin(115200);

  // Röle pinini çıkış (OUTPUT) olarak ayarlıyoruz
  pinMode(RELAY_PIN, OUTPUT); 
  digitalWrite(RELAY_PIN, relayState); // İlk başlangıçta röleyi kapalı yap

  // Buton pinini giriş (INPUT) olarak ayarlıyoruz. 
  // INPUT_PULLUP sayesinde harici direnç bağlamadan pini HIGH seviyesinde tutuyoruz.
  pinMode(BUTTON_PIN, INPUT_PULLUP); 
  
  Serial.println("Sistem baslatildi. Butona basarak roleyi kontrol edebilirsiniz.");
}

void loop() {
  // Butonun anlık lojik durumunu (HIGH veya LOW) okuyoruz
  bool currentButtonState = digitalRead(BUTTON_PIN);

  // EĞER butona yeni basıldıysa (Durum HIGH'dan LOW'a düştüyse)
  if (currentButtonState == LOW && lastButtonState == HIGH) 
  {
    
    relayState = !relayState; // Röle durumunu tersine çevir (Açıksa kapat, kapalıysa aç)
    digitalWrite(RELAY_PIN, relayState); // Yeni durumu röleye fiziksel olarak uygula
    
    // Bilgisayar ekranına (Seri Port Ekranı) durum raporu yazdırıyoruz
    if (relayState == HIGH) {
      Serial.println("Butona Basildi -> Röle AÇILDI [Çıt Sesi Geldi + Röle LED'i Yandı]");
    } else {
      Serial.println("Butona Basildi -> Röle KAPATILDI [Röle LED'i Söndü]");
    }
    
    // Buton arkını (sinyal titreşimini) önlemek için 200 milisaniye bekletiyoruz
    delay(200); 
  }

  // Bu döngüdeki buton durumunu, bir sonraki döngüde karşılaştırmak için eski durum olarak kaydet
  lastButtonState = currentButtonState;
}
