#include "ui_screens.h"
#include "ui_widgets.h"
#include "ui_keyboard.h"
#include "wifi_manager.h"
#include "lan_manager.h"

static ScreenState current_screen = SCREEN_DASHBOARD;
static int editing_target = 0; // 0=None, 1=SSID, 2=PW
static char wifi_ssid[32] = "han";
static char wifi_pass[32] = "hanhanhan";
static bool show_password = false;

static char temp_lan_ip[16] = "192.168.1.177";
static char temp_lan_gw[16] = "192.168.1.1";
static char temp_lan_sn[16] = "255.255.255.0";

void screens_init() {
    widgets_init();
}

void render_dashboard(BuildingState& state, int fps) {
    p_canvas->fillScreen(COLOR_BG_MAIN);
    p_canvas->fillRect(0, 0, 480, 320, COLOR_BG_MAIN);
    
    // ── 1. Top Notification Bar ─────────────────────────────────────
    const char* active_status = state.net.conn_status;
    if (state.net.net_priority == 1) {
        active_status = state.net.lan_connected ? "LAN OK" : "LAN FAIL";
    } else {
        if (state.net.wifi_connected) active_status = "WIFI OK";
    }

    drawNotifBar(state.net.wifi_connected, state.net.lan_connected, state.net.mqtt_ok, active_status, state.net.room_name, state.net.time_str);

    // ── 2. Dashboard Content (Split into 2 Pages) ───────────────────
    if (state.dashboard_page == 0) {
        // --- PAGE 1: AC CONTROL & 4 SENSORS ---
        drawCardBase(10, 40, 220, 245, COLOR_CARD_BG);
        canvas.setTextColor(COLOR_TEXT_SEC);
        canvas.setTextFont(2);
        canvas.drawString("AC CONTROL", 25, 55);
        
        canvas.setTextColor(COLOR_ACCENT_MAIN);
        canvas.setTextFont(6);
        char target_buf[16];
        snprintf(target_buf, sizeof(target_buf), "%.1f", state.sensor.temp_target);
        canvas.drawString(target_buf, 35, 80);
        
        int offset_x = canvas.textWidth(target_buf);
        canvas.setTextFont(2);
        canvas.drawString("o", 35 + offset_x + 4, 80);
        canvas.setTextFont(4);
        canvas.drawString("C", 35 + offset_x + 14, 85);

        drawUpDownButton(25, 140, 90, 60, "UP", true);
        drawUpDownButton(125, 140, 90, 60, "DOWN", false);
        drawToggleButton(25, 215, 190, 55, "AC POWER", state.sensor.ac_on);

        // Right side: 4 Suhu
        drawTempCard(240, 40, 110, 75, "Suhu 1", state.sensor.temp[0], state.sensor.sensor_error[0]);
        drawTempCard(360, 40, 110, 75, "Suhu 2", state.sensor.temp[1], state.sensor.sensor_error[1]);
        drawTempCard(240, 125, 110, 75, "Suhu 3", state.sensor.temp[2], state.sensor.sensor_error[2]);
        drawTempCard(360, 125, 110, 75, "Suhu 4", state.sensor.temp[3], state.sensor.sensor_error[3]);
        
        // Quick Toggles
        drawToggleButton(240, 210, 110, 75, "Projector", state.sensor.projector_on);
        drawToggleButton(360, 210, 110, 75, "LED CTL", state.sensor.light_on);
    } 
    else {
        // --- PAGE 2: ENVIRONMENT OVERVIEW ---
        drawCO2Card(10, 45, 225, 115, state.sensor.co2);
        drawLuxCard(245, 45, 225, 115, state.sensor.lux);
        
        drawPresenceBadge(10, 170, 225, 115, state.sensor.human_presence);
        
        // Avg Temp Calculation
        float sum = 0; int count = 0;
        for(int i=0; i<4; i++) {
            if(!state.sensor.sensor_error[i] && state.sensor.temp[i] > -50.0f) {
                sum += state.sensor.temp[i]; count++;
            }
        }
        float avg = (count > 0) ? (sum / count) : -100.0f;
        drawTempCard(245, 170, 225, 115, "AVERAGE TEMP", avg, avg < -50.0f);
    }

    // ── 3. Footer: Pagination & Stats ───────────────────────────────
    canvas.setTextColor(COLOR_TEXT_SEC);
    canvas.setTextFont(1);
    char footer_buf[64];
    snprintf(footer_buf, sizeof(footer_buf), "FPS: %d | PAGE %d/2 | S3 TURBO", fps, state.dashboard_page + 1);
    canvas.drawString(footer_buf, 10, 305);
    
    // Page Indicators (Dots)
    int dot1 = (state.dashboard_page == 0) ? COLOR_ACCENT_MAIN : COLOR_CARD_BG;
    int dot2 = (state.dashboard_page == 1) ? COLOR_ACCENT_MAIN : COLOR_CARD_BG;
    p_canvas->fillCircle(230, 305, 4, dot1);
    p_canvas->fillCircle(250, 305, 4, dot2);

    // NEXT button
    drawCardBase(380, 285, 90, 30, COLOR_CARD_BG);
    p_canvas->setTextDatum(MC_DATUM);
    p_canvas->setTextColor(COLOR_TEXT_MAIN);
    p_canvas->drawString("PAGE >>", 425, 300);
    p_canvas->setTextDatum(TL_DATUM);
}

