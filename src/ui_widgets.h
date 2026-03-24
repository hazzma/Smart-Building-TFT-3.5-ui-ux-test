#ifndef UI_WIDGETS_H
#define UI_WIDGETS_H

#include "display.h"
#include <LovyanGFX.hpp>

extern lgfx::LGFX_Sprite canvas;

void widgets_init();

// Design tokens used by widgets
#define CARD_RAD 8

// Widget prototypes
void drawCardBase(int x, int y, int w, int h, uint16_t color);
void drawTempCard(int x, int y, int w, int h, const char* label, float value, bool error);
void drawToggleButton(int x, int y, int w, int h, const char* label, bool state);
void drawUpDownButton(int x, int y, int w, int h, const char* label, bool isUp);
void drawPresenceBadge(int x, int y, int w, int h, bool detected);
void drawLuxCard(int x, int y, int w, int h, float lux);
void drawNotifBar(bool wifi, bool err, bool firebase, const char* room_name, const char* time_str);

#endif
