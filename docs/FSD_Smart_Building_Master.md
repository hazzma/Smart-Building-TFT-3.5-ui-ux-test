# Functional Specification Document (FSD)

## Project: Smart Building Master S3 (HMI Central Unit)

---

## Document Information

| Field        | Value                                  |
| ------------ | -------------------------------------- |
| Version      | 1.1                                    |
| Status       | Active Development                     |
| Checkpoint   | Checkpoint 1.1 — Optimized             |
| Framework    | Arduino (PlatformIO)                   |
| Target MCU   | ESP32-S3 N16R8                         |
| Display      | ILI9488 3.5" (480x320, Parallel 8-bit) |
| Connectivity | WiFi + Ethernet (W5500)                |
| Protocol     | MQTT (SSL/Non-SSL), EMQX Broker        |

---

## 1. Purpose

Firmware ini mengimplementasikan:

1. HMI (Human-Machine Interface) berbasis layar TFT 3.5" untuk monitoring gedung.
2. Koneksi MQTT dual-interface (WiFi dan LAN via W5500) untuk terima data sensor.
3. Manajemen jaringan (WiFi Scan, Reconnect, LAN Config, NTP Time Sync).
4. Arsitektur Dual-Core RTOS untuk memisahkan beban rendering UI dan networking.

Sistem dirancang modular, dengan pemisahan tanggung jawab yang ketat antar modul.

---

## 1.2 Design Philosophy

- Rendering UI tidak boleh diblokir oleh proses networking.
- Akses ke state bersama (`g_state`) wajib dijaga dengan Mutex.
- Akses ke bus hardware bersama (TFT + Touch) wajib dijaga dengan `bus_mutex`.
- Data sensor yang sudah kedaluwarsa (>10 detik) harus di-reset ke nilai NULL.

Engineering rule:

> Jika koneksi MQTT terputus, UI harus tetap responsif dan menampilkan status error yang jelas.

---

## 2. System Block Diagram

```
  ┌───────────────────────────────────────────────────────────┐
  │                   Smart Building Master S3                │
  │                                                           │
  │  ┌─────────────────────────┐   ┌───────────────────────┐ │
  │  │     CORE 0 (Network)    │   │    CORE 1 (Display)   │ │
  │  │                         │   │                        │ │
  │  │  ┌─────────────────┐   │   │  ┌──────────────────┐ │ │
  │  │  │   WiFi Manager  │   │   │  │  Task_UI         │ │ │
  │  │  └────────┬────────┘   │   │  │  (Sprite Render) │ │ │
  │  │           │            │   │  └────────┬─────────┘ │ │
  │  │  ┌────────▼────────┐   │   │           │           │ │
  │  │  │   LAN Manager   │   │   │  ┌────────▼─────────┐ │ │
  │  │  │   (W5500/SPI2)  │   │   │  │  Task_Touch      │ │ │
  │  │  └────────┬────────┘   │   │  │  (ADC Resistive) │ │ │
  │  │           │            │   │  └────────┬─────────┘ │ │
  │  │  ┌────────▼────────┐   │   │           │           │ │
  │  │  │  MQTT Manager   │◄──┼───┼───────────┘           │ │
  │  │  │  (PubSubClient) │   │   │                        │ │
  │  │  └────────┬────────┘   │   └───────────────────────┘ │
  │  │           │            │                              │
  │  │  ┌────────▼────────┐   │   ┌───────────────────────┐ │
  │  │  │  Time Manager   │   │   │  Concurrency Layer     │ │
  │  │  │  (NTP Sync)     │   │   │  ┌────────────────┐   │ │
  │  │  └─────────────────┘   │   │  │  bus_mutex     │   │ │
  │  │                         │   │  │  (TFT & Touch) │   │ │
  │  └─────────────┬───────────┘   │  ├────────────────┤   │ │
  │                │               │  │  data_mutex    │   │ │
  │  ┌─────────────▼───────────┐   │  │  (g_state)     │   │ │
  │  │  Global State           │   │  └────────────────┘   │ │
  │  │  BuildingState g_state  │◄──┼───────────────────────┘ │
  │  │  (Sensor, Net, UI Flag) │   │                         │
  │  └─────────────────────────┘   │                         │
  │                                                           │
  └──────────────┬──────────────────────────────┬────────────┘
                 │                              │
     ┌───────────▼──────────┐      ┌───────────▼──────────┐
     │   ILI9488 TFT 3.5"   │      │   W5500 Ethernet     │
     │   480x320 Parallel   │      │   SPI2_HOST          │
     │   8-bit Bus          │      │   DHCP / Static IP   │
     └──────────────────────┘      └──────────────────────┘
                 │
     ┌───────────▼──────────┐
     │  Resistive Touch     │
     │  (Shared Bus Pins)   │
     │  ADC 10-bit          │
     └──────────────────────┘
```