void render_settings(BuildingState& state) {
    p_canvas->fillScreen(COLOR_BG_MAIN);
    p_canvas->fillRect(0, 0, 480, 320, COLOR_BG_MAIN);
    
    // Header
    canvas.setTextColor(COLOR_TEXT_MAIN);
    canvas.setTextFont(4);
    canvas.drawString("SETTINGS", 20, 10);
    canvas.drawFastHLine(0, 45, 480, COLOR_ACCENT_MAIN);
    
    // Priority Switch
    p_canvas->setTextColor(COLOR_TEXT_SEC);
    p_canvas->setTextFont(2);
    p_canvas->drawString("Connection Priority:", 20, 60);
    drawToggleButton(20, 80, 440, 55, state.net.net_priority == 0 ? "PRIORITY: WIFI" : "PRIORITY: LAN", state.net.net_priority == 1);

    // Navigation Options
    drawCardBase(20, 150, 210, 60, COLOR_CARD_BG);
    p_canvas->setTextColor(COLOR_TEXT_MAIN);
    p_canvas->setTextFont(2);
    p_canvas->setTextDatum(MC_DATUM);
    p_canvas->drawString("WiFi Setup", 125, 180);

    drawCardBase(250, 150, 210, 60, COLOR_CARD_BG);
    p_canvas->drawString("LAN Setup", 355, 180);
    
    // Back button
    drawCardBase(20, 250, 120, 50, COLOR_STAT_OFF);
    p_canvas->drawString("BACK", 80, 275);
    
    // Quick LAN Status
    p_canvas->setTextFont(2);
    p_canvas->setTextColor(COLOR_TEXT_SEC);
    p_canvas->setTextDatum(MR_DATUM);
    p_canvas->drawString(state.net.lan_status_detail, 460, 275);
    p_canvas->setTextDatum(TL_DATUM);
}

