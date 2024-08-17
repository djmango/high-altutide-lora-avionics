#define HELTEC_POWER_BUTTON // must be before "#include <heltec_unofficial.h>"

#include <Arduino.h>
#include <SparkFun_BMI270_Arduino_Library.h>
#include <Wire.h>
#include <heltec_unofficial.h>
// https://github.com/ropg/heltec_esp32_lora_v3

// Uncomment this line to use the board as a transmitter
// Comment it out to use as a receiver
#define TRANSMITTER_MODE

// LoRa parameters
#define FREQUENCY 915.0
#define BANDWIDTH 125.0
#define SPREADING_FACTOR 7
#define CODING_RATE 5
#define TX_POWER 10

BMI270 imu;

// I2C address selection
// uint8_t i2cAddress = BMI2_I2C_PRIM_ADDR; // 0x68
uint8_t i2cAddress = BMI2_I2C_SEC_ADDR; // 0x69

unsigned long lastDisplayUpdate = 0;
const unsigned long displayUpdateInterval =
    30000; // Update display every 30 seconds

void transmitterLoop() {
  static unsigned long lastTransmitTime = 0;
  const unsigned long transmitInterval = 500; // 500 ms

  if (millis() - lastTransmitTime > transmitInterval) {
    String packet = "Packet #" + String(millis() / 1000);
    Serial.print("Sending packet: ");
    Serial.println(packet);

    int state = radio.transmit(packet);

    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "Transmitting:");
    display.drawString(0, 16, packet);

    if (state == RADIOLIB_ERR_NONE) {
      Serial.println("Packet sent successfully!");
      display.drawString(0, 32, "Sent successfully!");
    } else {
      Serial.print("Failed to send packet, code ");
      Serial.println(state);
      display.drawString(0, 32, "Send failed, code " + String(state));
    }

    // Read accelerometer data
    imu.getSensorData();

    Serial.print("Accel X: ");
    Serial.print(imu.data.accelX);
    Serial.print(" Y: ");
    Serial.print(imu.data.accelY);
    Serial.print(" Z: ");
    Serial.println(imu.data.accelZ);

    display.display();
    lastTransmitTime = millis();
  }
}

void receiverLoop() {
  String str;
  int state = radio.receive(str);

  if (state == RADIOLIB_ERR_NONE) {
    // Packet received successfully
    Serial.println("Received packet!");
    Serial.print("Data: ");
    Serial.println(str);

    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "Received:");
    display.drawString(0, 16, str);
    display.drawString(0, 32, "RSSI: " + String(radio.getRSSI()) + " dBm");
    display.drawString(0, 48, "SNR: " + String(radio.getSNR()) + " dB");
    display.display();

    lastDisplayUpdate = millis(); // Reset the display update timer
  } else if (state == RADIOLIB_ERR_RX_TIMEOUT) {
    // No packet received, this is normal
    // Update display periodically to show it's still listening
    if (millis() - lastDisplayUpdate > displayUpdateInterval) {
      display.clear();
      display.setFont(ArialMT_Plain_10);
      display.setTextAlignment(TEXT_ALIGN_CENTER);
      display.drawString(64, 0, "Receiver Mode");
      display.drawString(64, 16, "Listening...");
      display.drawString(64, 32, "Time: " + String(millis() / 1000) + "s");
      display.display();
      lastDisplayUpdate = millis();
    }
  } else {
    // Some other error occurred
    Serial.print("Failed to receive packet, code ");
    Serial.println(state);

    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "Receive Error:");
    display.drawString(0, 16, "Code: " + String(state));
    display.display();

    lastDisplayUpdate = millis(); // Reset the display update timer
  }

  // Start receiving again
  radio.startReceive();
}

void setup() {
  heltec_setup();
  Serial.println("Heltec board initialized");

  // Wire.begin(4, 5); // SDA = GPIO4, SCL = GPIO5
  // Wire.begin(); // SDA = GPIO4, SCL = GPIO5
  //    while(imu.beginI2C(i2cAddress) != BMI2_OK)

  // // if (imu.beginI2C() != 0) {
  //   Serial.println("Error: BMI270 not detected");
  //   Serial.println(imu.beginI2C());
  //   while (1)
  //     ;
  // }
  //
  // pinMode(RST_OLED, OUTPUT);
  // digitalWrite(RST_OLED, LOW);
  // delay(50);
  // digitalWrite(RST_OLED, HIGH);
  // Wire.begin(SDA_OLED, SCL_OLED); // Scan OLED's I2C address via I2C0

  Wire.begin();

  while (imu.beginI2C(i2cAddress) != BMI2_OK) {
    // Not connected, inform user
    Serial.println(
        "Error: BMI270 not connected, check wiring and I2C address!");

    // Wait a bit to see if connection is established
    delay(1000);
  }
  Serial.println("BMI270 connected!");

  // Initialize LoRa
  Serial.print("Initializing LoRa: ");
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("success!");
  } else {
    Serial.print("failed, code ");
    Serial.println(state);
    while (true)
      ;
  }

  // Set up LoRa parameters
  RADIOLIB_OR_HALT(radio.setFrequency(FREQUENCY));
  RADIOLIB_OR_HALT(radio.setBandwidth(BANDWIDTH));
  RADIOLIB_OR_HALT(radio.setSpreadingFactor(SPREADING_FACTOR));
  RADIOLIB_OR_HALT(radio.setCodingRate(CODING_RATE));
  RADIOLIB_OR_HALT(radio.setOutputPower(TX_POWER));

  Serial.println("LoRa initialized successfully!");

  // Display mode on OLED
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 22,
#ifdef TRANSMITTER_MODE
                     "Transmitter Mode"
#else
                     "Receiver Mode"
#endif
  );
  display.display();
  delay(2000);

#ifndef TRANSMITTER_MODE
  // Start listening for LoRa packets
  radio.startReceive();
  Serial.println("Started listening for LoRa packets");
#endif
}

void loop() {
  heltec_loop();

#ifdef TRANSMITTER_MODE
  transmitterLoop();
#else
  receiverLoop();
#endif
}
