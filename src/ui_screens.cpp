#include "ui_screens.h"
#include "ui_widgets.h"

static ScreenState current_screen = SCREEN_DASHBOARD;
static char time_buffer[10] = "12:34"; // Dummy NTP time

void screens_init() {
    widgets_init();
}

void screens_set(ScreenState s) {
    current_screen = s;
}

void drawDashboard(BuildingState& state, int fps) {
    // Fill background with subtle dark gradient (Premium feel)
    for(int y = 0; y < 320; y++) {
        uint16_t c = canvas.color565(5, 5 + y/40, 15 + y/20); 
        canvas.drawFastHLine(0, y, 480, c);
    }
    
    // Snapshot of current data using Mutex
    data_lock(state);
    SensorData s = state.sensor;
    NetworkState n = state.net;
    data_unlock(state);
    
    // 1. Notification Bar (y=0 to 28)
    drawNotifBar(n.wifi_connected, s.sensor_error[0], n.firebase_ok, n.room_name, time_buffer);
    
    // 2. Temp Cards at the top (y=36)
    drawTempCard(8,   36, 110, 80, "SENSOR 1", s.temp[0], s.sensor_error[0]);
    drawTempCard(124, 36, 110, 80, "SENSOR 2", s.temp[1], s.sensor_error[1]);
    drawTempCard(240, 36, 110, 80, "SENSOR 3", s.temp[2], s.sensor_error[2]);
    drawTempCard(356, 36, 115, 80, "SENSOR 4", s.temp[3], s.sensor_error[3]);
    
    // Left side column (Target Temp, AC, Up/Down, Projector)
    // Target Temp Box
    canvas.fillRoundRect(8, 126, 226, 32, 6, COLOR_CARD_BG);
    canvas.drawRoundRect(8, 126, 226, 32, 6, COLOR_ACCENT_SEC);
    canvas.setTextColor(COLOR_TEXT_MAIN);
    canvas.setFont(&fonts::Font2);
    canvas.setTextDatum(middle_center);
    char buf[32];
    sprintf(buf, "Target Suhu: %.1f C", s.temp_target);
    canvas.drawString(buf, 8 + 113, 126 + 16);
    
    // AC Toggle
    drawToggleButton(8, 166, 226, 42, "AC MASTER", s.ac_on);
    
    // Temp Up / Down
    drawUpDownButton(8,  216, 108, 42, "NAIK", true);
    drawUpDownButton(126, 216, 108, 42, "TURUN", false);
    
    // Projector Option (Below Up/Down)
    drawToggleButton(8, 266, 226, 42, "PROJECTOR", s.projector_on);
    
    // Right side column (Lights, Presence, Lux)
    drawToggleButton(240, 126, 230, 42, "LIGHTS", s.light_on);
    drawPresenceBadge(240, 176, 230, 42, s.human_presence);
    drawLuxCard(240, 226, 230, 82, s.lux_avg); // expanded height to look filled
    
    // FPS in bottom left corner overlay
    canvas.setTextColor(COLOR_TEXT_SEC);
    canvas.setFont(&fonts::Font0); // Tiny font
    canvas.setTextDatum(bottom_left);
    char fpsBuf[16];
    sprintf(fpsBuf, "FPS: %d", fps);
    canvas.drawString(fpsBuf, 4, 316);
}

void screens_render(BuildingState& state, int fps) {
    if (current_screen == SCREEN_DASHBOARD) {
        drawDashboard(state, fps);
    }
}
