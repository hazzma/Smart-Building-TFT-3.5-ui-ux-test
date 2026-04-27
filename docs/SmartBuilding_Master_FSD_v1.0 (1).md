# FUNCTIONAL SPECIFICATION DOCUMENT
## Smart Building — Master Device TFT Display Controller

> **Platform:** ESP32-S3 N16R8 | **Display:** 3.5" Parallel TFT 480×320
> **Library:** LovyanGFX + Adafruit TouchScreen | **Framework:** Arduino / PlatformIO
> **Slave Comm:** Modbus RTU / CAN Bus (TBD) | **Cloud:** Firebase + Flutter
> **Version:** 1.0 | 2025

---

## Block Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                        SMART BUILDING SYSTEM                        │
│                                                                     │
│  ┌──────────────────────────────────────────────────────────────┐   │
│  │                     MASTER DEVICE                            │   │
│  │                   ESP32-S3 N16R8                             │   │
│  │                                                              │   │
│  │  ┌──────────────┐   ┌──────────────┐   ┌─────────────────┐  │   │
│  │  │   Task_UI    │   │ Task_Comms   │   │ Task_Firebase   │  │   │
│  │  │   Core 1     │   │   Core 0     │   │    Core 0       │  │   │
│  │  │  (Priority3) │   │ (Priority 2) │   │  (Priority 1)   │  │   │
│  │  └──────┬───────┘   └──────┬───────┘   └────────┬────────┘  │   │
│  │         │                  │                     │           │   │
│  │         ▼                  ▼                     ▼           │   │
│  │  ┌─────────────────────────────────────────────────────────┐ │   │
│  │  │              BuildingState (Shared Memory)              │ │   │
│  │  │         SensorData  |  NetworkState  |  Mutex           │ │   │
│  │  └──────────────────────────┬──────────────────────────────┘ │   │
│  │                             │                                 │   │
│  │         ┌───────────────────┼───────────────────┐            │   │
│  │         ▼                   ▼                   ▼            │   │
│  │  ┌─────────────┐   ┌──────────────┐   ┌──────────────────┐  │   │
│  │  │ display.cpp │   │  comms.cpp   │   │  firebase.cpp    │  │   │
│  │  │ LovyanGFX   │   │ Proto Adapter│   │  Realtime DB     │  │   │
│  │  └──────┬──────┘   └──────┬───────┘   └────────┬─────────┘  │   │
│  │         │                  │                     │           │   │
│  └─────────┼──────────────────┼─────────────────────┼───────────┘   │
│            │                  │                     │               │
│            ▼                  ▼                     ▼               │
│  ┌─────────────────┐  ┌───────────────────┐  ┌───────────────────┐  │
│  │  TFT 3.5" +     │  │     RJ45          │  │     Firebase      │  │
│  │  Resistive      │  │  ┌────────────┐   │  │   Realtime DB     │  │
│  │  Touchscreen    │  │  │  SLAVE 1   │   │  └────────┬──────────┘  │
│  │  480×320px      │  │  │ Sensors +  │   │           │             │
│  │  8-bit Parallel │  │  │ Actuators  │   │           ▼             │
│  └─────────────────┘  │  └────────────┘   │  ┌───────────────────┐  │
│                       │  ┌────────────┐   │  │   Flutter App     │  │
│                       │  │  SLAVE 2   │   │  │  Remote Control   │  │
│                       │  │ Sensors +  │   │  └───────────────────┘  │
│                       │  │ Actuators  │   │                         │
│                       │  └────────────┘   │                         │
│                       └───────────────────┘                         │
└─────────────────────────────────────────────────────────────────────┘
```

### Data Flow Summary

```
[Slave 1/2] ──RJ45──► comms.cpp ──► BuildingState ──► ui_screens.cpp ──► TFT Display
[Touch Input] ────────► ui_widgets.cpp ──► comms.cpp ──────────────────► Slave Command
[BuildingState] ──────► firebase.cpp ──► Firebase Realtime DB ──────────► Flutter App
[Flutter App] ────────► Firebase /commands/ ──► firebase_pull_commands() ──► Slave
```

---

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [Hardware Specification](#2-hardware-specification)
3. [Software Architecture](#3-software-architecture)
4. [File Specification](#4-file-specification)
5. [UI/UX Specification](#5-uiux-specification)
6. [Communication Protocol](#6-communication-protocol)
7. [Usage Guide — AI Agent (Antigravity)](#7-usage-guide--ai-agent-antigravity)
8. [PlatformIO Configuration](#8-platformio-configuration)
9. [Revision History & Roadmap](#9-revision-history--roadmap)

---

## 1. Project Overview

### 1.1 Deskripsi Proyek

Smart Building Master Device adalah sebuah unit kontrol sentral berbasis ESP32-S3 yang dilengkapi layar TFT sentuh 3.5 inci. Perangkat ini berfungsi sebagai antarmuka utama (HMI) untuk memantau kondisi ruangan dan mengirimkan perintah ke slave device yang terpasang melalui koneksi RJ45.

**Master device TIDAK menjalankan sensor apapun secara langsung.** Seluruh data sensor (suhu, cahaya, kehadiran manusia, dsb.) berasal dari slave device yang terhubung.

### 1.2 Tujuan Dokumen

Dokumen FSD ini dibuat sebagai referensi teknis lengkap untuk:

- Developer yang mengimplementasikan firmware Master Device
- **AI Agent (Google Antigravity)** yang mengintegrasikan dan mengoperasikan sistem
- Tim hardware yang memverifikasi pinout dan koneksi
- Tim backend yang menghubungkan ke Firebase dan Flutter

### 1.3 Scope

Dokumen ini mencakup:

- Konfigurasi hardware dan pinout lengkap
- Arsitektur software dan struktur file
- Spesifikasi UI/UX layar TFT
- Lapisan komunikasi ke slave (protocol-agnostic)
- Integrasi Firebase
- Panduan penggunaan untuk AI Agent

---

## 2. Hardware Specification

### 2.1 Bill of Materials (BOM)

| Komponen | Spesifikasi | Keterangan |
|---|---|---|
| MCU | ESP32-S3 N16R8 (DevKitC-1) | 16MB Flash, 8MB PSRAM, dual-core 240MHz |
| Display | 3.5" TFT ILI9488 / ILI9486 | 480×320px, 16-bit parallel interface |
| Touch | Resistive 4-wire (XPT2046 compatible) | Terpasang pada panel TFT |
| Koneksi Slave | RJ45 via RS485 / CAN Bus (TBD) | Max 2 slave device |
| Power | 5V / 3.3V regulated | Dari DevKit onboard regulator |

### 2.2 Pinout TFT Display (8-bit Parallel)

> ⚠️ **PINOUT INI SUDAH DIVALIDASI DAN TIDAK BOLEH DIUBAH**

| Signal | GPIO ESP32-S3 | Deskripsi |
|---|---|---|
| LCD_D0 | GPIO 1 | Data bus bit 0 |
| LCD_D1 | GPIO 2 | Data bus bit 1 |
| LCD_D2 | GPIO 3 | Data bus bit 2 |
| LCD_D3 | GPIO 4 | Data bus bit 3 |
| LCD_D4 | GPIO 5 | Data bus bit 4 |
| LCD_D5 | GPIO 6 | Data bus bit 5 |
| LCD_D6 | GPIO 7 | Data bus bit 6 |
| LCD_D7 | GPIO 8 | Data bus bit 7 |
| LCD_RD | GPIO 14 | Read strobe (active LOW) |
| LCD_WR | GPIO 15 | Write strobe (active LOW) |
| LCD_RS / DC | GPIO 16 | Register select / Data-Command |
| LCD_CS | GPIO 17 | Chip select (active LOW) |
| LCD_RST | GPIO 48 | Reset (active LOW) |

### 2.3 Pinout Touchscreen (Resistive 4-wire — Shared Pin)

> ⚠️ **PENTING: Pin touchscreen di-SHARE dengan data bus dan control signal TFT.**
> LovyanGFX harus **melepas bus terlebih dahulu** sebelum Adafruit TouchScreen dapat membaca ADC,
> lalu bus dikembalikan ke mode TFT setelah pembacaan selesai. Lihat catatan implementasi di section 4.8.

| Signal Touch | GPIO ESP32-S3 | Shared Dengan | Mode saat baca touch |
|---|---|---|---|
| XP (X+) | GPIO 7 | LCD_D6 (Data Bit 6) | Ubah ke INPUT, baca ADC |
| XM (X-) | GPIO 16 | LCD_RS / DC (Command/Data) | Ubah ke OUTPUT LOW |
| YP (Y+) | GPIO 15 | LCD_WR (Write strobe) | Ubah ke OUTPUT HIGH |
| YM (Y-) | GPIO 8 | LCD_D7 (Data Bit 7) | Ubah ke INPUT, baca ADC |

**Urutan operasi touch yang benar:**
1. Panggil `lcd.endWrite()` — lepas bus TFT
2. Set pin XP, XM, YP, YM ke mode GPIO biasa (INPUT/OUTPUT sesuai kebutuhan ADC)
3. Baca koordinat touch via Adafruit TouchScreen
4. Set pin kembali ke mode bus TFT
5. Panggil `lcd.beginWrite()` — ambil kembali bus TFT

### 2.4 Panel Dimensions

| Parameter | Nilai |
|---|---|
| Physical Width (PHYS_W) | 480 pixel |
| Physical Height (PHYS_H) | 320 pixel |
| Orientasi default | Landscape |
| Color depth | 16-bit (RGB565) |

---

## 3. Software Architecture

### 3.1 Platform & Toolchain

| Parameter | Nilai | Keterangan |
|---|---|---|
| platform | `espressif32@6.4.0` | WAJIB, tidak boleh diubah |
| board | `esp32-s3-devkitc-1` | WAJIB, tidak boleh diubah |
| framework | `arduino` | WAJIB, tidak boleh diubah |
| Library Display | LovyanGFX | Rendering TFT performa tinggi |
| Library Touch | Adafruit TouchScreen | Resistive touch input |
| Library NVS | Preferences (built-in Arduino ESP32) | Simpan setting ke flash |
| Library WiFi | WiFi.h (built-in) | Koneksi WiFi & NTP |
| Library Firebase | Firebase ESP32 Client (mobizt) | Realtime DB & Firestore |

### 3.2 Struktur Direktori Proyek

```
smart-building-master/
├── platformio.ini
├── src/
│   ├── main.cpp              # Entry point, RTOS task launcher
│   ├── display.cpp / .h      # LovyanGFX driver & panel config
│   ├── ui_widgets.cpp / .h   # Komponen GUI (card, button, keyboard)
│   ├── ui_screens.cpp / .h   # Halaman: Dashboard, Settings, SlaveManager
│   ├── data.cpp / .h         # BuildingState struct, dummy & real data
│   ├── comms.cpp / .h        # Abstraksi protokol slave (Modbus/CAN)
│   ├── firebase.cpp / .h     # Push/pull data ke Firebase
│   ├── touch.cpp / .h        # Kalibrasi & mapping resistive touch
│   ├── settings.cpp / .h     # NVS read/write preferensi
│   └── error_handler.cpp / .h  # Centralized error & notif bar
└── docs/
    └── usage_guide.md        # Panduan penggunaan untuk AI Agent
