#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// Initialize the OLED display
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 18, /* data=*/ 17, /* reset=*/ 21);

void setup() {
  Serial.begin(115200);
  delay(1000);  // Give the serial connection time to start

  // Initialize OLED
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);

  Serial.println("Heltec WiFi LoRa 32 V3 Serial and OLED Echo");
  Serial.println("Type something and press enter...");

  // Display initial message on OLED
  u8g2.clearBuffer();
  u8g2.drawStr(0, 10, "Heltec WiFi LoRa 32");
  u8g2.drawStr(0, 25, "Type in serial...");
  u8g2.sendBuffer();
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    Serial.print("You typed: ");
    Serial.println(input);

    // Display on OLED
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "You typed:");
    u8g2.drawStr(0, 25, input.c_str());
    u8g2.sendBuffer();
  }
}
