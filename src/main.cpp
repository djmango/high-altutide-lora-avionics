#define HELTEC_POWER_BUTTON // must be before "#include <heltec_unofficial.h>"

#include <Arduino.h>
#include <heltec_unofficial.h>

void setup() {
  heltec_setup();
  // Serial.begin(115200);

  delay(1000); // Give the serial connection time to start

  // Initialize OLED
  // u8g2.begin();
  // u8g2.setFont(u8g2_font_ncenB08_tr);

  Serial.println("Serial works");
  // Display
  display.println("Display works");
  // Radio
  display.print("Radio ");
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE) {
    display.println("works");
  } else {
    display.printf("fail, code: %i\n", state);
  }
  // Battery
  float vbat = heltec_vbat();
  display.printf("Vbat: %.2fV (%d%%)\n", vbat, heltec_battery_percent(vbat));

  Serial.println("Heltec WiFi LoRa 32 V3 Serial and OLED Echo");
  Serial.println("Type something and press enter...");
}

void loop() {
  heltec_loop();

  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    Serial.print("You typed: ");
    Serial.println(input);

    // Display on OLED
    display.println("You typed:");

    // Parse if it's a number and set the LED brightness
    if (input.length() > 0 && isdigit(input[0])) {
      int brightness = input.toInt();

      // Ensure the brightness is within the valid range (0-100)
      brightness = constrain(brightness, 0, 100);

      // Set LED brightness
      heltec_led(brightness);

      // Display the set brightness on OLED
      display.printf("LED: %d\n", brightness);
      Serial.println("LED brightness set to: " + String(brightness));
    } else {
      display.println("Not a number");
      Serial.println("Input is not a number");
    }
  }
}
