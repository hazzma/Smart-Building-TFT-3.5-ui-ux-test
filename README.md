# ESP32S3 Master Smartbuilding

Smart Building Master unit based on **ESP32-S3 (N16R8)** with a **3.5" Parallel TFT Display**. This device serves as the central HMI for building control and monitoring.

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

## 📋 Hardware Specification
- **MCU**: ESP32-S3 (16MB Flash, 8MB PSRAM).
- **Display**: 3.5" Parallel TFT (ILI9488).
- **Touch**: Resistive 4-wire (Shared Pins with TFT Bus).
- **Ethernet**: W5500 (SPI2).
