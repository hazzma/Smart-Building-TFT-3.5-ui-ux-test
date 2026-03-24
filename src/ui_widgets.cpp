#include "ui_widgets.h"
#include <stdio.h>

// ── Double Buffering Canvas ─────────────────────────────────────
lgfx::LGFX_Sprite canvas0(&lcd);
lgfx::LGFX_Sprite canvas1(&lcd);
lgfx::LGFX_Sprite *p_canvas = &canvas0;

void widgets_init() {
    // 480x320 @ 16bit = 307.2 KB per canvas (Total ~615 KB)
    // PSRAM is available (8MB). LovyanGFX automatically tries PSRAM for large sprites.
    
    canvas0.setColorDepth(16);
    if (!canvas0.createSprite(480, 320)) {
        Serial.println("CRITICAL: Failed to create canvas0 in PSRAM!");
    }
    
    canvas1.setColorDepth(16);
    if (!canvas1.createSprite(480, 320)) {
        Serial.println("CRITICAL: Failed to create canvas1 in PSRAM!");
    }
}

void widgets_swap() {
    p_canvas = (p_canvas == &canvas0) ? &canvas1 : &canvas0;
}

// ── Widget Drawing Helpers ─────────────────────────────────────
void drawCardBase(int x, int y, int w, int h, uint16_t color) {
    canvas.fillRoundRect(x, y, w, h, 8, color);
    canvas.drawRoundRect(x, y, w, h, 8, canvas.color565(40, 50, 80)); 
}

void drawTempCard(int x, int y, int w, int h, const char* label, float value, bool error) {
    drawCardBase(x, y, w, h, error ? COLOR_STAT_ERR : COLOR_CARD_BG);
    canvas.setTextColor(COLOR_TEXT_SEC);
    canvas.setFont(&fonts::Font2);
    canvas.setTextDatum(top_center);
    canvas.drawString(label, x + w / 2, y + 8);
    canvas.setTextColor(COLOR_TEXT_MAIN);
    canvas.setFont(&fonts::Font4); 
    canvas.setTextDatum(middle_center);
    char buf[16];
    if (error) sprintf(buf, "ERR");
    else sprintf(buf, "%.1f", value);
    canvas.drawString(buf, x + w / 2, y + h / 2 + 5);
    if (!error) {
        canvas.setTextColor(COLOR_ACCENT_MAIN);
        canvas.setFont(&fonts::Font2);
        canvas.setTextDatum(bottom_center);
        canvas.drawString("~C", x + w / 2, y + h - 5); 
    }
}

void drawToggleButton(int x, int y, int w, int h, const char* label, bool state) {
    uint16_t bgColor = state ? COLOR_STAT_ON : COLOR_STAT_OFF;
    uint16_t accent = state ? canvas.color565(220, 255, 240) : COLOR_TEXT_SEC;
    drawCardBase(x, y, w, h, bgColor);
    canvas.setTextColor(accent);
    canvas.setFont(&fonts::Font2);
    canvas.setTextDatum(top_center);
    canvas.drawString(label, x + w / 2, y + 6);
    canvas.setTextColor(COLOR_TEXT_MAIN);
    canvas.setFont(&fonts::Font2);
    canvas.setTextDatum(bottom_center);
    canvas.drawString(state ? "ON" : "OFF", x + w / 2, y + h - 6);
}

void drawUpDownButton(int x, int y, int w, int h, const char* label, bool isUp) {
    drawCardBase(x, y, w, h, COLOR_CARD_BG);
    canvas.setTextColor(COLOR_ACCENT_SEC);
    canvas.setFont(&fonts::Font4);
    canvas.setTextDatum(middle_center);
    canvas.drawString(isUp ? "^" : "v", x + w / 2, y + h / 2 - 5);
    canvas.setTextColor(COLOR_TEXT_SEC);
    canvas.setFont(&fonts::Font2);
    canvas.setTextDatum(bottom_center);
    canvas.drawString(label, x + w / 2, y + h - 4);
}

void drawPresenceBadge(int x, int y, int w, int h, bool detected) {
    drawCardBase(x, y, w, h, COLOR_CARD_BG);
    canvas.setTextColor(COLOR_TEXT_SEC);
    canvas.setFont(&fonts::Font2);
    canvas.setTextDatum(top_center);
    canvas.drawString("Presence", x + w / 2, y + 6);
    uint16_t dotColor = detected ? COLOR_STAT_ON : COLOR_STAT_OFF;
    canvas.fillCircle(x + 20, y + h - 15, 6, dotColor);
    canvas.setTextColor(detected ? COLOR_TEXT_MAIN : COLOR_TEXT_SEC);
    canvas.setTextDatum(middle_left);
    canvas.drawString(detected ? "Detected" : "Clear", x + 35, y + h - 15);
}

void drawLuxCard(int x, int y, int w, int h, float lux) {
    drawCardBase(x, y, w, h, COLOR_CARD_BG);
    canvas.setTextColor(COLOR_TEXT_SEC);
    canvas.setFont(&fonts::Font2);
    canvas.setTextDatum(top_center);
    canvas.drawString("Avg Lux", x + w / 2, y + 6);
    canvas.setTextColor(COLOR_ACCENT_MAIN);
    canvas.setFont(&fonts::Font4);
    canvas.setTextDatum(middle_center);
    char buf[16];
    sprintf(buf, "%.0f", lux);
    canvas.drawString(buf, x + w / 2, y + h / 2 + 5);
    canvas.setTextColor(COLOR_TEXT_SEC);
    canvas.setFont(&fonts::Font2);
    canvas.setTextDatum(bottom_center);
    canvas.drawString("lx", x + w / 2, y + h - 4);
}

void drawNotifBar(bool wifi, bool err, bool firebase, const char* room_name, const char* time_str) {
    canvas.fillRect(0, 0, 480, 28, COLOR_CARD_BG);
    canvas.drawFastHLine(0, 28, 480, COLOR_ACCENT_MAIN);
    canvas.fillRect(10, 7, 20, 3, COLOR_ACCENT_MAIN);
    canvas.fillRect(10, 13, 20, 3, COLOR_ACCENT_MAIN);
    canvas.fillRect(10, 19, 20, 3, COLOR_ACCENT_MAIN);
    canvas.setTextColor(COLOR_TEXT_MAIN);
    canvas.setFont(&fonts::Font2);
    canvas.setTextDatum(middle_left);
    canvas.drawString(room_name, 45, 14);
    int rightX = 470;
    canvas.setTextDatum(middle_right);
    canvas.setTextColor(COLOR_TEXT_MAIN);
    canvas.drawString(time_str, rightX, 14);
    rightX -= 55;
    canvas.setTextColor(firebase ? COLOR_STAT_ON : COLOR_STAT_ERR);
    canvas.drawString("FB", rightX, 14);
    rightX -= 35;
    if (err) {
        canvas.setTextColor(COLOR_STAT_WARN);
        canvas.drawString("WARN", rightX, 14);
        rightX -= 50;
    }
    canvas.setTextColor(wifi ? COLOR_ACCENT_SEC : COLOR_STAT_ERR);
    canvas.drawString("WiFi", rightX, 14);
}
