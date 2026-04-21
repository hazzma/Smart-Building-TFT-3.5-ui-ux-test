#include "display.h"

TFT_eSPI tft = TFT_eSPI();

void display_init() {
    tft.init();
    tft.setRotation(1); // Landscape
    tft.invertDisplay(false); // Panels differ, Bos's panel is Normal
    tft.fillScreen(COLOR_BG_MAIN);
}



void display_brightness(uint8_t val) {
    // Backlight implementation
}
