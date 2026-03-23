#ifndef UI_WIDGETS_H
#define UI_WIDGETS_H

#include "display.h"

extern lgfx::LGFX_Sprite canvas;

void widgets_init();
void drawTempCard(int x, int y, int w, int h, const char* label, float value, bool error);
void drawToggleButton(int x, int y, int w, int h, const char* label, bool state);
void drawUpDownButton(int x, int y, int w, int h, const char* label, bool isUp);
void drawPresenceBadge(int x, int y, int w, int h, bool detected);
void drawLuxCard(int x, int y, int w, int h, float lux);
void drawNotifBar(bool wifi, bool err, bool firebase, const char* room_name, const char* time_str);

// Helper for UI elements
void drawGradientRoundRect(int x, int y, int w, int h, int r, uint16_t colorTop, uint16_t colorBot);

#endif