void render_lan_config(BuildingState& state) {
    p_canvas->fillScreen(COLOR_BG_MAIN);
    p_canvas->fillRect(0, 0, 480, 320, COLOR_BG_MAIN);
    
    p_canvas->setTextColor(COLOR_TEXT_MAIN);
    p_canvas->setTextFont(4);
    p_canvas->drawString("LAN CONFIGURATION", 20, 10);
    p_canvas->drawFastHLine(0, 45, 480, COLOR_ACCENT_MAIN);
    
    // DHCP Toggle
    p_canvas->setTextFont(2);
    p_canvas->setTextColor(COLOR_TEXT_SEC);
    p_canvas->drawString("Mode:", 20, 60);
    drawToggleButton(20, 80, 440, 50, state.net.lan_use_dhcp ? "DHCP (Automatic)" : "STATIC (Manual)", state.net.lan_use_dhcp);
    
    if (!state.net.lan_use_dhcp) {
        // IP Input
        p_canvas->drawString("IP Address:", 20, 140);
        drawCardBase(20, 160, 140, 45, COLOR_CARD_BG);
        p_canvas->setTextColor(COLOR_TEXT_MAIN);
        p_canvas->drawString(state.net.lan_static_ip, 30, 175);
        
        // GW Input
        p_canvas->setTextColor(COLOR_TEXT_SEC);
        p_canvas->drawString("Gateway:", 170, 140);
        drawCardBase(170, 160, 140, 45, COLOR_CARD_BG);
        p_canvas->setTextColor(COLOR_TEXT_MAIN);
        p_canvas->drawString(state.net.lan_gateway, 180, 175);
        
        // Subnet Input
        p_canvas->setTextColor(COLOR_TEXT_SEC);
        p_canvas->drawString("Subnet:", 320, 140);
        drawCardBase(320, 160, 140, 45, COLOR_CARD_BG);
        p_canvas->setTextColor(COLOR_TEXT_MAIN);
        p_canvas->drawString(state.net.lan_subnet, 330, 175);
    } else {
        p_canvas->setTextColor(COLOR_TEXT_SEC);
        p_canvas->drawString("Current IP: ", 20, 160);
        p_canvas->setTextColor(COLOR_ACCENT_MAIN);
        p_canvas->drawString(state.net.lan_ip, 120, 160);
    }
    
    // Save/Back Buttons
    drawCardBase(20, 260, 120, 45, COLOR_STAT_OFF);
    p_canvas->setTextDatum(MC_DATUM);
    p_canvas->setTextColor(COLOR_TEXT_MAIN);
    p_canvas->drawString("BACK", 80, 282);
    
    drawCardBase(340, 260, 120, 45, COLOR_STAT_ON);
    p_canvas->drawString("SAVE", 400, 282);
    p_canvas->setTextDatum(TL_DATUM);
}

void render_wifi_config(BuildingState& state) {
    p_canvas->fillScreen(COLOR_BG_MAIN);
    p_canvas->fillRect(0, 0, 480, 320, COLOR_BG_MAIN);

    // Header
    p_canvas->setTextColor(COLOR_TEXT_MAIN);
    p_canvas->setTextFont(4);
    p_canvas->drawString("WIFI SETUP", 20, 10);
    p_canvas->drawFastHLine(0, 45, 480, COLOR_ACCENT_MAIN);

    // SSID Field
    p_canvas->setTextColor(COLOR_TEXT_SEC);
    p_canvas->setTextFont(2);
    p_canvas->drawString("WiFi SSID:", 25, 65);
    drawCardBase(20, 85, 440, 50, COLOR_CARD_BG);
    p_canvas->setTextColor(COLOR_TEXT_MAIN);
    p_canvas->drawString(wifi_ssid, 35, 100);

    // Password Field
    p_canvas->setTextColor(COLOR_TEXT_SEC);
    p_canvas->drawString("Password:", 25, 150);
    drawCardBase(20, 170, 360, 50, COLOR_CARD_BG);
    p_canvas->setTextColor(COLOR_TEXT_MAIN);
    
    if (show_password) {
        p_canvas->drawString(wifi_pass, 35, 185);
    } else {
        char masked[32] = "";
        size_t len = strlen(wifi_pass);
        for(size_t i=0; i<len && i<31; i++) masked[i] = '*';
        p_canvas->drawString(masked, 35, 185);
    }

    // Show/Hide Toggle
    drawCardBase(390, 170, 70, 50, show_password ? COLOR_STAT_ON : COLOR_CARD_BG);
    p_canvas->setTextDatum(MC_DATUM);
    p_canvas->drawString(show_password ? "HIDE" : "SHOW", 390 + 35, 195);
    p_canvas->setTextDatum(TL_DATUM);

    // Action Buttons
    // Back button
    drawCardBase(15, 260, 80, 45, COLOR_STAT_OFF);
    p_canvas->setTextDatum(MC_DATUM);
    p_canvas->drawString("BACK", 15 + 40, 283);

    // Reconnect button
    drawCardBase(105, 260, 110, 45, COLOR_CARD_BG);
    p_canvas->drawString("RECONNECT", 105 + 55, 283);

    // Scan button
    drawCardBase(225, 260, 100, 45, COLOR_ACCENT_MAIN);
    p_canvas->setTextColor(COLOR_BG_MAIN); // Contrast
    p_canvas->drawString("SCAN", 225 + 50, 283);
    p_canvas->setTextColor(COLOR_TEXT_MAIN);

    // Connect button
    drawCardBase(335, 260, 130, 45, COLOR_STAT_ON);
    p_canvas->drawString("CONNECT", 335 + 65, 283);
    
    // Status Info
    p_canvas->setTextFont(2);
    p_canvas->setTextColor(COLOR_ACCENT_MAIN);
    p_canvas->setTextDatum(TL_DATUM);
    p_canvas->drawString(state.net.wifi_status_detail, 25, 235);
    
    p_canvas->setTextDatum(TL_DATUM);
}

