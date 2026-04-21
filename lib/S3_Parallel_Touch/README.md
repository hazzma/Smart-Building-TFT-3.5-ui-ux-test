# S3_Parallel_Touch

A specialized library for reading 8-bit resistive touch panels on ESP32-S3 boards where the touch pins (**XP, YP, XM, YM**) are shared with the display's 8-bit parallel bus pins (often **D6, WR, RS, D7**).

This library handles the delicate process of switching pin modes between the display bus and the ADC/Touch controller to ensure smooth operation without resetting the display controller.

## Features
- **Bus Restoration**: Automatically restores data bus pin modes after every touch read.
- **ESP32-S3 Optimized**: Designed specifically for S3 analog pins.
- **Easy Mapping**: Simple XY mapping for different rotations.

## Installation
Copy the `S3_Parallel_Touch` folder to your project's `lib` directory.

## Dependencies
- [Adafruit TouchScreen](https://github.com/adafruit/Adafruit_TouchScreen)

## Basic Usage

```cpp
#include <S3_Parallel_Touch.h>

// XP=D6, YP=WR, XM=RS, YM=D7
const int XP = 7, YP = 15, XM = 16, YM = 8; 
S3ParallelTouch touch(XP, YP, XM, YM);

void setup() {
    Serial.begin(115200);
    
    // Most calibrations from Arduino Uno use 10-bit resolution (0-1023)
    analogReadResolution(10); 
    
    // Set calibration based on your values (minX, maxX, minY, maxY)
    touch.begin(120, 950, 140, 940);
}

void loop() {
    TouchPoint p = touch.getPoint(320, 480, 1); // screenWidth, screenHeight, rotation
    
    if (p.pressed) {
        Serial.printf("X: %d, Y: %d\n", p.x, p.y);
    }
}
```

## Hardware Pin Mapping Reference
| Screen Pin | ESP32-S3 Pin | Touch Designation |
| :--- | :--- | :--- |
| D6 | 7 | **XP** |
| RS (DC) | 16 | **XM** |
| WR | 15 | **YP** |
| D7 | 8 | **YM** |
