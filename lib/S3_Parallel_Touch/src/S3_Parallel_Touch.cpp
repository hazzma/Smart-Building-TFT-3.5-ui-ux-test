#include "S3_Parallel_Touch.h"

S3ParallelTouch::S3ParallelTouch(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym, uint16_t rx)
    : _ts(xp, yp, xm, ym, rx), _xp(xp), _yp(yp), _xm(xm), _ym(ym) {
}

void S3ParallelTouch::begin(int minX, int maxX, int minY, int maxY) {
    _minX = minX;
    _maxX = maxX;
    _minY = minY;
    _maxY = maxY;
    
    // Default ADC resolution for ESP32S3 is 12-bit, but many calibrations use 10-bit.
    // It's recommended to call analogReadResolution(10) in your setup() for compatibility.
}

TouchPoint S3ParallelTouch::getPoint(uint16_t screenWidth, uint16_t screenHeight, uint8_t rotation) {
    TSPoint p = _ts.getPoint();
    
    // Restore bus pins so the display can receive data again
    restoreBusPins();

    TouchPoint tp;
    tp.pressure = p.z;
    tp.pressed = (p.z > _minPressure && p.z < _maxPressure);

    if (tp.pressed) {
        // Logika Mapping dari versi yang berhasil sebelumnya:
        // Swap XY: X pakai p.y, Y pakai p.x
        // Landscape (Rotation 1/3)
        if (rotation == 1 || rotation == 3) {
            tp.x = map(p.y, _minY, _maxY, screenWidth, 0); 
            tp.y = map(p.x, _minX, _maxX, 0, screenHeight);
        } else { // Portrait
            tp.x = map(p.x, _minX, _maxX, 0, screenWidth);
            tp.y = map(p.y, _minY, _maxY, 0, screenHeight);
        }
    } else {
        tp.x = -1;
        tp.y = -1;
    }

    return tp;
}

int S3ParallelTouch::getPressure() {
    TSPoint p = _ts.getPoint();
    restoreBusPins();
    return p.z;
}

void S3ParallelTouch::restoreBusPins() {
    // These pins are essential for the display bus to resume operation
    pinMode(_xm, OUTPUT);
    digitalWrite(_xm, HIGH);
    pinMode(_yp, OUTPUT);
    digitalWrite(_yp, HIGH);
    pinMode(_xp, OUTPUT);
    pinMode(_ym, OUTPUT);
}
