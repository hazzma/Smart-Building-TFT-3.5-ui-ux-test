# LAN Agent - Ethernet & Local Networking Agent

Focuses on stable wired connectivity using SPI Ethernet modules (W5500/ENC28J60).

## Hardware Configuration (W5500/ENC28J60)
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

## Workflow Steps

### 1. SPI LAN Initialization
- Use `SPIClass` to define a custom SPI bus on GPIO 11, 12, 13.
- Initialize the Ethernet library (e.g., `Ethernet.h` for W5500).
- Set manual Reset pulse for the module before calling `Ethernet.begin()`.

### 2. Status Monitoring
- Poll `Ethernet.linkStatus()` to update `g_state.net.lan_connected`.
- Ensure fallback logic if both WiFi and LAN are available.

### 3. Debugging
- Check SPI clock speed (reduce to 10-20MHz if data is corrupt).
- Verify voltage levels (ESP32-S3 is 3.3V tolerant).