void render_lan_status(BuildingState& state) {
    p_canvas->fillScreen(COLOR_BG_MAIN);
    p_canvas->fillRect(0, 0, 480, 320, COLOR_BG_MAIN);

    // Header
    p_canvas->setTextColor(COLOR_TEXT_MAIN);
    p_canvas->setTextFont(4);
    p_canvas->drawString("LAN STATUS", 20, 10);
    p_canvas->drawFastHLine(0, 45, 480, COLOR_ACCENT_MAIN);

    // Status Card
    drawCardBase(20, 70, 440, 150, COLOR_CARD_BG);
    
    p_canvas->setTextColor(COLOR_TEXT_SEC);
    p_canvas->setTextFont(2);
    p_canvas->drawString("Hardware: W5500 (SPI2)", 40, 85);
    
    p_canvas->drawString("Link Status:", 40, 115);
    if (state.net.lan_connected) {
        p_canvas->setTextColor(COLOR_STAT_ON);
        p_canvas->drawString("CONNECTED", 150, 115);
    } else {
        p_canvas->setTextColor(COLOR_STAT_ERR);
        p_canvas->drawString("DISCONNECTED / NO LINK", 150, 115);
    }

    p_canvas->setTextColor(COLOR_TEXT_SEC);
    p_canvas->drawString("IP Address:", 40, 145);
    p_canvas->setTextColor(COLOR_ACCENT_MAIN);
    p_canvas->setTextFont(4);
    p_canvas->drawString(state.net.lan_ip, 40, 170);

    // Footer Info
    p_canvas->setTextFont(2);
    p_canvas->setTextColor(COLOR_TEXT_SEC);
    p_canvas->drawString("MAC: Derived from ESP32", 40, 210);

    // Back button
    drawCardBase(20, 250, 120, 50, COLOR_STAT_OFF);
    p_canvas->setTextDatum(MC_DATUM);
    p_canvas->setTextColor(COLOR_TEXT_MAIN);
    p_canvas->drawString("BACK", 80, 275);
    p_canvas->setTextDatum(TL_DATUM);
}

