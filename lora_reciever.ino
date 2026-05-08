#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ✅ Updated address
LiquidCrystal_I2C lcd(0x27, 16, 2);

// LoRa pins
#define SS 5
#define RST 14
#define DIO0 26

// Outputs
#define GREEN_LED 25
#define RED_LED 27
#define BUZZER 17

int screen = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Default SAFE
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);

  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(433E6)) {
    lcd.setCursor(0,0);
    lcd.print("LoRa Failed");
    while (1);
  }

  Serial.println("RX READY");

  lcd.setCursor(0,0);
  lcd.print("RX READY");
  delay(1500);
}

void loop() {

  int packetSize = LoRa.parsePacket();

  if (packetSize) {

    String data = "";

    while (LoRa.available()) {
      data += (char)LoRa.read();
    }

    Serial.println("Received: " + data);

    float t = 0, h = 0;
    int g = 0, m = 0, v = 0;

    sscanf(data.c_str(), "%f,%f,%d,%d,%d", &t, &h, &g, &m, &v);

    // 🔥 ALERT LOGIC
    String alertMsg = "SAFE";

    if (g > 1000)       alertMsg = "GAS HIGH";
    else if (m < 2000)  alertMsg = "WATER!";
    else if (v > 800)   alertMsg = "VIBRATION";

    bool isAlert = (alertMsg != "SAFE");

    // 🔴 LED + BUZZER CONTROL
    digitalWrite(RED_LED, isAlert);
    digitalWrite(GREEN_LED, !isAlert);

    if (isAlert) {
      digitalWrite(BUZZER, HIGH);
    } else {
      digitalWrite(BUZZER, LOW);
    }

    // 📺 LCD DISPLAY
    lcd.clear();

    if (isAlert) {
      lcd.setCursor(0,0);
      lcd.print("!!! ALERT !!!");

      lcd.setCursor(0,1);
      lcd.print(alertMsg);
    }
    else {
      if (screen == 0) {
        lcd.setCursor(0,0);
        lcd.print("T:");
        lcd.print(t,1);
        lcd.print(" H:");
        lcd.print(h,1);

        lcd.setCursor(0,1);
        lcd.print("G:");
        lcd.print(g);
      }
      else {
        lcd.setCursor(0,0);
        lcd.print("Moist:");
        lcd.print(m);

        lcd.setCursor(0,1);
        lcd.print("Vib:");
        lcd.print(v);
      }

      screen = (screen + 1) % 2;
    }

    delay(1500);
  }
}