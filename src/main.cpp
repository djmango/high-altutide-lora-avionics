#define HELTEC_POWER_BUTTON // must be before "#include <heltec_unofficial.h>"

#include <Arduino.h>
#include <SparkFunBME280.h>
#include <SparkFun_ENS160.h>
#include <Wire.h>
#include <heltec_unofficial.h>
// https://github.com/ropg/heltec_esp32_lora_v3

// Uncomment this line to use the board as a transmitter
// Comment it out to use as a receiver
// #define TRANSMITTER_MODE

// LoRa parameters
#define FREQUENCY 915.0
#define BANDWIDTH 125.0
#define SPREADING_FACTOR 7
#define CODING_RATE 5
#define TX_POWER 10

unsigned long lastDisplayUpdate = 0;
const unsigned long displayUpdateInterval =
    30000; // Update display every 30 seconds
unsigned long lastTransmitTime = 0;
const unsigned long transmitInterval = 5000; // 5 seconds

BME280 myBME;
SparkFun_ENS160 myENS;

String createSensorPacket() {
  String packet = "";

  // BME280 data
  packet += "T:" + String(myBME.readTempC(), 1) + "C,";
  packet += "P:" + String(myBME.readFloatPressure() / 100.0F, 1) + "hPa,";
  packet += "H:" + String(myBME.readFloatHumidity(), 1) + "%,";

  // ENS160 data
  if (myENS.checkDataStatus()) {
    packet += "AQI:" + String(myENS.getAQI()) + ",";
    packet += "TVOC:" + String(myENS.getTVOC()) + "ppb,";
    packet += "CO2:" + String(myENS.getECO2()) + "ppm";
  } else {
    packet += "ENS:NoData";
  }

  return packet;
}

void transmitterLoop() {
  if (millis() - lastTransmitTime > transmitInterval) {
    String packet = createSensorPacket();
    Serial.print("Sending packet: ");
    Serial.println(packet);

    int state = radio.transmit(packet);

    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "Transmitting:");
    display.drawString(
        0, 16, packet.substring(0, 20) + "..."); // Display first 20 chars

    if (state == RADIOLIB_ERR_NONE) {
      Serial.println("Packet sent successfully!");
      display.drawString(0, 32, "Sent successfully!");
    } else {
      Serial.print("Failed to send packet, code ");
      Serial.println(state);
      display.drawString(0, 32, "Send failed, code " + String(state));
    }

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

  // Wire.begin();

  Serial.println("I2C initialized");

  // Scan for I2C devices
  Serial.println("Scanning for I2C devices...");
  byte error, address;
  int nDevices = 0;
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
      nDevices++;
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found");
  } else {
    Serial.println("I2C scan complete");
  }

  // Initialize BME280
  if (myBME.beginI2C() == false) {
    Serial.println("BME280 not detected. Please check wiring.");
    while (1)
      ;
  } else {
    Serial.println("BME280 initialized successfully");
  }

  // Initialize ENS160
  if (!myENS.begin()) {
    Serial.println("Could not communicate with the ENS160, check wiring.");
    while (1)
      ;
  } else {
    Serial.println("ENS160 initialized successfully");
  }

  Serial.println("Both sensors initialized successfully.");

  // Reset the ENS160 sensor's settings
  if (myENS.setOperatingMode(SFE_ENS160_RESET)) {
    Serial.println("ENS160 reset complete.");
  }
  delay(100);

  // Set ENS160 to standard operation
  myENS.setOperatingMode(SFE_ENS160_STANDARD);

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