void render_wifi_scan(BuildingState& state) {
    p_canvas->fillScreen(COLOR_BG_MAIN);
    p_canvas->fillRect(0, 0, 480, 320, COLOR_BG_MAIN);

    // Header
    p_canvas->setTextColor(COLOR_TEXT_MAIN);
    p_canvas->setTextFont(4);
    p_canvas->drawString("AVAILABLE NETWORKS", 20, 10);
    p_canvas->drawFastHLine(0, 45, 480, COLOR_ACCENT_MAIN);

    // Dummy Scan Results
    const char* dummy_ssids[] = {"Home_WiFi_5G", "Smart_Building_Guest", "TP-Link_9921", "Coffee_Shop_Free"};
    int signals[] = {90, 75, 40, 20};

    for(int i=0; i<4; i++) {
        int y_pos = 60 + (i * 55);
        drawCardBase(20, y_pos, 440, 50, COLOR_CARD_BG);
        p_canvas->setTextColor(COLOR_TEXT_MAIN);
        p_canvas->setTextFont(2);
        p_canvas->drawString(dummy_ssids[i], 40, y_pos + 15);
        
        // Signal strength bar (dummy)
        p_canvas->setTextColor(COLOR_TEXT_SEC);
        char rssi_buf[16];
        snprintf(rssi_buf, sizeof(rssi_buf), "Signal: %d%%", signals[i]);
        p_canvas->drawString(rssi_buf, 350, y_pos + 15);
    }

    // Refresh / Back Buttons
    drawCardBase(20, 280, 120, 35, COLOR_STAT_OFF);
    p_canvas->setTextDatum(MC_DATUM);
    p_canvas->setTextColor(COLOR_TEXT_MAIN);
    p_canvas->drawString("BACK", 80, 297);

    drawCardBase(340, 280, 120, 35, COLOR_CARD_BG);
    p_canvas->drawString("REFRESH", 400, 297);
    p_canvas->setTextDatum(TL_DATUM);
}

void render_keyboard_screen(BuildingState& state) {
    keyboard_draw();
}

void screens_render(BuildingState& state, int fps) {
    switch (current_screen) {
        case SCREEN_DASHBOARD:
            render_dashboard(state, fps);
            break;
        case SCREEN_SETTINGS:
            render_settings(state);
            break;
        case SCREEN_WIFI_CONFIG:
            render_wifi_config(state);
            break;
        case SCREEN_KEYBOARD:
            render_keyboard_screen(state);
            break;
        case SCREEN_LAN_STATUS:
            render_lan_status(state);
            break;
        case SCREEN_LAN_CONFIG:
            render_lan_config(state);
            break;
        case SCREEN_WIFI_SCAN:
            render_wifi_scan(state);
            break;
        default:
            break;
    }
}

void screens_set(ScreenState s) {
    if (current_screen == s) return;
    
    current_screen = s;
    
    // Brute force clear BOTH buffers and hardware
    tft.fillScreen(COLOR_BG_MAIN);
    canvas0.fillScreen(COLOR_BG_MAIN);
    canvas1.fillScreen(COLOR_BG_MAIN);
    canvas0.fillRect(0, 0, 480, 320, COLOR_BG_MAIN);
    canvas1.fillRect(0, 0, 480, 320, COLOR_BG_MAIN);
    
    Serial.printf("[UI] Screen Change: %d\n", (int)s);
}

// Box Collision Helper
bool isHit(int tx, int ty, int x, int y, int w, int h) {
    return (tx >= x && tx <= x + w && ty >= y && ty <= y + h);
}