---

## 3. Data Flow Diagram

```
  MQTT Broker (EMQX)
         │
         │ topic: binus/ayam
         │ payload: { temperature, lux, co2 }
         ▼
  mqtt_callback()
         │  data_lock(g_state)
         │  g_state.sensor.temp[0] = ...
         │  g_state.sensor.lux = ...
         │  g_state.last_data_ts = millis()
         │  g_state.ui_needs_update = true
         │  data_unlock(g_state)
         ▼
  g_state (Global BuildingState)
         │
         │  Task_UI polls ui_needs_update flag
         ▼
  screens_render(g_state)
         │
         │  canvas.pushSprite(0, 0)
         ▼
  ILI9488 Display (480x320)
```

---

## 4. Hardware Specification

### 4.1 MCU

| Field     | Value              |
| --------- | ------------------ |
| SoC       | ESP32-S3           |
| Flash     | 16 MB (QIO)        |
| PSRAM     | 8 MB (OPI)         |
| CPU Speed | 240 MHz            |
| RTOS      | FreeRTOS (ESP-IDF) |

### 4.2 Pin Mapping Table

| Feature                | Function          | ESP32-S3 Pin | Notes                            |
| :--------------------- | :---------------- | :----------- | :------------------------------- |
| **TFT Parallel 8-bit** | D0                | 1            | Data Bus                         |
|                        | D1                | 2            |                                  |
|                        | D2                | 3            |                                  |
|                        | D3                | 4            |                                  |
|                        | D4                | 5            |                                  |
|                        | D5                | 6            |                                  |
|                        | **D6 / TOUCH_XP** | **7**        | ⚠️ Shared (TFT Data & ADC Touch) |
|                        | **D7 / TOUCH_YM** | **8**        | ⚠️ Shared (TFT Data & ADC Touch) |
|                        | **WR / TOUCH_YP** | **15**       | ⚠️ Shared (TFT Control & Touch)  |
|                        | **RS / TOUCH_XM** | **16**       | ⚠️ Shared (TFT Control & Touch)  |
|                        | CS                | 17           | TFT Chip Select                  |
|                        | RD                | 14           | TFT Read                         |
|                        | RST               | 48           | TFT Reset                        |
| **Ethernet (W5500)**   | SCK               | 12           | SPI2_HOST                        |
|                        | MISO              | 13           | SPI2_HOST                        |
|                        | MOSI              | 11           | SPI2_HOST                        |
|                        | CS                | 10           | W5500 Chip Select                |
|                        | RST               | 18           | W5500 Reset                      |

> ⚠️ **Critical**: Pin 7, 8, 15, 16 di-share antara TFT Parallel Bus dan Resistive Touch ADC. Akses ke pin ini WAJIB diproteksi oleh `bus_mutex`.

---

## 5. Software Architecture

### 5.1 File Structure

```
Master S3/
│
├── platformio.ini           # Build config, TFT pin definitions
├── README.md
│
├── src/
│   ├── main.cpp             # RTOS Task orchestration & setup
│   ├── data.h / data.cpp    # Global state (BuildingState), Mutex
│   ├── display.h / display.cpp  # TFT init, bus_mutex declaration
│   ├── touch.h / touch.cpp  # Resistive touch (shared bus, ADC)
│   ├── mqtt_manager.h / .cpp# MQTT pub/sub (SSL WiFi, non-SSL LAN)
│   ├── wifi_manager.h / .cpp# WiFi connect/reconnect/scan
│   ├── lan_manager.h / .cpp # W5500 Ethernet (DHCP/Static)
│   ├── time_manager.h / .cpp# NTP synchronization
│   ├── ui_screens.h / .cpp  # Screen rendering & navigation
│   ├── ui_widgets.h / .cpp  # Widget library (cards, buttons)
│   └── ui_keyboard.h / .cpp # Virtual QWERTY Keyboard
│
├── lib/
│   └── S3_Parallel_Touch/   # Custom lib: resistive touch on shared bus
│
└── docs/
    ├── FSD_Smart_Building_Master.md   # This document
    └── LAN/
        └── PINOUTlan.md
```

