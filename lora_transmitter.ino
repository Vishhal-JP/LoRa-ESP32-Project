#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

LiquidCrystal_I2C lcd(0x26, 16, 2);

// LoRa
#define SS 5
#define RST 14
#define DIO0 26

// Sensors
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define MQ8_PIN 34
#define MOIST_PIN 32
#define VIB_PIN 35

// Outputs
#define GREEN_LED 25
#define RED_LED 33
#define BUZZER 17   // 🔥 updated

int screen = 0;

// Stable read
int readStable(int pin) {
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(pin);
    delay(5);
  }
  return sum / 10;
}

void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  // 🔹 Title Screen
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("TUNNEL SAFETY");

  lcd.setCursor(0,1);
  lcd.print("MONITORING");

  delay(2000);

  dht.begin();

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);

  LoRa.setPins(SS, RST, DIO0);
  LoRa.begin(433E6);

  // 🔹 Ready Screen (FIXED)
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("TX READY");

  delay(1500);
}

void loop() {

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  int g = readStable(MQ8_PIN);
  int m = readStable(MOIST_PIN);
  int v = readStable(VIB_PIN);

  String alertMsg = "SAFE";

  if (g > 1000)       alertMsg = "GAS HIGH";
  else if (m < 2000)  alertMsg = "WATER!";
  else if (v > 800)   alertMsg = "VIBRATION";

  bool isAlert = (alertMsg != "SAFE");

  // LED control
  digitalWrite(RED_LED, isAlert);
  digitalWrite(GREEN_LED, !isAlert);

  // 🔥 BUZZER CONTROL (SIMPLE & RELIABLE)
  if (isAlert) {
    digitalWrite(BUZZER, HIGH);   // ON
  } else {
    digitalWrite(BUZZER, LOW);    // OFF
  }

  // LCD
  // LCD
// LCD
lcd.clear();

if (isAlert) {
  lcd.setCursor(0,0);
  lcd.print("!!! ALERT !!!");

  lcd.setCursor(0,1);
  lcd.print(alertMsg);
} 
else {

  if (screen == 0) {   // 🔹 P1 → Temp + Hum

    lcd.setCursor(0,0);
    lcd.print("Temp:");
    lcd.print(t,1);

    lcd.setCursor(0,1);
    lcd.print("Hum :");
    lcd.print(h,1);

  } 
  else if (screen == 1) {   // 🔹 P2 → Gas + Moist

    lcd.setCursor(0,0);
    lcd.print("Gas  :");
    lcd.print(g);

    lcd.setCursor(0,1);
    lcd.print("Moist:");
    lcd.print(m);

  } 
  else {   // 🔹 P3 → Vibration

    lcd.setCursor(0,0);
    lcd.print("Vibration");

    lcd.setCursor(0,1);
    lcd.print("Val:");
    lcd.print(v);

  }

  screen = (screen + 1) % 3;   // 🔥 3 screens
}

  // Send
  String data = String(t,1) + "," + String(h,1) + "," +
                String(g) + "," + String(m) + "," + String(v);

  LoRa.beginPacket();
  LoRa.print(data);
  LoRa.endPacket();

  delay(1500);
}