```

### 3.3 RTOS Task Architecture

ESP32-S3 dual-core dimanfaatkan penuh dengan pembagian task sebagai berikut:

| Task Name | Core | Priority | Stack | Fungsi |
|---|---|---|---|---|
| `Task_UI` | Core 1 | **4 (Highest)** | 8KB | Render display full-time, target max FPS (~53 FPS). Loop: `screens_render` → `canvas.pushSprite` → `vTaskDelay(1)`. |
| `Task_Net` | Core 0 | 1 (Low) | 4KB | Placeholder: polling slave, Firebase sync, WiFi reconnect. Gunakan `data_lock()`/`data_unlock()` saat akses `BuildingState`. |

> **Catatan Performa (March 2026):**
> - Bus Parallel 8-bit dikonfigurasi di **30 MHz** via `cfg.freq_write = 30000000`
> - `bus_shared = false` saat mode display-only (tanpa touch aktif) → prioritas penuh ke I80 bus
> - `vTaskDelay(1)` pada Task_UI memberikan yield minimal ke scheduler tanpa menurunkan FPS secara signifikan
> - Hasil pengukuran aktual: **53 FPS** pada resolusi 480×320, direct PSRAM sprite DMA push


### 3.4 Data Flow Diagram

```
[Slave 1 / Slave 2]
        │
        ▼ RJ45
  comms.cpp (Protocol Adapter)
        │
        ▼
  data.cpp ──── BuildingState (mutex-protected)
        │
        ├──────────────────────────────────────┐
        ▼                                      ▼
  ui_screens.cpp                         firebase.cpp
  (TFT Display)                          (Firebase Realtime DB)
        ▲                                      ▲
        │                                      │
  ui_widgets.cpp                        Flutter App
  (Touch Input)                         /commands/ path
        │
        ▼
  comms.cpp → Slave Command
