#include "ui_widgets.h"
#include "data.h"
#include <stdio.h>

// ── Double Buffering Canvas ─────────────────────────────────────
TFT_eSprite canvas0 = TFT_eSprite(&tft);
TFT_eSprite canvas1 = TFT_eSprite(&tft);
TFT_eSprite *p_canvas = &canvas0;

void widgets_init() {
    Serial.println("\n[MEM] --- Display Buffer Audit ---");
    
    canvas0.setColorDepth(16);
    void* ptr0 = canvas0.createSprite(480, 320);
    if (ptr0) {
        if (esp_ptr_external_ram(ptr0)) {
            Serial.println("[MEM] Canvas 0: Allocated in PSRAM ✓");
        } else {
            Serial.println("[MEM] Canvas 0: Allocated in INTERNAL RAM ⚠");
        }
    } else {
        Serial.println("[MEM] Canvas 0: FAILED TO ALLOCATE!");
    }
    
    canvas1.setColorDepth(16);
    void* ptr1 = canvas1.createSprite(480, 320);
    if (ptr1) {
        if (esp_ptr_external_ram(ptr1)) {
            Serial.println("[MEM] Canvas 1: Allocated in PSRAM ✓");
        } else {
            Serial.println("[MEM] Canvas 1: Allocated in INTERNAL RAM ⚠");
        }
    } else {
        Serial.println("[MEM] Canvas 1: FAILED TO ALLOCATE!");
    }
    
    Serial.printf("[MEM] Free PSRAM: %d KB\n", ESP.getFreePsram() / 1024);
    Serial.println("[MEM] --------------------------\n");
    
    canvas0.fillScreen(COLOR_BG_MAIN);
    canvas1.fillScreen(COLOR_BG_MAIN);
}


void widgets_swap() {
    p_canvas = (p_canvas == &canvas0) ? &canvas1 : &canvas0;
}

// ── Widget Implementations ──────────────────────────────────────

void drawCardBase(int x, int y, int w, int h, uint16_t color) {
    p_canvas->fillRoundRect(x, y, w, h, CARD_RAD, color);
    p_canvas->drawRoundRect(x, y, w, h, CARD_RAD, p_canvas->color565(40, 50, 80)); // Border
}

void drawTempCard(int x, int y, int w, int h, const char* label, float temp, bool error) {
    drawCardBase(x, y, w, h, COLOR_CARD_BG);
    
    p_canvas->setTextColor(COLOR_TEXT_SEC);
    p_canvas->setTextFont(2);
    p_canvas->drawString(label, x + 8, y + 8);
    
    if (error) {
        p_canvas->setTextColor(COLOR_STAT_ERR);
        p_canvas->drawString("ERROR", x + 8, y + 35);
    } else if (temp < -50.0f) {
        p_canvas->setTextColor(COLOR_TEXT_SEC);
        p_canvas->setTextFont(4);
        p_canvas->drawString("NULL", x + 8, y + 32);
    } else {
        p_canvas->setTextColor(COLOR_ACCENT_MAIN);
        p_canvas->setTextFont(4);
        char val_buf[16];
        snprintf(val_buf, sizeof(val_buf), "%.1f", temp);
        p_canvas->drawString(val_buf, x + 8, y + 32);
        
        // Simbol C kecil agar tidak numpuk
        int off_x = p_canvas->textWidth(val_buf);
        p_canvas->setTextFont(2);
        p_canvas->drawString("C", x + 8 + off_x + 4, y + 35);
    }
}

void drawToggleButton(int x, int y, int w, int h, const char* label, bool state) {
    uint16_t btnColor = state ? COLOR_STAT_ON : COLOR_STAT_OFF;
    drawCardBase(x, y, w, h, COLOR_CARD_BG);
    
    // Switch background
    int swW = 40, swH = 20;
    int swX = x + w - swW - 10;
    int swY = y + (h - swH) / 2;
    p_canvas->fillRoundRect(swX, swY, swW, swH, 10, p_canvas->color565(30, 40, 60));
    
    // Switch knob
    int kR = 8;
    int kX = state ? (swX + swW - kR - 2) : (swX + kR + 2);
    canvas.fillCircle(kX, swY + swH/2, kR, btnColor);
    
    canvas.setTextColor(COLOR_TEXT_MAIN);
    canvas.setTextFont(2);
    canvas.setTextDatum(ML_DATUM);
    canvas.drawString(label, x + 12, y + h/2);
    canvas.setTextDatum(TL_DATUM); // Reset
}

void drawUpDownButton(int x, int y, int w, int h, const char* label, bool isUp) {
    drawCardBase(x, y, w, h, COLOR_CARD_BG);
    
    canvas.setTextFont(2);
    canvas.setTextDatum(MC_DATUM);
    canvas.drawString(label, x + w/2, y + h/2);
    canvas.setTextDatum(TL_DATUM);
}

void drawPresenceBadge(int x, int y, int w, int h, bool detected) {
    drawCardBase(x, y, w, h, COLOR_CARD_BG);
    
    canvas.setTextColor(COLOR_TEXT_SEC);
    p_canvas->setTextColor(COLOR_TEXT_SEC);
    p_canvas->setTextFont(2);
    p_canvas->drawString("Presence", x + 10, y + 8);
    
    uint16_t dotColor = detected ? COLOR_STAT_ON : COLOR_STAT_OFF;
    p_canvas->fillCircle(x + 18, y + 35, 6, dotColor);
    
    p_canvas->setTextColor(COLOR_TEXT_MAIN);
    p_canvas->drawString(detected ? "Detected" : "Empty", x + 35, y + 27);
}

