# Inertia LoRa Firmware

## Hardware Required
- ESP32 Dev Board
- LoRa module (SX1278 / SX1276 / RA-02)
- Antenna (868/915 MHz)

## Wiring

| LoRa Module | ESP32 |
|-------------|-------|
| VCC | 3.3V |
| GND | GND |
| NSS | GPIO5 |
| SCK | GPIO18 |
| MOSI | GPIO23 |
| MISO | GPIO19 |
| RST | GPIO14 |
| DIO0 | GPIO26 |

## Flashing

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Add ESP32 board: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3. Install LoRa library: `Tools → Manage Libraries → LoRa by Sandeep Mistry`
4. Select board: `ESP32 Dev Module`
5. Open `inertia_lora.ino`
6. Adjust frequency for your region:
   - Europe: `#define FREQUENCY 868E6`
   - USA: `#define FREQUENCY 915E6`
   - Asia: `#define FREQUENCY 433E6`
7. Upload and open Serial Monitor (115200 baud)

## Testing

Send AT commands via Serial Monitor:
AT+SEND=1,13,Hello Inertia!
AT+STATUS

## Integration with Web Interface

1. Flash ESP32
2. Connect via USB
3. Open Inertia web app
4. Click "LORA CONNECT"
5. Select USB port
6. Ready! 🚀
