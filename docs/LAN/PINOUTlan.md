# LAN Pinout Configuration

| Pin Modul Ethernet | Pin ESP32-S3 | Keterangan |
| :--- | :--- | :--- |
| **GND** | GND | Ground |
| **VCC** | 3.3V | Power Supply (Do NOT use 5V) |
| **MOSI** | GPIO 11 | SPI Data Out |
| **MISO** | GPIO 13 | SPI Data In |
| **SCK** | GPIO 12 | SPI Clock |
| **CS** | GPIO 10 | Chip Select |
| **RST** | GPIO 18 | Reset Pin |
| **INT** | GPIO 9 | Interrupt Pin (Optional) |

---

### Implementation Notes:
- Controller: W5500 / ENC28J60 (Standard SPI Ethernet)
- Wiring must be kept short for stability at high SPI speeds.
