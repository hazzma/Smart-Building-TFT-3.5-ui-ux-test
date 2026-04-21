#include "MasterTouch.h"

/**
 * MASTER TOUCH - REUSABLE TOUCH MASTER AGENT LIBRARY
 * (c) 2026 Antigravity System
 */

MasterTouch::MasterTouch(uint8_t yp, uint8_t xm, uint8_t ym, uint8_t xp, uint16_t rx)
    : _ts(xp, yp, xm, ym, rx), _yp(yp), _xm(xm), _ym(ym), _xp(xp) {
    
    // Default calibration (standard 3.5" TFT shield)
    _minX = 120; _maxX = 950;
    _minY = 140; _maxY = 940;
    
    // Default Orientation (Standard Landscape Uno)
    _swapXY = true;
    _invertX = true;
    _invertY = false;
    
    // Default pressure
    _minP = 10;
    _maxP = 1000;
}

MasterPoint MasterTouch::read(uint16_t screenWidth, uint16_t screenHeight) {
    TSPoint p = _ts.getPoint();
    restorePins();

    if (p.z > _minP && p.z < _maxP) {
        int x_raw = p.x;
        int y_raw = p.y;
        
        // 1. SWAP X / Y if requested (Portrait vs Landscape)
        if (_swapXY) {
            int tmp = x_raw;
            x_raw = y_raw;
            y_raw = tmp;
        }
        
        // 2. Map to Screen Coordinates with Inversion Logic
        int outX, outY;
        
        if (_invertX) outX = map(x_raw, _minY, _maxY, screenWidth, 0);
        else          outX = map(x_raw, _minY, _maxY, 0, screenWidth);
        
        if (_invertY) outY = map(y_raw, _minX, _maxX, screenHeight, 0);
        else          outY = map(y_raw, _minX, _maxX, 0, screenHeight);
        
        return {outX, outY, p.z};
    }
    return {-1, -1, 0};
}

MasterPoint MasterTouch::readRaw() {
    TSPoint p = _ts.getPoint();
    restorePins();
    return {p.x, p.y, p.z};
}

void MasterTouch::setCalibration(int minX, int maxX, int minY, int maxY) {
    _minX = minX; _maxX = maxX;
    _minY = minY; _maxY = maxY;
}

void MasterTouch::setOrientation(bool swapXY, bool invertX, bool invertY) {
    _swapXY = swapXY;
    _invertX = invertX;
    _invertY = invertY;
}

void MasterTouch::setPressureLimit(int minP, int maxP) {
    _minP = minP;
    _maxP = maxP;
}

void MasterTouch::restorePins() {
    // Standard Arduino Pin Reset
    pinMode(_yp, OUTPUT);
    pinMode(_xm, OUTPUT);
    pinMode(_ym, OUTPUT);
    pinMode(_xp, OUTPUT);
}
