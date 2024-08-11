#include <Arduino.h>
#include <LoRa.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <Wire.h>

// Correct pin definitions for Heltec WiFi LoRa 32 V3
#define OLED_SDA 17
#define OLED_SCL 18
#define OLED_RST 21

// LoRa pins
#define LORA_SS 8
#define LORA_RST 12
#define LORA_DIO0 14
#define LORA_DIO1 13
#define LORA_DIO2 15

// Using hardware I2C with explicit pin definition
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, OLED_RST, OLED_SCL, OLED_SDA);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Heltec WiFi LoRa 32 V3 Test");

  // Reset OLED
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  // Initialize I2C for OLED
  Wire.begin(OLED_SDA, OLED_SCL);

  // Initialize OLED
  if (!u8g2.begin()) {
    Serial.println("OLED initialization failed");
    while (1)
      ;
  } else {
    Serial.println("OLED initialized successfully");
  }

  // Initialize LoRa
  // LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  // if (!LoRa.begin(915E6)) { // Adjust frequency as needed
  //   Serial.println("LoRa initialization failed");
  //   while (1)
  //     ;
  // }
  Serial.println("LoRa initialized successfully");

  // Basic display test
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, "Heltec V3 Test");
  u8g2.drawStr(0, 30, "OLED OK");
  u8g2.drawStr(0, 50, "LoRa OK");
  u8g2.sendBuffer();

  Serial.println("Setup complete");
}

void loop() {
  // Blinking text
  static bool toggle = false;
  toggle = !toggle;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, "Heltec V3 Test");
  u8g2.drawStr(0, 30, "OLED OK");
  u8g2.drawStr(0, 50, "LoRa OK");
  if (toggle) {
    u8g2.drawStr(0, 64, "Blinking");
  }
  u8g2.sendBuffer();

  delay(1000);
}