void drawCO2Card(int x, int y, int w, int h, int co2) {
    uint16_t valColor = COLOR_STAT_ON; // Green
    const char* status = "GOOD";
    
    if (co2 > 1500) {
        valColor = COLOR_STAT_ERR; // Red
        status = "POOR";
    } else if (co2 > 1000) {
        valColor = COLOR_STAT_WARN; // Yellow
        status = "FAIR";
    }

    drawCardBase(x, y, w, h, COLOR_CARD_BG);
    
    p_canvas->setTextFont(2);
    p_canvas->setTextColor(COLOR_TEXT_SEC);
    p_canvas->setTextDatum(TL_DATUM);
    p_canvas->drawString("CO2", x + 10, y + 8);
    
    p_canvas->setTextColor(valColor);
    p_canvas->setTextFont(4);
    p_canvas->setTextDatum(MC_DATUM);
    char buf[16];
    if (co2 == -1) {
        strcpy(buf, "NULL");
        p_canvas->setTextColor(COLOR_TEXT_SEC);
    } else {
        snprintf(buf, 16, "%d", co2);
    }
    p_canvas->drawString(buf, x + w/2, y + h/2 + 5);
    
    p_canvas->setTextFont(2);
    p_canvas->setTextColor(COLOR_TEXT_SEC);
    p_canvas->drawString("ppm", x + w - 25, y + h - 15);

    // Status dot
    p_canvas->fillCircle(x + w - 15, y + 15, 4, valColor);
}

void drawNotifBar(bool wifi, bool lan, bool mqtt, const char* conn_status, const char* room_name, const char* time_str) {
    p_canvas->fillRect(0, 0, 480, 28, COLOR_BG_MAIN);
    p_canvas->drawFastHLine(0, 27, 480, p_canvas->color565(40, 50, 80));
    
    p_canvas->setTextColor(COLOR_TEXT_MAIN);
    p_canvas->setTextFont(2);
    
    // Hamburger Icon (3 Lines)
    int hamX = 10;
    int hamY = 14;
    p_canvas->fillRect(hamX, hamY - 6, 20, 3, COLOR_TEXT_MAIN);
    p_canvas->fillRect(hamX, hamY, 20, 3, COLOR_TEXT_MAIN);
    p_canvas->fillRect(hamX, hamY + 6, 20, 3, COLOR_TEXT_MAIN);

    // Clock (Center)
    p_canvas->setTextColor(COLOR_ACCENT_MAIN);
    p_canvas->setTextFont(4);
    p_canvas->setTextDatum(MC_DATUM);
    p_canvas->drawString(time_str, 240, 14);
    
    // Connection Status Text (Left of Clock)
    p_canvas->setTextFont(2);
    if (conn_status && strlen(conn_status) > 0) {
        if (strstr(conn_status, "Fail") != nullptr || strstr(conn_status, "DISCONNECTED") != nullptr) {
            p_canvas->setTextColor(COLOR_STAT_ERR);
        } else if (strstr(conn_status, "Connecting") != nullptr) {
            p_canvas->setTextColor(p_canvas->color565(255, 165, 0)); 
        } else {
            p_canvas->setTextColor(COLOR_STAT_ON);
        }
        p_canvas->setTextDatum(MR_DATUM);
        p_canvas->drawString(conn_status, 190, 14);
    }
    
    // Presence Indicator (Right of Clock)
    if (g_state.sensor.human_presence) {
        p_canvas->setTextColor(COLOR_STAT_ON);
        p_canvas->setTextDatum(ML_DATUM);
        p_canvas->drawString("PRES", 290, 14);
    }
    
    p_canvas->setTextDatum(TL_DATUM); // Reset
    
    // Status Icons: WiFi, LAN, MQTT
    int iconX = 390; 
    p_canvas->fillCircle(iconX, 14, 4, wifi ? COLOR_STAT_ON : COLOR_STAT_ERR);
    p_canvas->fillCircle(iconX + 12, 14, 4, lan ? COLOR_STAT_ON : COLOR_STAT_ERR);
    p_canvas->fillCircle(iconX + 24, 14, 4, mqtt ? COLOR_STAT_ON : COLOR_STAT_ERR);
}

void drawLuxCard(int x, int y, int w, int h, float lux) {
    drawCardBase(x, y, w, h, COLOR_CARD_BG);
    
    canvas.setTextFont(2);
    canvas.drawString("Luminance", x + 10, y + 8);
    
    char buf[16];
    if (lux < 0) {
        strcpy(buf, "NULL");
        canvas.setTextColor(COLOR_TEXT_SEC);
    } else {
        snprintf(buf, sizeof(buf), "%.0f lx", lux);
        canvas.setTextColor(COLOR_TEXT_MAIN);
    }
    
    canvas.setTextFont(4);
    canvas.setTextDatum(MC_DATUM);
    canvas.drawString(buf, x + w/2, y + h/2 + 5);
    canvas.setTextDatum(TL_DATUM);
}