### 5.2 RTOS Task Allocation

| Task         | Core | Priority | Stack | Responsibility                        |
| :----------- | :--- | :------- | :---- | :------------------------------------ |
| `Task_Net`   | 0    | 1        | 8 KB  | WiFi, LAN, MQTT, NTP, Data Timeout    |
| `Task_LAN`   | 0    | 1        | 4 KB  | Ethernet init & link monitoring       |
| `Task_Touch` | 1    | 1        | 4 KB  | Resistive touch polling (20ms / 50Hz) |
| `Task_UI`    | 1    | 4        | 16 KB | Sprite render + display push          |

### 5.3 Screen State Machine

```
  [BOOT / data_init]
         │
         ▼
  SCREEN_DASHBOARD  ◄──────────────────────────────┐
         │                                          │
         │ [Hamburger Menu]                         │
         ▼                                          │
  SCREEN_SETTINGS                                   │
         │                                          │
    ┌────┴────┐                                     │
    ▼         ▼                                     │
SCREEN_      SCREEN_                                │
WIFI_CONFIG  LAN_CONFIG                             │
    │         │                                     │
    │         │ [BACK]─────────────────────────────►┤
    │                                               │
    │ [SCAN]                                        │
    ▼                                               │
SCREEN_WIFI_SCAN ──── [BACK] ──────────────────────►┤
    │                                               │
    │ [select SSID → Keyboard opens]                │
    ▼                                               │
SCREEN_KEYBOARD ──── [OK / CANCEL] ─────────────────┘
```

### 5.4 WiFi State Machine

```
  WIFI_INIT
     │
     ▼
  WIFI_CONNECTING ──── timeout/error ──► WIFI_RETRY
     │                                       │
     ▼                                       │
  WIFI_CONNECTED ◄───────────────────────────┘
     │
     ├── MQTT connect attempt
     ├── NTP sync
     └── Normal operation
```

### 5.5 Network Priority Logic

```
  NVS: net_priority = 0 (WiFi) | 1 (LAN)
         │
    ┌────┴────┐
    ▼         ▼
  WiFi ON   WiFi OFF
  LAN OFF   LAN ON (W5500)
    │         │
    └────┬────┘
         ▼
    MQTT connect
    (SSL via WiFi OR non-SSL via LAN)
```

---

## 6. Functional Requirements

### 6.1 Display & UI

**DISP-001**
System SHALL use PSRAM Double Buffer (Sprite) for all rendering.

**DISP-002**
System SHALL only push frame to display when `g_state.ui_needs_update == true`.

**DISP-003**
System SHALL force-refresh every 2000ms for clock and status bar updates.

**DISP-004**
Screen transition SHALL set `ui_needs_update = true` immediately upon call.

**DISP-005**
All touch events SHALL set `ui_needs_update = true` to trigger immediate re-render.

---

### 6.2 Touch

**TOUCH-001**
Touch polling SHALL occur every 20ms (50Hz) in a dedicated `Task_Touch`.

**TOUCH-002**
Before any ADC read, system SHALL call `tft.endWrite()` to release the shared bus.

**TOUCH-003**
After ADC read, system SHALL call `tft.startWrite()` to restore display bus.

**TOUCH-004**
All touch reads SHALL be protected by `xSemaphoreTake(bus_mutex, 20ms)`.

**TOUCH-005**
Debounce SHALL be enforced at 80ms minimum between valid touch events.

---

### 6.3 Network

**NET-001**
System SHALL support dynamic priority switching between WiFi and LAN without reboot.

**NET-002**
WiFi credentials SHALL be persisted in NVS namespace `"wifi_cfg"`.
Default SSID: `han`, Default Password: `hanhanhan`.

**NET-003**
If no MQTT data received for >10 seconds, all sensor values SHALL reset to NULL.
(`temp = -100.0f`, `lux = -1.0f`, `co2 = -1`)

**NET-004**
MQTT SHALL use SSL (port 8883) over WiFi, and non-SSL (port 1883) over LAN.

**NET-005**
WiFi Scan page SHALL display available SSIDs without blocking the render pipeline.

---

### 6.4 Concurrency

**CONC-001**
All access to `g_state` SHALL use `data_lock(g_state)` and `data_unlock(g_state)`.

**CONC-002**
Lock duration SHALL be minimized. No I/O, delay, or network call inside a locked block.

**CONC-003**
Access to TFT parallel bus (render + touch) SHALL be protected by `bus_mutex`.

