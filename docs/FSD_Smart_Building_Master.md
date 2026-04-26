# ⚡ SmartBuilding Master S3 — HMI Central

📌 **Project Overview**

Embedded system berbasis **ESP32-S3** yang berfungsi sebagai unit sentral kendali gedung (*Smart Building Controller*), mengintegrasikan:

* 🖥️ **HMI Display** — TFT Parallel 3.5" ILI9488 dengan Touch
* 📡 **MQTT over WiFi & LAN** — Dual-network connectivity (W5500 Ethernet)
* 🧠 **Dual-Core RTOS** — Pemisahan beban UI dan Network
* 🔄 **OTA Firmware Update** — Via Dashboard (Roadmap)

---

## 🏗️ System Architecture

```
                ┌────────────────────────────────┐
                │           ESP32-S3              │
                │       Arduino / IDF Framework   │
                │                                 │
                │  CORE 0                         │
                │  ┌──────────────────────────┐   │
                │  │   WiFi Manager           │   │
                │  │   LAN Manager (W5500)    │   │
                │  │   MQTT Manager           │   │
                │  │   Simulation Engine      │   │
                │  └──────────┬───────────────┘   │
                │             │ Global State       │
                │  CORE 1     ▼                   │
                │  ┌──────────────────────────┐   │
                │  │   Task_UI (Render)       │   │
                │  │   Task_Touch (Polling)   │   │
                │  │   Task_Touch (Handler)   │   │
                │  └──────────┬───────────────┘   │
                │             │                   │
                └─────────────┼───────────────────┘
                              │
               ┌──────────────┼──────────────┐
               ▼                             ▼
      ILI9488 3.5" TFT                  W5500 Ethernet
      (Parallel 8-bit + ADC Touch)       (SPI2)
               │
               ▼
         WiFi Router / LAN Switch
               │
               ▼
         MQTT Broker (EMQX)
```

---

## 🗂️ Project Structure (Actual)

```
smartbuilding-s3/
│
├── platformio.ini           # Build config & TFT pin definitions
├── src/
│   ├── main.cpp             # RTOS Task orchestration (UI, Net, Touch)
│   ├── data.cpp / .h        # Global state (BuildingState) & Mutex
│   ├── display.cpp / .h     # TFT init & bus_mutex
│   ├── touch.cpp / .h       # Resistive touch handling (shared bus)
│   ├── mqtt_manager.cpp / .h# MQTT client (SSL/Non-SSL)
│   ├── wifi_manager.cpp / .h# WiFi connectivity & scanning
│   ├── lan_manager.cpp / .h # W5500 Ethernet manager
│   ├── time_manager.cpp / .h# NTP time synchronization
│   ├── ui_screens.cpp / .h  # Screen rendering & navigation
│   ├── ui_widgets.cpp / .h  # Widget library (cards, graphs, buttons)
│   └── ui_keyboard.cpp / .h # Virtual QWERTY keyboard
│
└── docs/
    └── FSD_Smart_Building_Master.md
```

---

## 📖 Functional Specification (FSD) Detail

### 1. Hardware Pin Mapping

| Feature               | Function         | ESP32-S3 Pin | Notes                          |
| :-------------------- | :--------------- | :----------- | :----------------------------- |
| **TFT Parallel 8-bit**| D0 - D5          | 1, 2, 3, 4, 5, 6 | Data Bus                   |
|                       | **D6 / TOUCH_XP**| **7**        | ⚠️ Shared (TFT Data & ADC Touch)|
|                       | **D7 / TOUCH_YM**| **8**        | ⚠️ Shared (TFT Data & ADC Touch)|
|                       | **WR / TOUCH_YP**| **15**       | ⚠️ Shared (TFT Control & Touch)|
|                       | **RS / TOUCH_XM**| **16**       | ⚠️ Shared (TFT Control & Touch)|
|                       | CS / RD / RST    | 17, 14, 48   | TFT Control                    |
| **Ethernet (W5500)**  | SCK, MISO, MOSI  | 12, 13, 11   | SPI2_HOST                      |
|                       | CS, RST          | 10, 18       | W5500 Control                  |

### 2. Software Requirements (Rules)

#### ✅ DO (Harus Dilakukan)
1. **SHALL** gunakan `vTaskDelay(pdMS_TO_TICKS(1))` di setiap loop task untuk jaga watchdog.
2. **SHALL** panggil `tft.endWrite()` sebelum membaca ADC Touch (Shared Bus Rule).
3. **SHALL** inisialisasi `last_data_ts` dengan `millis()` saat startup.
4. **SHALL** alokasikan stack `Task_UI` minimal **16KB**.

#### ❌ DON'T (Jangan Dilakukan)
1. **SHALL NOT** gunakan `delay()` di task manapun.
2. **SHALL NOT** tahan `data_lock` terlalu lama (terutama saat Network I/O).
3. **SHALL NOT** render langsung ke layar — wajib melalui Sprite buffer (PSRAM).
4. **SHALL NOT** abaikan error `DMA not supported`; gunakan CPU transfer optimal.

### 3. Screen & Data State Machine

- **Screen**: Dashboard ↔ Settings ↔ WiFi/LAN Config ↔ Keyboard.
- **Data Timeout**: Jika data MQTT > 10 detik ga ada update, sensor otomatis reset ke `-100` (NUL).
- **Network Priority**: NVS-based switching antara WiFi (Priority 0) dan LAN (Priority 1).

---

## 🗺️ Roadmap
- [ ] Implementasi Real-time WiFi Scanning Logic
- [ ] Integrasi Modbus/RS485 untuk komunikasi Slave fisik
- [ ] Penambahan fitur OTA via Web Dashboard
- [ ] Optimasi Bus Speed (overclocking parallel timing)

---
*Documented by Antigravity Master Agent based on User's High-End Structure*
