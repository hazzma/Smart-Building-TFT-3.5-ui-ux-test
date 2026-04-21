#ifndef UI_WIDGETS_H
#define UI_WIDGETS_H

#include "display.h"

// FSD 4.2: Double-buffered sprites in PSRAM (using TFT_eSPI)
extern TFT_eSprite canvas0;
extern TFT_eSprite canvas1;
extern TFT_eSprite *p_canvas;

// Macro to keep old code working: 'canvas' now points to the active buffer
#define canvas (*p_canvas)

void widgets_init();
void widgets_swap();

// Design tokens used by widgets
#define CARD_RAD 8

// Widget prototypes
void drawCardBase(int x, int y, int w, int h, uint16_t color);
void drawTempCard(int x, int y, int w, int h, const char* label, float value, bool error);
void drawToggleButton(int x, int y, int w, int h, const char* label, bool state);
void drawUpDownButton(int x, int y, int w, int h, const char* label, bool isUp);
void drawPresenceBadge(int x, int y, int w, int h, bool detected);
void drawLuxCard(int x, int y, int w, int h, float lux);
void drawCO2Card(int x, int y, int w, int h, int co2);
void drawNotifBar(bool wifi, bool lan, bool mqtt, const char* conn_status, const char* room_name, const char* time_str);

#endif
