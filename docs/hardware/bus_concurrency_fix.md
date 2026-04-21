# Display Bus & Touchscreen Concurrency Fix
**Date:** 2026-04-14
**Agent:** TC Agent
**Issue:** Display corruption (image noise/shifting) when the touchscreen is pressed rapidly (spam clicking).

## Root Cause Analysis
The 3.5" Parallel TFT shield shares its data and control lines with the resistive touch screen matrix:
- **XP** -> **LCD_D6** (GPIO 7)
- **XM** -> **LCD_RS/DC** (GPIO 16)
- **YP** -> **LCD_WR** (GPIO 15)
- **YM** -> **LCD_D7** (GPIO 8)

When the user "spams" the screen, the `touch_get_point()` function—which manually bit-bangs these pins into ADC mode—was occasionally firing while the `TFT_eSPI` driver was still in the middle of a high-speed parallel transfer or just before a command cycle. This left the ILI9488 controller in an inconsistent state.

## Solution Implemented

### 1. Driver-Level Isolation (`src/touch.cpp`)
Modified `touch_get_point` to wrap the touch reading logic with explicit driver lock/unlock calls:
```cpp
bool touch_get_point(int &tx, int &ty) {
    tft.endWrite(); // Force the driver to release the bus and set CS HIGH
    TouchPoint p = touchAgent.getPoint(480, 320, 1);
    tft.startWrite(); // Re-claim the bus for the next render cycle
    ...
}
```

### 2. Task Synchronization (`src/main.cpp`)
Reorganized the `Task_UI` loop to ensure a strict sequence:
1.  **Render Page**: CPU-bound drawing to the internal sprite (No bus activity).
2.  **Push Sprite**: Synchronous/Dedicated bus activity to update the physical LCD.
3.  **Swap Buffers**: Change pointers.
4.  **Touch Poll**: Attempt touch reading ONLY after the display push is finished.

By moving the touch polling to the end of the loop (after `pushSprite`), we ensure that we never attempt to read ADC values while the Parallel Bus LCD Peripheral on the ESP32-S3 is actively driving the data lines.

## Result
Stability is significantly improved. Even during rapid interaction, the display remains clear as the bus ownership is strictly handed off between the display driver and the touch ADC logic.

---
*TC Agent Documentation*