```

---

## 4. File Specification

### 4.1 `main.cpp`

Entry point aplikasi. Bertanggung jawab untuk:

- Inisialisasi hardware (display, touch, serial)
- Memuat settings dari NVS
- Membuat dan meluncurkan semua RTOS task
- **Tidak mengandung logika bisnis apapun**

| Fungsi | Parameter | Return | Deskripsi |
|---|---|---|---|
| `setup()` | - | void | Inisialisasi hardware & launch tasks |
| `loop()` | - | void | Kosong — semua logika di RTOS tasks |

---

### 4.2 `display.cpp` / `display.h`

Konfigurasi LovyanGFX untuk panel 8-bit parallel. File ini berisi struct `LGFX_Config` dengan seluruh pinout TFT. Mengekspos instance global `lcd` yang digunakan oleh seluruh modul lain.

| Fungsi / Objek | Deskripsi |
|---|---|
| `LGFX lcd` | Instance global LovyanGFX — gunakan ini di seluruh file |
| `LGFX_Sprite canvas(&lcd)` | Sprite untuk double-buffering — deklarasi di ui_screens |
| `display_init()` | Inisialisasi lcd, set rotasi landscape, fill hitam |
| `display_brightness(uint8_t val)` | Set backlight brightness (0–255) jika pin BL tersedia |

Konfigurasi `LGFX_Config` wajib memuat:

```cpp
// Wajib ada di display.h — TIDAK BOLEH DIUBAH
bus_type  : lgfx::v1::Bus_Parallel8
pin_wr    : 15
pin_rd    : 14
pin_rs    : 16   // DC
pin_cs    : 17
pin_rst   : 48
pin_d0..d7: 1, 2, 3, 4, 5, 6, 7, 8
freq_write: 20000000   // 20MHz — dapat dinaikkan ke 40MHz untuk tes
panel_type: lgfx::v1::Panel_ILI9488  // sesuaikan jika panel berbeda
use_dma   : true       // WAJIB untuk performa maksimal
```

---

### 4.3 `ui_widgets.cpp` / `ui_widgets.h`

Kumpulan komponen UI reusable. Semua widget menggunakan `LGFX_Sprite` untuk zero-flicker rendering.

| Widget / Fungsi | Parameter | Deskripsi |
|---|---|---|
| `drawTempCard()` | x, y, w, h, label, value, unit, color | Card suhu dengan nilai dan label sensor |
| `drawToggleButton()` | x, y, w, h, label, bool state | Tombol ON/OFF dengan state visual |
| `drawUpDownButton()` | x, y, label | Pair tombol ▲ ▼ untuk naik/turun nilai |
| `drawPresenceBadge()` | x, y, bool detected | Badge 'Detected' (hijau) / 'Not Detected' (abu) |
| `drawLuxCard()` | x, y, w, h, float lux | Kartu tampilan nilai lux rata-rata |
| `drawNotifBar()` | bool wifi, bool err, bool firebase | Render bar notifikasi atas layar |
| `drawSlideMenu()` | bool open, Settings& cfg | Animasi slide-in menu dari kiri |
| `drawVirtualKeyboard()` | char* buffer, int maxLen | Keyboard sentuh on-screen untuk input teks |
| `drawToast()` | const char* msg, uint16_t color | Notifikasi popup sementara di bawah layar |

---

### 4.4 `ui_screens.cpp` / `ui_screens.h`

Mengelola halaman-halaman utama aplikasi menggunakan state machine sederhana.

| Screen State | Enum Value | Deskripsi |
|---|---|---|
| `SCREEN_DASHBOARD` | 0 | Halaman utama — semua sensor & kontrol |
| `SCREEN_SETTINGS` | 1 | WiFi/LAN switch, Room Name editor |
| `SCREEN_SLAVE_MANAGER` | 2 | Daftar slave terdeteksi, enable/disable |
| `SCREEN_TOUCH_CALIB` | 3 | Halaman kalibrasi touchscreen |

| Fungsi | Parameter | Deskripsi |
|---|---|---|
| `screens_init()` | - | Alokasi sprite double-buffer, init state machine |
| `screens_render()` | BuildingState& state | Render halaman aktif ke sprite, lalu push ke LCD |
| `screens_handle_touch()` | TouchPoint& tp | Routing touch event ke screen yang aktif |
| `screens_set()` | ScreenState s | Ganti halaman aktif + trigger animasi transisi |

---

### 4.5 `data.cpp` / `data.h`

Mendefinisikan struct `BuildingState` sebagai **satu-satunya sumber data (single source of truth)** yang dishare ke seluruh modul. Saat slave belum terhubung, data dummy digunakan.

#### Struct `BuildingState`

```cpp
struct SensorData {
  float    temp[4];         // Suhu sensor 1-4 (°C)
  float    temp_target;     // Target suhu AC (°C)
  float    lux_avg;         // Lux rata-rata ruangan
  bool     ac_on;           // Status AC
  bool     projector_on;    // Status proyektor
  bool     light_on;        // Status lampu
  bool     human_presence;  // Ada/tidak kehadiran manusia
  bool     sensor_error[4]; // Error flag per sensor
  uint8_t  slave_count;     // Jumlah slave terhubung (0-2)
  bool     slave_online[2]; // Status online per slave
};

