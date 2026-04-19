/*
  INERTIA LoRa Firmware v1.0
  ESP32 + SX1278/SX1276
  Works with Inertia Web Serial Bridge
*/

#include <SPI.h>
#include <LoRa.h>

// ============ CONFIGURATION ============
#define SS 5
#define RST 14
#define DIO0 26

// Frequency: 868E6 (EU) / 915E6 (US) / 433E6 (ASIA)
#define FREQUENCY 868E6

#define SPREADING_FACTOR 9
#define TX_POWER 20        // 2-20 dBm
#define SERIAL_BAUD 115200
// ========================================

unsigned long lastBeacon = 0;
const unsigned long BEACON_INTERVAL = 30000; // 30 seconds

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println("\n\n◉ INERTIA LoRa Node v1.0");
  Serial.println("In Physics We Trust. 🌌");
  
  // Initialize LoRa
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(FREQUENCY)) {
    Serial.println("❌ LoRa init failed! Check wiring.");
    while (1);
  }
  
  LoRa.setSpreadingFactor(SPREADING_FACTOR);
  LoRa.setTxPower(TX_POWER);
  LoRa.setCodingRate4(5);
  LoRa.setSignalBandwidth(125E3);
  
  Serial.println("✅ LoRa ready");
  Serial.print("📡 Frequency: "); Serial.println(FREQUENCY / 1E6);
  Serial.print("⚡ Spreading Factor: SF"); Serial.println(SPREADING_FACTOR);
  Serial.println("💡 Send AT commands via Serial Monitor");
}

void loop() {
  // Handle incoming packets
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }
    
    int rssi = LoRa.packetRssi();
    float snr = LoRa.packetSnr();
    
    Serial.print("📥 RX: ");
    Serial.print(incoming);
    Serial.print(" | RSSI: ");
    Serial.print(rssi);
    Serial.print(" dBm | SNR: ");
    Serial.println(snr);
    
    // Echo back for Web Serial bridge
    Serial.print("RX:");
    Serial.println(incoming);
  }
  
  // Handle AT commands from Serial (Web Serial Bridge)
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    if (cmd.startsWith("AT+SEND=")) {
      // Format: AT+SEND=1,5,hello
      int firstComma = cmd.indexOf(',');
      int secondComma = cmd.indexOf(',', firstComma + 1);
      
      if (firstComma > 0 && secondComma > 0) {
        String payload = cmd.substring(secondComma + 1);
        
        LoRa.beginPacket();
        LoRa.print(payload);
        LoRa.endPacket();
        
        Serial.println("OK");
        Serial.print("📤 TX: ");
        Serial.println(payload);
      }
    } 
    else if (cmd.startsWith("AT+FREQ=")) {
      long freq = cmd.substring(8).toInt();
      if (freq > 0) {
        LoRa.setFrequency(freq);
        Serial.println("OK");
      }
    }
    else if (cmd.startsWith("AT+SF=")) {
      int sf = cmd.substring(5).toInt();
      if (sf >= 6 && sf <= 12) {
        LoRa.setSpreadingFactor(sf);
        Serial.println("OK");
      }
    }
    else if (cmd.startsWith("AT+POWER=")) {
      int pwr = cmd.substring(9).toInt();
      if (pwr >= 2 && pwr <= 20) {
        LoRa.setTxPower(pwr);
        Serial.println("OK");
      }
    }
    else if (cmd.startsWith("AT+STATUS")) {
      Serial.print("Freq: "); Serial.print(LoRa.getFrequency() / 1E6);
      Serial.print(" MHz | SF: "); Serial.print(LoRa.getSpreadingFactor());
      Serial.print(" | Power: "); Serial.println(LoRa.getTxPower());
    }
  }
  
  // Periodic beacon
  if (millis() - lastBeacon > BEACON_INTERVAL) {
    lastBeacon = millis();
    String beacon = "INERTIA:beacon:" + String(millis());
    LoRa.beginPacket();
    LoRa.print(beacon);
    LoRa.endPacket();
    Serial.print("📤 BCN: ");
    Serial.println(beacon);
  }
}