---

## 7. Rules: Do's and Don'ts

### ✅ DO

| #   | Rule                                                                                                           |
| --- | -------------------------------------------------------------------------------------------------------------- |
| 1   | **SHALL** use `vTaskDelay(pdMS_TO_TICKS(N))` in every task loop.                                               |
| 2   | **SHALL** call `tft.endWrite()` before ADC touch read.                                                         |
| 3   | **SHALL** initialize `last_data_ts = millis()` on boot.                                                        |
| 4   | **SHALL** keep Task_UI stack at minimum 16KB.                                                                  |
| 5   | **SHALL** set `ui_needs_update = true` on every state change and touch event.                                  |
| 6   | **SHALL** use unified `data_lock` block — read, modify, unlock in one block.                                   |
| 7   | **SHALL** log `[NET] Alive` heartbeat every 5 seconds to Serial.                                               |
| 8   | **SHALL** use `Serial.printf()` format for all debug logs with module prefix, e.g. `[WIFI]`, `[MQTT]`, `[TC]`. |

### ❌ DON'T

| #   | Rule                                                                                       |
| --- | ------------------------------------------------------------------------------------------ |
| 1   | **SHALL NOT** use `delay()` anywhere in Task_UI, Task_Touch, or Task_Net.                  |
| 2   | **SHALL NOT** hold `data_lock` during any network call, I/O, or `vTaskDelay`.              |
| 3   | **SHALL NOT** render directly to `tft` object — always use Sprite buffer.                  |
| 4   | **SHALL NOT** touch Pin 7, 8, 15, 16 as GPIO OUTPUT while Touch read is in progress.       |
| 5   | **SHALL NOT** call `WiFi.begin()` without first calling `WiFi.mode(WIFI_STA)`.             |
| 6   | **SHALL NOT** run Simulation data (`use_dummy`) alongside real MQTT data simultaneously.   |
| 7   | **SHALL NOT** create additional tasks on Core 1 with priority > 1 (to not starve Task_UI). |

---

## 8. Non-Functional Requirements

| Metric                 | Target                     |
| :--------------------- | :------------------------- |
| Boot to UI             | < 3 seconds                |
| WiFi connect time      | < 15 seconds               |
| MQTT reconnect retry   | every 5 seconds            |
| UI touch response      | < 80ms debounce            |
| Data staleness timeout | 10 seconds → reset to NULL |
| Serial baud rate       | 115200                     |
| Task_UI stack          | ≥ 16 KB                    |
| Touch poll rate        | 50 Hz (every 20ms)         |

---

## 9. Constraints

- `delay()` SHALL NOT be used for any timing logic.
- WiFi reconnect SHALL NOT block the display or touch pipeline.
- MQTT `loop()` SHALL be called at minimum every 30ms to prevent broker timeout.
- All task communication SHALL go through `g_state` — no direct inter-task calls.
- Touch and Display CANNOT access the shared bus concurrently.

---

## 10. Future Implementation Roadmap

- [ ] **WiFi Scan Real Logic** — Implementasi async `WiFi.scanNetworks(true)`, populate hasil ke `g_state` untuk ditampilkan di `SCREEN_WIFI_SCAN`.
- [ ] **Slave Communication** — Integrasi protokol Modbus/RS485 atau protokol custom via UART untuk komunikasi ke unit Slave.
- [ ] **OTA (Over-The-Air)** — Update firmware via WiFi tanpa perlu kabel USB.
- [ ] **Bus Overclocking** — Optimasi timing parallel bus TFT untuk meningkatkan FPS di atas 30.
- [ ] **Firebase Sync** — Sinkronisasi state gedung ke Firebase Realtime Database.
- [ ] **Screen: Slave Manager** — UI untuk monitoring dan kontrol unit-unit Slave terhubung.

---

## 11. Example `platformio.ini` (Key Config)

```ini
[env:esp32-s3-devkitc-1]
platform = espressif32@6.4.0
board = esp32-s3-devkitc-1
framework = arduino
board_build.f_cpu = 240000000L
upload_port = COM14

build_flags =
    -D TFT_CS=17  -D TFT_DC=16  -D TFT_RST=48
    -D TFT_WR=15  -D TFT_RD=14
    -D TFT_D0=1   -D TFT_D1=2   -D TFT_D2=3   -D TFT_D3=4
    -D TFT_D4=5   -D TFT_D5=6   -D TFT_D6=7   -D TFT_D7=8
```

---