struct NetworkState {
  bool  wifi_connected;
  bool  lan_connected;
  bool  firebase_ok;
  char  room_name[32];
  char  slave_name[2][32];
};

struct BuildingState {
  SensorData          sensor;
  NetworkState        net;
  bool                use_dummy;   // true = gunakan data dummy
  SemaphoreHandle_t   mutex;       // RTOS mutex untuk akses aman
};
```

| Fungsi | Deskripsi |
|---|---|
| `data_init(BuildingState&)` | Init struct, load dummy data, buat mutex |
| `data_load_dummy(BuildingState&)` | Isi data dengan nilai dummy realistis |
| `data_lock(BuildingState&)` | Ambil mutex sebelum baca/tulis (max 50ms timeout) |
| `data_unlock(BuildingState&)` | Lepas mutex setelah selesai |

---

### 4.6 `comms.cpp` / `comms.h`

Lapisan abstraksi komunikasi ke slave. Didesain agar protokol (Modbus RTU atau CAN Bus) dapat **diganti hanya dengan mengubah file ini** tanpa menyentuh logika lain.

| Fungsi | Parameter | Deskripsi |
|---|---|---|
| `comms_init()` | CommsProtocol proto | Init hardware UART/CAN sesuai proto yang dipilih |
| `comms_poll()` | BuildingState& state | Baca data terbaru dari semua slave, update state |
| `comms_send_ac()` | bool on | Kirim perintah ON/OFF AC ke slave |
| `comms_send_temp_target()` | float target | Kirim target suhu ke slave |
| `comms_send_light()` | bool on | Kirim perintah ON/OFF lampu |
| `comms_send_projector()` | bool on | Kirim perintah ON/OFF proyektor |
| `comms_get_slave_list()` | SlaveInfo* out, int& count | Scan dan return daftar slave aktif |
| `comms_set_slave_enable()` | int idx, bool enable | Enable/disable slave tertentu |

```cpp
enum CommsProtocol {
  COMMS_MODBUS_RTU,  // Implementasi Modbus RTU via UART
  COMMS_CAN_BUS,     // Implementasi CAN Bus via TWAI driver
  COMMS_DUMMY        // Simulasi slave tanpa hardware (dev mode)
};
```

> Saat `proto = COMMS_DUMMY`, semua fungsi comms mensimulasikan response slave tanpa hardware fisik.

---

### 4.7 `firebase.cpp` / `firebase.h`

Handle push dan pull data ke Firebase Realtime Database menggunakan library Firebase ESP32 Client (mobizt).

| Fungsi | Deskripsi |
|---|---|
| `firebase_init()` | Autentikasi ke Firebase menggunakan API key dari settings |
| `firebase_push(BuildingState&)` | Upload seluruh SensorData ke path `/buildings/{room_name}/` |
| `firebase_pull_commands(BuildingState&)` | Baca command dari Flutter app di `/buildings/{room_name}/commands/` |
| `firebase_set_status(bool ok)` | Update status koneksi di notif bar |

#### Firebase JSON Path Structure

```
/buildings/{room_name}/sensors/temp_1
/buildings/{room_name}/sensors/temp_2
/buildings/{room_name}/sensors/temp_3
/buildings/{room_name}/sensors/temp_4
/buildings/{room_name}/sensors/lux_avg
/buildings/{room_name}/sensors/human_presence
/buildings/{room_name}/controls/ac_on
/buildings/{room_name}/controls/light_on
/buildings/{room_name}/controls/projector_on
/buildings/{room_name}/controls/temp_target
/buildings/{room_name}/commands/          ← Flutter writes here
```

---

### 4.8 `touch.cpp` / `touch.h`

Wrapper Adafruit TouchScreen dengan kalibrasi dan mapping koordinat ke resolusi layar.

> ⚠️ **Shared Pin — Wajib Dibaca:** Pin touchscreen berbagi GPIO dengan bus TFT (DB6, DB7, WR, CD).
> Setiap kali ingin membaca touch, bus TFT harus dilepas dulu dan dikembalikan setelahnya.

#### Konfigurasi Pin (Shared dengan TFT)

```cpp
// Di touch.h — pin ini SHARED dengan TFT bus, JANGAN diubah
#define TOUCH_XP  7   // Shared dengan LCD_D6
#define TOUCH_XM  16  // Shared dengan LCD_RS/DC
#define TOUCH_YP  15  // Shared dengan LCD_WR
#define TOUCH_YM  8   // Shared dengan LCD_D7
```

#### Prosedur Baca Touch (Wajib Diikuti)

```cpp
// BENAR — urutan wajib saat membaca touch
lcd.endWrite();                          // 1. Lepas bus TFT
TouchPoint tp = ts.getPoint();           // 2. Baca ADC touch
pinMode(TOUCH_XP, OUTPUT);              // 3. Kembalikan pin ke mode output
pinMode(TOUCH_YM, OUTPUT);
lcd.beginWrite();                        // 4. Ambil kembali bus TFT
```

| Konstanta / Fungsi | Deskripsi |
|---|---|
| `TS_MINX`, `TS_MAXX` | Nilai raw ADC minimum dan maksimum sumbu X (dari kalibrasi) |
| `TS_MINY`, `TS_MAXY` | Nilai raw ADC minimum dan maksimum sumbu Y |
| `TS_MINPRESSURE` | Threshold tekanan minimum agar touch dianggap valid (default: 200) |
| `touch_init()` | Inisialisasi objek TouchScreen dengan pin shared yang dikonfigurasi |
| `touch_get_point(TouchPoint& tp)` | Lepas bus TFT → baca ADC → kembalikan bus → map ke koordinat layar (0..479, 0..319), return true jika valid |
| `touch_run_calibration()` | Mode kalibrasi interaktif, simpan hasil ke NVS |

---

### 4.9 `settings.cpp` / `settings.h`

Manajemen konfigurasi persisten menggunakan ESP32 NVS (Non-Volatile Storage) via `Preferences` library.

| Key NVS | Tipe | Default | Deskripsi |
|---|---|---|---|
| `room_name` | String | `"Room 1"` | Nama ruangan ditampilkan di notif bar |
| `use_wifi` | Bool | `true` | true=WiFi, false=LAN/wired |
| `wifi_ssid` | String | `""` | SSID WiFi tersimpan |
| `wifi_pass` | String | `""` | Password WiFi tersimpan |
| `touch_minx` | Int | `200` | Kalibrasi touch sumbu X min |
| `touch_maxx` | Int | `3800` | Kalibrasi touch sumbu X max |
| `touch_miny` | Int | `200` | Kalibrasi touch sumbu Y min |
| `touch_maxy` | Int | `3800` | Kalibrasi touch sumbu Y max |
| `slave_0_name` | String | `"Slave 1"` | Nama slave pertama |
| `slave_1_name` | String | `"Slave 2"` | Nama slave kedua |

---

### 4.10 `error_handler.cpp` / `error_handler.h`

Mengelola semua error kondisi dan mengupdate state notif bar secara terpusat.

| Fungsi | Deskripsi |
|---|---|
| `err_report(ErrorCode code, uint8_t src)` | Catat error dengan kode dan sumber (sensor index, slave index, dsb.) |
| `err_clear(ErrorCode code)` | Hapus error jika kondisi sudah pulih |
| `err_has_active()` | Return true jika ada error aktif — digunakan notif bar |
| `err_get_list(ErrorInfo* out)` | Return array semua error aktif untuk ditampilkan |

```cpp
enum ErrorCode {
  ERR_SENSOR_TIMEOUT = 0x01,  // Sensor tidak merespons
  ERR_SLAVE_OFFLINE  = 0x02,  // Slave tidak terdeteksi
  ERR_FIREBASE_FAIL  = 0x03,  // Gagal sync ke Firebase
  ERR_WIFI_LOST      = 0x04,  // Koneksi WiFi terputus
  ERR_TOUCH_UNCAL    = 0x05   // Touch belum dikalibrasi
};
```

---

## 5. UI/UX Specification

### 5.1 Design Language

| Atribut | Spesifikasi |
|---|---|
| Tema | Dark Elegant Modern |
| Background utama | `#0A0E1A` (biru gelap pekat) |
| Accent utama | `#00D4FF` (cyan elektrik) |
| Accent sekunder | `#0A84FF` (biru cerah) |
| Card background | `#141A2E` (biru gelap medium) |
| Teks utama | `#E8F4FD` (putih kebiruan) |
| Teks sekunder | `#6B8CAE` (abu biru) |
| Status ON | `#00C896` (hijau teal) |
| Status OFF | `#2C3A4A` (abu gelap) |
| Status Error | `#FF4D4D` (merah) |
| Status Warning | `#FFB800` (kuning amber) |
| Font | Built-in LovyanGFX font — Font7 untuk angka besar, Font2 untuk teks |

