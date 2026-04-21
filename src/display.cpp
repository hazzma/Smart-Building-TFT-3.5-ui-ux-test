#include "display.h"

TFT_eSPI tft = TFT_eSPI();
SemaphoreHandle_t bus_mutex = NULL;

void display_init() {
    bus_mutex = xSemaphoreCreateMutex();
    tft.init();
    tft.setRotation(1); // Landscape
    tft.invertDisplay(false); // Panels differ, Bos's panel is Normal
    tft.fillScreen(COLOR_BG_MAIN);
}



void display_brightness(uint8_t val) {
    // Backlight implementation
}
