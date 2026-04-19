/*
  INERTIA LoRa Firmware v1.0
  ESP32 + SX1278/SX1276 (RA-02)
  Works with Inertia Web Serial Bridge
  
  In Physics We Trust. 🌌
*/

#include <SPI.h>
#include <LoRa.h>

// ============================================================
//  CONFIGURATION - ADJUST FOR YOUR REGION AND HARDWARE
// ============================================================

// LoRa pins for ESP32 (standard wiring)
#define SS_PIN     5
#define RST_PIN    14
#define DIO0_PIN   26

// Frequency: 868E6 (Europe), 915E6 (USA/Canada/Australia), 433E6 (Asia)
#define FREQUENCY       868E6

// Spreading Factor (SF7-SF12, higher = more range, lower speed)
#define SPREADING_FACTOR 9

// Transmission power (2-20 dBm)
#define TX_POWER        20

// Serial baud rate (must match Web Serial)
#define SERIAL_BAUD     115200

// Beacon interval (milliseconds, 0 to disable)
#define BEACON_INTERVAL 30000

// ============================================================
//  END OF CONFIGURATION
// ============================================================

unsigned long lastBeacon = 0;
unsigned long lastStatus = 0;

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1000);
  
  Serial.println("\n\n╔════════════════════════════════════════════════════════════╗");
  Serial.println("║                    ◉ INERTIA LoRa Node                     ║");
  Serial.println("║                    v1.0 | Post-Internet Protocol           ║");
  Serial.println("║                  In Physics We Trust. 🌌                    ║");
  Serial.println("╚════════════════════════════════════════════════════════════╝");
  
  // Initialize LoRa
  LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);
  
  Serial.print("📡 Initializing LoRa at ");
  Serial.print(FREQUENCY / 1E6);
  Serial.println(" MHz...");
  
  if (!LoRa.begin(FREQUENCY)) {
    Serial.println("❌ LoRa init FAILED!");
    Serial.println("   Check wiring:");
    Serial.println("   LoRa → ESP32");
    Serial.println("   VCC  → 3.3V");
    Serial.println("   GND  → GND");
    Serial.println("   NSS  → GPIO5");
    Serial.println("   SCK  → GPIO18");
    Serial.println("   MOSI → GPIO23");
    Serial.println("   MISO → GPIO19");
    Serial.println("   RST  → GPIO14");
    Serial.println("   DIO0 → GPIO26");
    while (1) {
      delay(1000);
      Serial.print(".");
    }
  }
  
  // Configure LoRa
  LoRa.setSpreadingFactor(SPREADING_FACTOR);
  LoRa.setTxPower(TX_POWER);
  LoRa.setCodingRate4(5);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setSyncWord(0x12);  // Inertia sync word (avoid interference)
  
  Serial.println("✅ LoRa initialized successfully!");
  Serial.print("   Frequency: "); Serial.print(FREQUENCY / 1E6); Serial.println(" MHz");
  Serial.print("   Spreading Factor: SF"); Serial.println(SPREADING_FACTOR);
  Serial.print("   TX Power: "); Serial.print(TX_POWER); Serial.println(" dBm");
  Serial.print("   Sync Word: 0x12\n");
  
  Serial.println("💡 Commands:");
  Serial.println("   AT+SEND=<len>,<data>  - Send LoRa packet");
  Serial.println("   AT+FREQ=<hz>          - Change frequency");
  Serial.println("   AT+SF=<7-12>          - Change spreading factor");
  Serial.println("   AT+POWER=<2-20>       - Change TX power");
  Serial.println("   AT+STATUS             - Show status");
  Serial.println("   AT+BEACON=<ms>        - Set beacon interval");
  Serial.println("");
  Serial.println("🔗 Ready for Web Serial Bridge connection.\n");
}