### 5.2 Layout Dashboard (480×320px Landscape)

```
┌──────────────────────────────────────────────────────────────────────────────┐
│ ☰  Room Name              🌐  ⚠  🔥  12:34                         [28px]  │
├──────────────────────────────────────────────────────────────────────────────┤
│ ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐  ┌──────────────────┐  │
│ │ SENSOR 1 │ │ SENSOR 2 │ │ SENSOR 3 │ │ SENSOR 4 │  │   PROJECTOR      │  │
│ │  24.5°C  │ │  25.1°C  │ │  23.8°C  │ │  26.0°C  │  │  [ ON / OFF ]    │  │
│ └──────────┘ └──────────┘ └──────────┘ └──────────┘  ├──────────────────┤  │
│                                                        │  Human Presence  │  │
│  Target Suhu: 23.0 °C                                  │  ● Detected      │  │
│                                                        └──────────────────┘  │
│  ┌──────────────────────────────┐                                            │
│  │       [ ON / OFF AC ]        │       ┌──────────────────────────────┐    │
│  └──────────────────────────────┘       │        [ LIGHT ON/OFF ]      │    │
│                                         └──────────────────────────────┘    │
│  ┌───────────────┐  ┌───────────────┐                                        │
│  │   ▲  NAIK     │  │   ▼  TURUN    │   Avg Lux: 430 lx                     │
│  └───────────────┘  └───────────────┘                                        │
└──────────────────────────────────────────────────────────────────────────────┘
```

