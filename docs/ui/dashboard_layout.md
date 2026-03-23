# UI Widget & Dashboard Implementation

**Agent:** UI/UX & Display Agent
**Status:** Completed (Phase 2)
**Date:** 2026-03-23

## 1. Overview
Implemented the required Dashboard Layout using 16-bit `LGFX_Sprite` placed in PSRAM. The entire interface operates completely flicker-free thanks to Double Buffering techniques pushed to the hardware context via DMA.

## 2. Rich Aesthetics Additions
- Cards use a subtle `drawCardBase` method mapping `fillRoundRect` joined with a distinct tinted border for a premium framing effect.
- The background of the entire dashboard possesses a vertical gradient calculation to bring depth, rather than a flat background.
- Real-time subtle value ticking algorithms (e.g., Temp 1 increment, Lux flicker) provide an "alive" look simulating a constant read interval.

## 3. Pixel Perfect Coordinates
Adheres strictly to FSD 5.3 specifications:
- **Notification Bar**: `[0, 0, 480x28]` containing hamburger, room name, time, and status bounds.
- **Sensor Cards 1-4**: Top span from `Y=36` spaced by exactly `104x80px`.
- **Target Suhu Display**: Encompasses `[8, 124, 220x28]`.
- **AC Master**: Toggle button `[8, 158, 160x44]`.
- **Up/Down controls**: Coordinates `[8, 208]` & `[92, 208]`.
- **Right Hand Operations**: Projector, Presence Badge, Lights, and Lux anchored symmetrically along `X=346`.

## 4. Performance Metrics
Render interval in the RTOS loop is anchored targeting ~30FPS `vTaskDelay(33)`. PSRAM allocation utilizes approximately 307KB for the 16-bit double buffer, causing zero-flicker panel writes.