void loop() {
  // ========== RECEIVE LORA PACKETS ==========
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }
    
    int rssi = LoRa.packetRssi();
    float snr = LoRa.packetSnr();
    
    // Log to Serial (for Web Serial Bridge)
    Serial.print("📥 RX: ");
    Serial.print(incoming);
    Serial.print(" | RSSI: ");
    Serial.print(rssi);
    Serial.print(" dBm | SNR: ");
    Serial.print(snr);
    Serial.println(" dB");
    
    // Format for Web Serial Bridge parser
    Serial.print("RX:");
    Serial.println(incoming);
  }
  
  // ========== PROCESS AT COMMANDS ==========
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();
    
    // AT+SEND=1,5,hello
    if (cmd.startsWith("AT+SEND=")) {
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
      } else {
        Serial.println("ERROR: Invalid format. Use AT+SEND=1,<len>,<data>");
      }
    }
    
    // AT+FREQ=868000000
    else if (cmd.startsWith("AT+FREQ=")) {
      long freq = cmd.substring(8).toInt();
      if (freq > 0 && freq < 1000000000) {
        LoRa.setFrequency(freq);
        Serial.print("OK | Frequency set to ");
        Serial.print(freq / 1E6);
        Serial.println(" MHz");
      } else {
        Serial.println("ERROR: Invalid frequency");
      }
    }
    
    // AT+SF=9
    else if (cmd.startsWith("AT+SF=")) {
      int sf = cmd.substring(5).toInt();
      if (sf >= 6 && sf <= 12) {
        LoRa.setSpreadingFactor(sf);
        Serial.print("OK | Spreading Factor set to SF");
        Serial.println(sf);
      } else {
        Serial.println("ERROR: SF must be 6-12");
      }
    }
    
    // AT+POWER=20
    else if (cmd.startsWith("AT+POWER=")) {
      int pwr = cmd.substring(9).toInt();
      if (pwr >= 2 && pwr <= 20) {
        LoRa.setTxPower(pwr);
        Serial.print("OK | TX Power set to ");
        Serial.print(pwr);
        Serial.println(" dBm");
      } else {
        Serial.println("ERROR: Power must be 2-20 dBm");
      }
    }
    
    // AT+STATUS
    else if (cmd.startsWith("AT+STATUS")) {
      Serial.println("=== INERTIA LoRa Node Status ===");
      Serial.print("Frequency: "); Serial.print(LoRa.getFrequency() / 1E6); Serial.println(" MHz");
      Serial.print("Spreading Factor: SF"); Serial.println(LoRa.getSpreadingFactor());
      Serial.print("TX Power: "); Serial.print(LoRa.getTxPower()); Serial.println(" dBm");
      Serial.print("Bandwidth: "); Serial.print(LoRa.getSignalBandwidth() / 1000); Serial.println(" kHz");
      Serial.print("Coding Rate: 4/"); Serial.println(LoRa.getCodingRate4());
      Serial.print("Sync Word: 0x"); Serial.println(LoRa.getSyncWord(), HEX);
      Serial.print("Uptime: "); Serial.print(millis() / 1000); Serial.println(" seconds");
      Serial.println("================================");
    }
    
    // AT+BEACON=30000
    else if (cmd.startsWith("AT+BEACON=")) {
      unsigned long interval = cmd.substring(10).toInt();
      extern unsigned long BEACON_INTERVAL;
      const_cast<unsigned long&>(BEACON_INTERVAL) = interval;
      Serial.print("OK | Beacon interval set to ");
      Serial.print(interval);
      Serial.println(" ms");
    }
    
    // Unknown command
    else if (cmd.length() > 0 && !cmd.startsWith("AT")) {
      // Ignore non-AT commands (like raw data)
    }
    else if (cmd.length() > 0) {
      Serial.println("ERROR: Unknown command");
    }
  }
  
  // ========== PERIODIC BEACON ==========
  if (BEACON_INTERVAL > 0 && millis() - lastBeacon > BEACON_INTERVAL) {
    lastBeacon = millis();
    String beacon = "INERTIA:beacon:" + String(millis());
    LoRa.beginPacket();
    LoRa.print(beacon);
    LoRa.endPacket();
    Serial.print("📤 BCN: ");
    Serial.println(beacon);
  }
  
  // ========== PERIODIC STATUS (every 60 seconds) ==========
  if (millis() - lastStatus > 60000) {
    lastStatus = millis();
    Serial.print("💚 Heartbeat | RSSI: ");
    Serial.print(LoRa.packetRssi());
    Serial.println(" dBm");
  }
}