### 5.3 Pixel-Perfect Widget Coordinates

| Area | Posisi (x, y) | Ukuran (w × h) | Konten |
|---|---|---|---|
| Notification Bar | 0, 0 | 480 × 28 | ☰ \| Room Name \| WiFi icon \| ⚠ icon \| Firebase \| Clock |
| Divider Line | 0, 28 | 480 × 2 | Garis separator accent |
| Temp Card 1 | 8, 36 | 104 × 80 | Sensor 1 — label + nilai suhu + unit °C |
| Temp Card 2 | 118, 36 | 104 × 80 | Sensor 2 |
| Temp Card 3 | 228, 36 | 104 × 80 | Sensor 3 |
| Temp Card 4 | 338, 36 | 104 × 80 | Sensor 4 |
| Target Suhu | 8, 124 | 220 × 28 | Target: XX.X °C |
| AC Toggle | 8, 158 | 160 × 44 | Tombol ON/OFF AC |
| Temp Up | 8, 208 | 78 × 44 | Tombol ▲ Naik |
| Temp Down | 92, 208 | 78 × 44 | Tombol ▼ Turun |
| Projector Toggle | 346, 36 | 126 × 44 | ON/OFF Proyektor |
| Presence Badge | 346, 90 | 126 × 44 | Human Presence status |
| Light Toggle | 346, 180 | 126 × 44 | ON/OFF Lampu |
| Lux Display | 346, 234 | 126 × 44 | Lux: XXX lx |

### 5.4 Notification Bar

Bar 28px di bagian atas layar, selalu terlihat di semua halaman:

- **Kiri:** Icon ☰ (menu hamburger) — area tap 40×28px
- **Tengah-kiri:** Room Name (dari NVS)
- **Tengah-kanan:** Icon WiFi (tercoret jika tidak terhubung)
- **Tengah-kanan:** Icon ⚠ kuning (hanya muncul jika ada error aktif)
- **Kanan:** Icon Firebase (warna hijau=ok / merah=fail)
- **Kanan:** Waktu HH:MM (dari NTP jika WiFi aktif)

### 5.5 Slide-in Side Menu

Muncul dari kiri saat icon ☰ ditekan. Animasi geser dengan durasi ~200ms.

- Lebar panel: 200px, tinggi penuh 320px, background `#1A2744`
- **Section 1 — Settings:** switch WiFi/LAN, opsi scanning WiFi dan tampilkan list WiFi yang tersedia untuk dihubungkan, edit Room Name. Masing-masing opsi input (Password WiFi atau Room Name) akan membuka virtual keyboard.
- **Section 2 — Slaves:** daftar slave (max 2), nama + status online + toggle enable/disable
- **Section 3 — System:** tombol Touch Calibration, versi firmware
- Tap di luar panel untuk menutup

### 5.6 Virtual Keyboard

Keyboard on-screen untuk input Room Name dan nama slave. Muncul di bagian bawah layar.

- QWERTY layout 10+9+7 karakter + spasi
- Baris tambahan: angka 0-9, backspace, OK/Enter
- Setiap key: 36×36px, background `#1E2D42`, border accent saat ditekan
- Input field di atas keyboard menampilkan teks yang sedang diketik
- Tombol OK menyimpan ke NVS dan menutup keyboard

### 5.7 Performa Rendering

| Strategi | Detail |
|---|---|
| Double Buffering | `LGFX_Sprite canvas` 480×320 di PSRAM — render ke sprite, lalu `pushSprite()` ke LCD |
| Partial Update | Hanya re-render widget yang datanya berubah (dirty flag per widget) |
| DMA Transfer | Aktifkan DMA di LovyanGFX config (`use_dma: true`) untuk transfer non-blocking |
| Target FPS | 30–60 FPS pada kondisi idle, minimum 15 FPS saat semua widget update |
| Bus Speed | Mulai di 20MHz, tes stabilitas, naikkan hingga 40MHz jika stabil |