void handle_dashboard_touch(BuildingState& state, int tx, int ty) {
    // Hamburger Menu Hit Detection (Top-Left)
    if (isHit(tx, ty, 0, 0, 80, 50)) {
        screens_set(SCREEN_SETTINGS);
        Serial.println("[UI] Navigation: Opening Settings");
        return; 
    }

    data_lock(state);
    
    // 1. UP Button (25, 140, 90, 60)
    if (isHit(tx, ty, 20, 135, 100, 70)) {
        state.sensor.temp_target += 0.5f;
        if (state.sensor.temp_target > 30.0f) state.sensor.temp_target = 30.0f;
        Serial.println("[UI] Action: TEMP UP");
    }
    // 2. DOWN Button (125, 140, 90, 60)
    else if (isHit(tx, ty, 120, 135, 100, 70)) {
        state.sensor.temp_target -= 0.5f;
        if (state.sensor.temp_target < 16.0f) state.sensor.temp_target = 16.0f;
        Serial.println("[UI] Action: TEMP DOWN");
    }
    // 3. AC Power Toggle (25, 215, 190, 55)
    else if (isHit(tx, ty, 20, 210, 200, 65)) {
        state.sensor.ac_on = !state.sensor.ac_on;
        Serial.println("[UI] Action: AC POWER TOGGLE");
    }
    // 4. Projector Toggle (240, 210, 110, 75)
    else if (isHit(tx, ty, 235, 205, 120, 70)) {
        state.sensor.projector_on = !state.sensor.projector_on;
        Serial.println("[UI] Action: PROJECTOR TOGGLE");
    }
    // 5. LED Toggle (360, 210, 110, 75)
    else if (isHit(tx, ty, 355, 205, 120, 70)) {
        state.sensor.light_on = !state.sensor.light_on; // Use light_on state for LED
        Serial.println("[UI] Action: LED TOGGLE");
    }
    // 6. Dashboard Page Toggle (380, 285, 90, 30)
    else if (isHit(tx, ty, 375, 280, 100, 40)) {
        state.dashboard_page = (state.dashboard_page == 0) ? 1 : 0;
        Serial.println("[UI] Action: SWITCH PAGE");
    }

    data_unlock(state);
}

void handle_settings_touch(BuildingState& state, int tx, int ty) {
    // Back Button
    if (isHit(tx, ty, 20, 250, 120, 50)) {
        screens_set(SCREEN_DASHBOARD);
        Serial.println("[UI] Navigation: Back to Dashboard");
    }
    // WiFi / LAN Priority Switch
    else if (isHit(tx, ty, 20, 80, 440, 55)) {
        data_lock(state);
        state.net.net_priority = (state.net.net_priority == 0) ? 1 : 0;
        data_unlock(state);
        Serial.printf("[UI] Net Priority: %s\n", state.net.net_priority == 0 ? "WIFI" : "LAN");
    }
    // WiFi Setup Button
    else if (isHit(tx, ty, 20, 150, 210, 60)) {
        screens_set(SCREEN_WIFI_CONFIG);
    }
    // LAN Setup
    else if (isHit(tx, ty, 250, 150, 210, 60)) {
        screens_set(SCREEN_LAN_CONFIG);
    }
}

void handle_lan_config_touch(BuildingState& state, int tx, int ty) {
    // Back to Settings
    if (isHit(tx, ty, 20, 260, 120, 45)) {
        screens_set(SCREEN_SETTINGS);
    }
    // Toggle DHCP
    else if (isHit(tx, ty, 20, 80, 440, 50)) {
        state.net.lan_use_dhcp = !state.net.lan_use_dhcp;
    }
    // Static IP Fields (Only if DHCP is off)
    else if (!state.net.lan_use_dhcp) {
        if (isHit(tx, ty, 20, 160, 140, 45)) {
            editing_target = 3;
            keyboard_set_text(state.net.lan_static_ip);
            screens_set(SCREEN_KEYBOARD);
        }
        else if (isHit(tx, ty, 170, 160, 140, 45)) {
            editing_target = 4;
            keyboard_set_text(state.net.lan_gateway);
            screens_set(SCREEN_KEYBOARD);
        }
        else if (isHit(tx, ty, 320, 160, 140, 45)) {
            editing_target = 5;
            keyboard_set_text(state.net.lan_subnet);
            screens_set(SCREEN_KEYBOARD);
        }
    }
    
    // Save Action
    if (isHit(tx, ty, 340, 260, 120, 45)) {
        lan_manager_save_config();
        screens_set(SCREEN_SETTINGS);
    }
}

