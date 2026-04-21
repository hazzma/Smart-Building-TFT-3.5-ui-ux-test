#ifndef MASTER_TOUCH_H
#define MASTER_TOUCH_H

#include <Arduino.h>
#include <TouchScreen.h>

/**
 * MasterTouch - A "Master Agent" oriented library for resistive touchscreens.
 * Designed particularly for 8-bit parallel TFT shields with shared pins.
 * Handles automatic pin-state restoration and flexible coordinate mapping.
 */

struct MasterPoint {
    int x, y, z;
};

class MasterTouch {
public:
    /**
     * @param yp, xm, ym, xp Shared touchscreen pins
     * @param rx Resistance across X plates (typical 300)
     */
    MasterTouch(uint8_t yp, uint8_t xm, uint8_t ym, uint8_t xp, uint16_t rx = 300);
    
    /**
     * Reads mapped screen coordinates (0 to screenWidth/Height)
     * Automatically handles pin-restoration for shared bus stability.
     */
    MasterPoint read(uint16_t screenWidth, uint16_t screenHeight);
    
    /**
     * Reads raw ADC values from the panel (0-1023)
     */
    MasterPoint readRaw();
    
    /**
     * Sets the physical calibration boundaries
     */
    void setCalibration(int minX, int maxX, int minY, int maxY);
    
    /**
     * Configure orientation logic (Swap/Invert) for different rotations/boards
     */
    void setOrientation(bool swapXY, bool invertX, bool invertY);
    
    /**
     * Sets touch sensitivity (Pressure)
     */
    void setPressureLimit(int minP, int maxP);

private:
    TouchScreen _ts;
    uint8_t _yp, _xm, _ym, _xp;
    int _minX, _maxX, _minY, _maxY;
    int _minP, _maxP;
    bool _swapXY, _invertX, _invertY;
    
    void restorePins();
};

#endif
