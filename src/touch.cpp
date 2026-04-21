#include "touch.h"
#include "display.h"

// Singleton for touch management
S3ParallelTouch touchAgent(TOUCH_XP, TOUCH_YP, TOUCH_XM, TOUCH_YM);

void touch_init() {
    analogReadResolution(10); // Standard for these calibrations
    touchAgent.begin(120, 950, 140, 940);
    Serial.println("[TC] Touch initialized (10-bit ADC)");
}

static bool touch_was_pressed = false;
static uint32_t last_valid_press = 0;

bool touch_get_point(int &tx, int &ty) {
    // 1. Mandatory Bus Release
    tft.endWrite();
    digitalWrite(17, HIGH); // CS HIGH
    
    // 2. Read Touch via S3ParallelTouch library
    // The library internally calls restoreBusPins()
    TouchPoint p = touchAgent.getPoint(480, 320, 1);
    
    // 3. Re-enable TFT bus for TFT_eSPI
    tft.startWrite();
    
    // 4. Response logic
    if (p.pressed && p.pressure > 10) { 
        if (!touch_was_pressed && (millis() - last_valid_press > 80)) { 
            tx = p.x; ty = p.y;
            touch_was_pressed = true;
            last_valid_press = millis();
            
            // Critical Debug: See what coordinates we actually get
            Serial.printf("[TC] Hit! X:%d Y:%d (P:%d)\n", tx, ty, p.pressure);
            return true;
        }
    } else {
        touch_was_pressed = false;
    }
    
    return false;
}