---

## 6. Communication Protocol

### 6.1 Abstraction Layer

`comms.cpp` mengimplementasikan pola **Protocol Adapter**. Saat tim memutuskan protokol final, hanya implementasi internal `comms.cpp` yang perlu diubah — seluruh file lain tidak perlu dimodifikasi.

| Scenario | Cara Swap Protokol |
|---|---|
| Ganti ke Modbus RTU | Ubah `comms_init()` ke `COMMS_MODBUS_RTU`, implementasi UART + framing Modbus |
| Ganti ke CAN Bus | Ubah `comms_init()` ke `COMMS_CAN_BUS`, implementasi TWAI driver ESP32 |
| Mode dummy/dev | Set `COMMS_DUMMY` — data disimulasikan tanpa hardware slave |

### 6.2 Data yang Dikirim Master → Slave

| Command | Tipe Data | Range | Deskripsi |
|---|---|---|---|
| `CMD_AC_ON` | bool | 0/1 | Nyalakan / matikan AC |
| `CMD_TEMP_TARGET` | float | 16.0 – 30.0°C | Set target suhu AC langsung |
| `CMD_TEMP_UP` | trigger | - | Naikkan target suhu +0.5°C |
| `CMD_TEMP_DOWN` | trigger | - | Turunkan target suhu -0.5°C |
| `CMD_LIGHT_ON` | bool | 0/1 | Nyalakan / matikan lampu |
| `CMD_PROJECTOR_ON` | bool | 0/1 | Nyalakan / matikan proyektor |

### 6.3 Data yang Dibaca Master ← Slave

| Field | Tipe Data | Range | Deskripsi |
|---|---|---|---|
| `temp[0..3]` | float | -10.0 – 85.0°C | Suhu dari 4 sensor di slave |
| `lux_avg` | float | 0 – 100000 lx | Rata-rata lux dari slave |
| `human_presence` | bool | 0/1 | Status kehadiran manusia |
| `sensor_error[0..3]` | bool | 0/1 | Flag error per sensor |
| `slave_id` | uint8_t | 1 / 2 | ID slave yang mengirim data |
| `slave_status` | uint8_t | 0=offline, 1=online | Status koneksi slave |

### 6.4 Firebase ↔ Flutter App

Untuk kontrol remote melalui Flutter, Firebase Realtime DB digunakan:

```
/buildings/{room_name}/commands/ac_on        ← Flutter writes
/buildings/{room_name}/commands/temp_target  ← Flutter writes
/buildings/{room_name}/commands/light_on     ← Flutter writes
/buildings/{room_name}/commands/projector    ← Flutter writes
```

`firebase_pull_commands()` membaca path ini setiap 2 detik dan mengeksekusi command yang ditemukan, lalu **menghapus node command tersebut** agar tidak dieksekusi ulang.

---

## 7. Usage Guide — AI Agent (Antigravity)

> 📌 **Bagian ini khusus untuk AI Agent Google Antigravity** yang mengintegrasikan dan mengoperasikan Smart Building Master Device.

### 7.1 Cara Membaca State Sistem

Seluruh state sistem tersedia dalam satu struct `BuildingState` yang dapat diakses via pointer global:

```cpp
extern BuildingState g_state;
```

Selalu gunakan mutex saat membaca atau menulis data:

```cpp
data_lock(g_state);

float t1  = g_state.sensor.temp[0];        // Suhu sensor 1
float t2  = g_state.sensor.temp[1];        // Suhu sensor 2
float t3  = g_state.sensor.temp[2];        // Suhu sensor 3
float t4  = g_state.sensor.temp[3];        // Suhu sensor 4
float tgt = g_state.sensor.temp_target;    // Target suhu
float lux = g_state.sensor.lux_avg;        // Lux rata-rata
bool  ac  = g_state.sensor.ac_on;          // Status AC
bool  lt  = g_state.sensor.light_on;       // Status lampu
bool  pj  = g_state.sensor.projector_on;   // Status proyektor
bool  pr  = g_state.sensor.human_presence; // Kehadiran manusia

data_unlock(g_state);
```

### 7.2 Cara Mengirim Perintah

Semua fungsi `comms_send_*` bersifat thread-safe dan dapat dipanggil dari task manapun:

| Intent | Fungsi | Contoh |
|---|---|---|
| Nyalakan AC | `comms_send_ac(true)` | `comms_send_ac(true);` |
| Matikan AC | `comms_send_ac(false)` | `comms_send_ac(false);` |
| Set suhu target | `comms_send_temp_target(float)` | `comms_send_temp_target(22.5f);` |
| Naikkan suhu +0.5°C | increment + send | `comms_send_temp_target(g_state.sensor.temp_target + 0.5f);` |
| Turunkan suhu -0.5°C | decrement + send | `comms_send_temp_target(g_state.sensor.temp_target - 0.5f);` |
| Nyalakan lampu | `comms_send_light(true)` | `comms_send_light(true);` |
| Nyalakan proyektor | `comms_send_projector(true)` | `comms_send_projector(true);` |

### 7.3 Cara Mengganti Nama Ruangan