void handle_wifi_touch(BuildingState& state, int tx, int ty) {
    // Back to Settings
    if (isHit(tx, ty, 20, 260, 100, 45)) {
        screens_set(SCREEN_SETTINGS);
    }
    // Select SSID
    else if (isHit(tx, ty, 20, 85, 440, 50)) {
        editing_target = 1;
        keyboard_set_text(""); // Auto-clear
        screens_set(SCREEN_KEYBOARD);
    }
    // Select Password
    else if (isHit(tx, ty, 20, 170, 360, 50)) {
        editing_target = 2;
        keyboard_set_text(""); // Auto-clear
        screens_set(SCREEN_KEYBOARD);
    }
    // Toggle Password Visibility
    else if (isHit(tx, ty, 390, 170, 70, 50)) {
        show_password = !show_password;
    }
    // Reconnect Action
    else if (isHit(tx, ty, 105, 260, 110, 45)) {
        data_lock(state);
        strcpy(state.net.wifi_status_detail, "RECONNECTING...");
        state.ui_needs_update = true;
        data_unlock(state);
        wifi_manager_reconnect();
    }
    // Scan Action
    else if (isHit(tx, ty, 225, 260, 100, 45)) {
        screens_set(SCREEN_WIFI_SCAN);
    }
    // Connect Action
    else if (isHit(tx, ty, 335, 260, 130, 45)) {
        wifi_manager_connect(wifi_ssid, wifi_pass);
        screens_set(SCREEN_DASHBOARD);
    }
}

void handle_wifi_scan_touch(BuildingState& state, int tx, int ty) {
    // Back to WiFi Setup
    if (isHit(tx, ty, 20, 280, 120, 35)) {
        screens_set(SCREEN_WIFI_CONFIG);
    }
    // Refresh (No-op for now)
    else if (isHit(tx, ty, 340, 280, 120, 35)) {
        Serial.println("[UI] Action: WiFi Rescan Triggered");
    }
}

void handle_keyboard_touch(BuildingState& state, int tx, int ty) {
    int kY_row4 = 244; // 100 + (42 + 6) * 3

    // OK / ENTER button
    if (isHit(tx, ty, 320, kY_row4, 150, 42)) {
        if (editing_target == 1) strcpy(wifi_ssid, keyboard_get_text());
        else if (editing_target == 2) strcpy(wifi_pass, keyboard_get_text());
        else if (editing_target == 3) strncpy(state.net.lan_static_ip, keyboard_get_text(), 16);
        else if (editing_target == 4) strncpy(state.net.lan_gateway, keyboard_get_text(), 16);
        else if (editing_target == 5) strncpy(state.net.lan_subnet, keyboard_get_text(), 16);
        
        if (editing_target >= 3) screens_set(SCREEN_LAN_CONFIG);
        else screens_set(SCREEN_WIFI_CONFIG);
        
        editing_target = 0;
        return;
    }
    
    // CANCEL button
    if (isHit(tx, ty, 10, kY_row4, 90, 42)) {
        if (editing_target >= 3) screens_set(SCREEN_LAN_CONFIG);
        else screens_set(SCREEN_WIFI_CONFIG);
        editing_target = 0;
        return;
    }
    
    keyboard_handle_touch(tx, ty);
}

void handle_lan_status_touch(BuildingState& state, int tx, int ty) {
    if (isHit(tx, ty, 20, 250, 120, 50)) {
        screens_set(SCREEN_SETTINGS);
    }
}

void screens_handle_touch(BuildingState& state, int tx, int ty) {
    switch (current_screen) {
        case SCREEN_DASHBOARD:
            handle_dashboard_touch(state, tx, ty);
            break;
        case SCREEN_SETTINGS:
            handle_settings_touch(state, tx, ty);
            break;
        case SCREEN_WIFI_CONFIG:
            handle_wifi_touch(state, tx, ty);
            break;
        case SCREEN_KEYBOARD:
            handle_keyboard_touch(state, tx, ty);
            break;
        case SCREEN_LAN_STATUS:
            handle_lan_status_touch(state, tx, ty);
            break;
        case SCREEN_LAN_CONFIG:
            handle_lan_config_touch(state, tx, ty);
            break;
        case SCREEN_WIFI_SCAN:
            handle_wifi_scan_touch(state, tx, ty);
            break;
        default:
            break;
    }
}
