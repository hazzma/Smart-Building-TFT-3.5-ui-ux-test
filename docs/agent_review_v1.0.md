# Codebase Understanding & Review (v1.0)
**Agent:** Antigravity (Master Agent)
**Date:** 2026-04-14
**Project:** Smart Building Master S3

## 1. Executive Summary
This document outlines my complete understanding of the codebase as of today. The project has undergone a significant architectural shift from **LovyanGFX** to **TFT_eSPI** to resolve hardware bus contention issues with the resistive touchscreen.

## 2. Technical Stack (Current State)
- **MCU**: ESP32-S3 (N16R8) - 16MB Flash, 8MB PSRAM.
- **Display Driver**: `TFT_eSPI` configured for **8-bit Parallel** (ILI9488).
- **Touch Driver**: `Adafruit TouchScreen` (Low-level ADC) + `S3_Parallel_Touch` (Pin management).
- **Graphics Strategy**: Double-buffered `TFT_eSprite` in PSRAM (480x320, 16-bit).
- **RTOS Architecture**:
    - **Core 1 (Task_UI)**: Rendering, Touch Polling, UI state handling.
    - **Core 0 (Task_Net)**: Network sync, Firebase, Slave communication.

## 3. Hardware Conflict & Solution
The 3.5" TFT Shield shares control and data lines with the Touch matrix:
- `TOUCH_XP` -> `LCD_D6` (GPIO 7)
- `TOUCH_YP` -> `LCD_WR` (GPIO 15)
- `TOUCH_XM` -> `LCD_RS/DC` (GPIO 16)
- `TOUCH_YM` -> `LCD_D7` (GPIO 8)

**The Solution:**
We use a custom library `S3_Parallel_Touch`. After every ADC read of the touch matrix, the library executes `restoreBusPins()` which forces the shared pins back into the correct `OUTPUT` and `HIGH/LOW` states required for the I80 Parallel Bus to function. This prevents "kacau" (distorted) display output that occurred with LovyanGFX's internal touch handling.

## 4. Source Code Mapping
| File | Responsibility |
|---|---|
| `main.cpp` | RTOS Task initialization and main control loop. |
| `display.cpp/h` | `TFT_eSPI` hardware initialization and color tokens. |
| `ui_widgets.cpp/h` | Low-level drawing functions (Cards, Buttons, Badges). |
| `ui_screens.cpp/h` | Dashboard layout and touch hit-box logic. |
| `touch.cpp/h` | Touch calibration and coordinate mapping. |
| `data.cpp/h` | `BuildingState` struct and Thread-safe mutex access. |
| `lib/S3_Parallel_Touch` | Custom glue library to safely share pins between TFT and Touch. |

## 5. Master Agent Perspective
As the **Master Agent**, my role is to oversee the integration of these modules. 
- **UI/UX Agent** should focus on `ui_widgets.cpp` and `ui_screens.cpp`.
- **Data/Comms Agent** should focus on `data.cpp`, `comms.cpp` (to be implemented), and `Task_Net`.
- **TC/Troubleshoot Agent** should focus on `touch.cpp` and `S3_Parallel_Touch`.

## 6. Logic Flow
1. `Task_UI` polls touch via `touch_get_point()` (50Hz limit).
2. If hit detected, `screens_handle_touch()` updates `g_state`.
3. `screens_render()` draws the UI to the *inactive* sprite.
4. `canvas.pushSprite(0, 0)` transfers the buffer to the ILI9488.
5. `widgets_swap()` toggles the active/inactive sprite for the next frame.

## 7. Known Discrepancies
- The `docs/SmartBuilding_Master_FSD_v1.0 (1).md` still refers to **LovyanGFX** in several sections. This should be updated by the next documentation pass to reflect the move to `TFT_eSPI`.
- FPS is currently measured around 88 FPS (previously with Lovyan), let's verify if `TFT_eSPI` maintains this performance. Current `main.cpp` prints FPS to Serial.

---
*End of Understanding Doc*