```cpp
settings_set_room_name("Meeting Room A");
// NVS akan update otomatis
// Notif bar akan refresh di render cycle berikutnya
```

### 7.4 Cara Mengelola Slave

```cpp
// Cek jumlah slave aktif
int count = g_state.sensor.slave_count;

// Cek status slave
bool s1_online = g_state.sensor.slave_online[0]; // Slave 1
bool s2_online = g_state.sensor.slave_online[1]; // Slave 2

// Cek nama slave
char* name_s1 = g_state.net.slave_name[0];
char* name_s2 = g_state.net.slave_name[1];

// Enable / disable slave
comms_set_slave_enable(0, true);  // Enable Slave 1
comms_set_slave_enable(1, false); // Disable Slave 2
```

### 7.5 Cara Mengecek Error

```cpp
if (err_has_active()) {
  ErrorInfo errors[8];
  int count = err_get_list(errors);

  for (int i = 0; i < count; i++) {
    // errors[i].code   — lihat enum ErrorCode di bawah
    // errors[i].source — index sensor/slave yang bermasalah
    Serial.printf("Error code: 0x%02X, source: %d\n",
                  errors[i].code, errors[i].source);
  }
}
```

### 7.6 Cara Switch WiFi / LAN

```cpp
settings_set_use_wifi(false); // Switch ke LAN
settings_set_use_wifi(true);  // Switch ke WiFi
// Device akan reconnect otomatis
```

### 7.7 Firebase Push Manual

```cpp
firebase_push(g_state); // Push state saat ini ke Firebase secara langsung
```

### 7.8 Mode Data Dummy

Untuk development atau testing tanpa hardware slave:

```cpp
// Aktifkan mode dummy
g_state.use_dummy = true;
data_load_dummy(g_state); // Isi dengan data dummy realistis

// Kembali ke data real dari slave
g_state.use_dummy = false;
```

### 7.9 Error Codes Reference

| Error Code | Nilai Hex | Arti | Solusi |
|---|---|---|---|
| `ERR_SENSOR_TIMEOUT` | `0x01` | Sensor tidak merespons | Cek koneksi slave, restart comms |
| `ERR_SLAVE_OFFLINE` | `0x02` | Slave tidak terdeteksi | Cek kabel RJ45, cek power slave |
| `ERR_FIREBASE_FAIL` | `0x03` | Gagal sync ke Firebase | Cek koneksi WiFi, cek API key |
| `ERR_WIFI_LOST` | `0x04` | Koneksi WiFi terputus | Auto-reconnect aktif, tunggu |
| `ERR_TOUCH_UNCAL` | `0x05` | Touch belum dikalibrasi | Buka Settings → Touch Calibration |

> 💡 **Catatan untuk AI Agent:** Jika terjadi ghost touch atau koordinat kacau, kemungkinan besar
> urutan lepas/ambil bus TFT tidak dijalankan dengan benar di `touch_get_point()`.
> Pastikan `lcd.endWrite()` selalu dipanggil sebelum membaca ADC touchscreen.

---

## 8. PlatformIO Configuration

### 8.1 `platformio.ini` — Fixed, Tidak Boleh Diubah

```ini
[env:esp32-s3-devkitc-1]
platform  = espressif32@6.4.0
board     = esp32-s3-devkitc-1
framework = arduino

build_flags =
  -DBOARD_HAS_PSRAM
  -mfix-esp32-psram-cache-issue
  -DCORE_DEBUG_LEVEL=0
  -O2

lib_deps =
  lovyan03/LovyanGFX @ ^1.1.16
  adafruit/Adafruit TouchScreen @ ^1.1.5
  mobizt/Firebase ESP32 Client @ ^4.4.14

monitor_speed = 115200
```

### 8.2 Build Flags Penjelasan

| Flag | Fungsi |
|---|---|
| `-DBOARD_HAS_PSRAM` | Aktifkan dukungan PSRAM 8MB untuk sprite double-buffer |
| `-mfix-esp32-psram-cache-issue` | Fix bug cache PSRAM pada ESP32-S3 |
| `-DCORE_DEBUG_LEVEL=0` | Matikan debug output untuk performa maksimal di production |
| `-O2` | Optimasi compiler level 2 — balance antara ukuran dan kecepatan |

---

## 9. Revision History & Roadmap

### 9.1 Revision History

| Versi | Tanggal | Perubahan | Author |
|---|---|---|---|
| 1.0 | 2025 | Initial release — FSD lengkap Master Device TFT | Tim Smart Building |

### 9.2 Pending Decisions

| Item | Status | Impact |
|---|---|---|
| Protokol slave (Modbus vs CAN) | TBD — keputusan tim | Hanya mempengaruhi `comms.cpp` |
| IC driver panel (ILI9488 vs ILI9486) | Konfirmasi saat hardware tiba | Mempengaruhi `panel_type` di `display.h` |
| Firebase API key & project ID | Setup oleh tim backend | Masuk ke settings NVS |

### 9.3 Roadmap

1. **Phase 1 (Sekarang):** Display + Touch + UI — tanpa slave hardware, data dummy aktif
2. **Phase 2:** Integrasi `comms.cpp` dengan protokol slave yang dipilih (Modbus/CAN)
3. **Phase 3:** Integrasi Firebase + Flutter app remote control
4. **Phase 4:** Production hardening, multi-room support

---

*Smart Building Master Device FSD v1.0 — ESP32-S3 N16R8 — LovyanGFX — PlatformIO espressif32@6.4.0*
