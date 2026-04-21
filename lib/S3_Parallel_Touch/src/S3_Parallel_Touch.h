#ifndef S3_PARALLEL_TOUCH_H
#define S3_PARALLEL_TOUCH_H

#include <Arduino.h>
#include <TouchScreen.h>

/**
 * S3_Parallel_Touch - Library for 8-bit Parallel Resistive Touch on ESP32-S3
 * 
 * Specifically designed for displays that share pins between the 8-bit parallel bus
 * and the resistive touch panel (XP, YP, XM, YM).
 */

struct TouchPoint {
    int x;
    int y;
    int pressure;
    bool pressed;
};

class S3ParallelTouch {
public:
    S3ParallelTouch(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym, uint16_t rx = 300);

    void begin(int minX = 120, int maxX = 950, int minY = 140, int maxY = 940);
    
    // Reads the touch point and automatically restores pin modes for the display bus
    TouchPoint getPoint(uint16_t screenWidth, uint16_t screenHeight, uint8_t rotation = 1);

    // Raw pressure reading
    int getPressure();

private:
    TouchScreen _ts;
    uint8_t _xp, _yp, _xm, _ym;
    int _minX, _maxX, _minY, _maxY;
    int _minPressure = 10;
    int _maxPressure = 1000;

    void restoreBusPins();
};

#endif
