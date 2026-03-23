# Display Initialization & Token System

**Agent:** UI/UX & Display Agent
**Status:** Initialized (Phase 1)
**Date:** 2026-03-23

## 1. Overview
The display system is based on **LovyanGFX** using a parallel 8-bit bus on the ESP32-S3. This phase focuses on the initialization of the bus and the establishment of the **Design Language** (Design Tokens).

## 2. Design Tokens Added
The following tokens are implemented in `src/display.h` as 16-bit RGB565 values:

| Token Name | Hex Code | Purpose |
|---|---|---|
| `COLOR_BG_MAIN` | `#0A0E1A` | Main application background |
| `COLOR_ACCENT_MAIN` | `#00D4FF` | Primary action/highlight colors |
| `COLOR_CARD_BG` | `#141A2E` | Sensor cards and header bar background |
| `COLOR_TEXT_MAIN` | `#E8F4FD` | Primary text |
| `COLOR_STAT_ON` | `#00C896` | Active devices (AC, Projector, etc.) |
| `COLOR_STAT_ERR` | `#FF4D4D` | Persistent error reporting |

## 3. Bus Configuration
- **Interface:** Parallel 8-bit
- **Bus Speed:** 20MHz (Standard)
- **Rotation:** Landscape (Mode 1: 480x320)
- **DMA:** Enabled (`lcd.setDMA(true)`) for non-blocking buffer transfers.

## 4. Performance Notes
- Currently running on **Core 1 (Priority 3)** as per FSD 3.3.
- The use of the `lgfx::LGFX_Device` class allows for highly efficient low-level bus access.
- `lcd.beginWrite()` / `lcd.endWrite()` pattern is established to support future bus-sharing with the resistive touchscreen.

---
*Next planned step: Implementation of reusable UI components in `ui_widgets.cpp`.*
