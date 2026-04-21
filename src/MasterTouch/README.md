# MasterTouch Agent Library

A premium "Master Agent" oriented library for resistive touchscreens, specifically optimized for parallel TFT shields where touchscreen pins are shared with the display data bus.

## 🚀 Key Features

- **Automatic Pin Restoration**: Ensuring shared pins are reset to `OUTPUT` after every touch read, preventing display glitching.
- **Cross-Platform**: Designed for PlatformIO, compatible with Arduino Uno, ESP32, and other microcontrollers.
- **Flexible Mapping**: Easily swap axes or invert directions to match any orientation without complex math.

## 📡 Multiplexed Pin Architecture

In typical parallel shields, pins are shared between the **LCD Controller** and the **Resistive Touch Panel**. This library acts as an agent that manages the "hand-over" between these two systems.

| Signal Type | LCD Function | Touch Panel Function | Note |
| :--- | :--- | :--- | :--- |
| **Control** | LCD Write (WR) | Touch Y+ (YP) | Shared Multiplexed |
| **Control** | LCD Command/Data (CD) | Touch X- (XM) | Shared Multiplexed |
| **Data Bus** | Data Bit 7 (DB7) | Touch Y- (YM) | Shared Multiplexed |
| **Data Bus** | Data Bit 6 (DB6) | Touch X+ (XP) | Shared Multiplexed |
| **Control** | LCD Read (RD) | - | Dedicated |
| **Control** | LCD Chip Select (CS) | - | Dedicated |

## 🛠️ Usage Example

```cpp
#include <MasterTouch.h>

// Initialize with shared pins (YP, XM, YM, XP)
MasterTouch touchAgent(A1, A2, 7, 6);

void setup() {
    // 1. Set physical calibration (Raw ADC values)
    touchAgent.setCalibration(120, 980, 140, 940);
    
    // 2. Set screen orientation (SwapXY, InvertX, InvertY)
    touchAgent.setOrientation(true, true, false); 
}

void loop() {
    // 3. Read processed coordinates
    MasterPoint p = touchAgent.read(480, 320);

    if (p.z > 0) {
        Serial.print("Touch: "); Serial.print(p.x); Serial.println(p.y);
    }
}
```

## 📐 Calibration Guide

1. Use `readRaw()` to find the ADC values (0-1023) of the screen edges.
2. Update `setCalibration()` with those values to ensure high precision.
3. Use `setOrientation()` to align the touch movement with your graphical rotation.

---
*Created by Antigravity Agentic System*
