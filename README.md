# ESP32S3 Master Smartbuilding

Smart Building Master unit based on **ESP32-S3 (N16R8)** with a **3.5" Parallel TFT Display**. This device serves as the central HMI for building control and monitoring.

## 📄 Documentation
- **Functional Specification Document (FSD)**: [FSD_Smart_Building_Master.md](docs/FSD_Smart_Building_Master.md)


## 🛠 Current Working Features (Checkpoint 1)
- **TFT Display**: 480x320 Parallel 8-bit interface driven by `TFT_eSPI`.
- **UI Architecture**: Double-buffered sprite system for flicker-free rendering.
- **Networking**:
    - **WiFi**: Manual SSID/Password input, NVS credential persistence, and Auto-reconnect. Default SSID: "han".
    - **LAN**: Support for W5500 via SPI2 with DHCP and Static IP configuration.
    - **Network Priority**: Dynamic switching between WiFi and Ethernet.
- **UI Screens**:
    - **Dashboard**: Real-time sensor monitoring (temperature, CO2, Lux) and device control (AC, Projector, Lights).
    - **WiFi Setup**: Advanced setup with Reconnect and Scan UI.
    - **LAN Setup**: DHCP/Static IP configuration manager.
    - **Virtual Keyboard**: Custom touch-based QWERTY keyboard for data entry.
- **System**:
    - Dual-core RTOS implementation (Core 0: Network, Core 1: UI).
    - Global state management with Mutex protection.

## 🚀 Planned Fixes & Optimizations
The system is currently undergoing performance tuning to improve the frame rate:
- **Touch Decoupling**: Moving resistive touch polling to a dedicated RTOS task to prevent blocking the rendering pipeline.
- **Bus Mutex**: Implementing a hardware-level semaphore to safely manage the shared parallel bus between the Display and Touch controller.
- **Conditional Rendering**: Implementing a "Dirty Flag" system to bypass redundant frame pushes when UI state is static.
- **WiFi Scan Logic**: Implementing actual asynchronous network scanning to replace current UI placeholders.

## 📋 Hardware Specification & Pinout

### MCU & Core
- **MCU**: ESP32-S3 (16MB Flash, 8MB PSRAM).
- **Architecture**: Dual-core RTOS (Core 0: Network, Core 1: UI).

### Pin Mapping Table

| Feature | Function | ESP32-S3 Pin | Notes |
| :--- | :--- | :--- | :--- |
| **TFT Parallel 8-bit** | D0 | 1 | |
| | D1 | 2 | |
| | D2 | 3 | |
| | D3 | 4 | |
| | D4 | 5 | |
| | D5 | 6 | |
| | **D6 / TOUCH_XP** | **7** | Shared with Touch |
| | **D7 / TOUCH_YM** | **8** | Shared with Touch |
| | **WR / TOUCH_YP** | **15** | Shared with Touch |
| | **RS (DC) / TOUCH_XM**| **16** | Shared with Touch |
| | CS | 17 | |
| | RD | 14 | |
| | RST | 48 | |
| **Ethernet (W5500)** | CS | 10 | SPI2_HOST |
| | MOSI | 11 | SPI2_HOST |
| | SCK | 12 | SPI2_HOST |
| | MISO | 13 | SPI2_HOST |
| | RST | 18 | |

---
*Checkpoint 1.1 - Final Optimized State*
