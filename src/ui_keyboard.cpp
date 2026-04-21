#include "ui_keyboard.h"
#include <string.h>

static char keyboard_buffer[64] = "";
static bool shift_mode = true; // Caps by default

const char* keys_row1_up[] = {"Q","W","E","R","T","Y","U","I","O","P"};
const char* keys_row1_low[] = {"q","w","e","r","t","y","u","i","o","p"};

const char* keys_row2_up[] = {"A","S","D","F","G","H","J","K","L"};
const char* keys_row2_low[] = {"a","s","d","f","g","h","j","k","l"};

const char* keys_row3_up[] = {"^","Z","X","C","V","B","N","M","<"};
const char* keys_row3_low[] = {"^","z","x","c","v","b","n","m","<"};

void keyboard_draw() {
    p_canvas->fillScreen(COLOR_BG_MAIN);
    p_canvas->fillRect(0, 0, 480, 320, COLOR_BG_MAIN);
    
    // Preview bar
    drawCardBase(10, 20, 460, 45, COLOR_CARD_BG);
    p_canvas->setTextColor(COLOR_TEXT_MAIN);
    p_canvas->setTextFont(4);
    p_canvas->setTextDatum(ML_DATUM);
    p_canvas->drawString(keyboard_buffer, 25, 42);
    
    // Keys
    int kX = 10, kY = 100; 
    int kW = 41, kH = 42; 
    int spacing = 6;

    p_canvas->setTextDatum(MC_DATUM);
    p_canvas->setTextFont(2);

    // Row 1
    for(int i=0; i<10; i++) {
        drawCardBase(kX + i*(kW+spacing), kY, kW, kH, COLOR_CARD_BG);
        p_canvas->drawString(shift_mode ? keys_row1_up[i] : keys_row1_low[i], kX + i*(kW+spacing) + kW/2, kY + kH/2);
    }
    
    // Row 2
    kY += kH + spacing;
    int off2 = (480 - (9 * (kW+spacing))) / 2;
    for(int i=0; i<9; i++) {
        drawCardBase(off2 + i*(kW+spacing), kY, kW, kH, COLOR_CARD_BG);
        p_canvas->drawString(shift_mode ? keys_row2_up[i] : keys_row2_low[i], off2 + i*(kW+spacing) + kW/2, kY + kH/2);
    }

    // Row 3 (Includes SHIFT ^ at index 0)
    kY += kH + spacing;
    int off3 = (480 - (9 * (kW+spacing))) / 2;
    for(int i=0; i<9; i++) {
        uint16_t color = COLOR_CARD_BG;
        if(i == 0) color = shift_mode ? COLOR_ACCENT_MAIN : COLOR_STAT_OFF; // SHIFT
        if(i == 8) color = COLOR_STAT_ERR; // Backspace
        
        drawCardBase(off3 + i*(kW+spacing), kY, kW, kH, color);
        p_canvas->drawString(shift_mode ? keys_row3_up[i] : keys_row3_low[i], off3 + i*(kW+spacing) + kW/2, kY + kH/2);
    }

    // Row 4: Cancel, Space, OK
    kY += kH + spacing;
    drawCardBase(10, kY, 90, kH, COLOR_STAT_OFF);
    p_canvas->drawString("CANCEL", 10 + 45, kY + kH/2);

    drawCardBase(110, kY, 200, kH, COLOR_CARD_BG);
    p_canvas->drawString("SPACE", 210, kY + kH/2);
    
    drawCardBase(320, kY, 150, kH, COLOR_STAT_ON);
    p_canvas->drawString("OK / ENTER", 320 + 75, kY + kH/2);

    p_canvas->setTextDatum(TL_DATUM);
}

static bool checkHit(int tx, int ty, int x, int y, int w, int h) {
    return (tx >= x && tx <= x + w && ty >= y && ty <= y + h);
}

void keyboard_handle_touch(int tx, int ty) {
    int kX = 10, kY = 100; 
    int kW = 41, kH = 42;
    int spacing = 6;

    // Row 1
    for(int i=0; i<10; i++) {
        if(checkHit(tx, ty, kX + i*(kW+spacing), kY, kW, kH)) {
            size_t len = strlen(keyboard_buffer);
            if(len < 63) {
                strcat(keyboard_buffer, shift_mode ? keys_row1_up[i] : keys_row1_low[i]);
            }
            return;
        }
    }

    // Row 2
    kY += kH + spacing;
    int off2 = (480 - (9 * (kW+spacing))) / 2;
    for(int i=0; i<9; i++) {
        if(checkHit(tx, ty, off2 + i*(kW+spacing), kY, kW, kH)) {
            size_t len = strlen(keyboard_buffer);
            if(len < 63) {
                strcat(keyboard_buffer, shift_mode ? keys_row2_up[i] : keys_row2_low[i]);
            }
            return;
        }
    }

    // Row 3
    kY += kH + spacing;
    int off3 = (480 - (9 * (kW+spacing))) / 2;
    for(int i=0; i<9; i++) {
        if(checkHit(tx, ty, off3 + i*(kW+spacing), kY, kW, kH)) {
            if(i == 0) { // SHIFT ^
                shift_mode = !shift_mode;
            } else if(i == 8) { // Backspace
                size_t len = strlen(keyboard_buffer);
                if(len > 0) keyboard_buffer[len-1] = '\0';
            } else {
                size_t len = strlen(keyboard_buffer);
                if(len < 63) strcat(keyboard_buffer, shift_mode ? keys_row3_up[i] : keys_row3_low[i]);
            }
            return;
        }
    }

    // Row 4
    kY += kH + spacing;
    if(checkHit(tx, ty, 10, kY, 90, kH)) { // Cancel
        keyboard_clear();
        return;
    }
    if(checkHit(tx, ty, 110, kY, 200, kH)) { // Space
        size_t len = strlen(keyboard_buffer);
        if(len < 63) strcat(keyboard_buffer, " ");
        return;
    }
}

const char* keyboard_get_text() {
    return keyboard_buffer;
}

void keyboard_set_text(const char* text) {
    if(text) strncpy(keyboard_buffer, text, 63);
    else keyboard_buffer[0] = '\0';
}

void keyboard_clear() {
    keyboard_buffer[0] = '\0';
}

void keyboard_toggle_shift() {
    shift_mode = !shift_mode;
}
