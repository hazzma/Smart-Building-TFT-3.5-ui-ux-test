#include "ui_screens.h"
#include "ui_widgets.h"

static ScreenState current_screen = SCREEN_DASHBOARD;

void screens_init() {
    widgets_init();
}

void render_dashboard(BuildingState& state, int fps) {
    canvas.fillScreen(COLOR_BG_MAIN);

    // ── Top Notification Bar (y:0-28) ────────────────────────────
    drawNotifBar(state.net.wifi_connected, false, state.net.firebase_ok, state.net.room_name, "12:00");

    // ── Row 1: Temperature Cards (y:32-117) ──────────────────────
    // 4 cards, each 116px wide with 2px gap, total = 4*116 + 3*4 = 476px, start x=2
    drawTempCard(2,   32, 116, 86, "Suhu 1", state.sensor.temp[0], state.sensor.sensor_error[0]);
    drawTempCard(120, 32, 116, 86, "Suhu 2", state.sensor.temp[1], state.sensor.sensor_error[1]);
    drawTempCard(238, 32, 116, 86, "Suhu 3", state.sensor.temp[2], state.sensor.sensor_error[2]);
    drawTempCard(356, 32, 122, 86, "Suhu 4", state.sensor.temp[3], state.sensor.sensor_error[3]);

    // ── Row 2: AC Control (y:122-175) ────────────────────────────
    drawToggleButton(2, 122, 234, 54, "Air Conditioner", state.sensor.ac_on);
    drawUpDownButton(240, 122, 114, 54, "Turunkan", false);
    drawUpDownButton(358, 122, 120, 54, "Naikkan", true);

    // ── Row 3: Projector + Presence (y:180-233) ──────────────────
    drawToggleButton(2, 180, 234, 54, "Projector", state.sensor.projector_on);
    drawPresenceBadge(240, 180, 238, 54, state.sensor.human_presence);

    // ── Row 4: Light + Lux (y:238-291) ──────────────────────────
    drawToggleButton(2, 238, 234, 54, "Lampu", state.sensor.light_on);
    drawLuxCard(240, 238, 238, 54, state.sensor.lux_avg);

    // ── FPS counter (y:295-319, fills bottom gap) ─────────────────
    canvas.fillRect(0, 295, 480, 25, COLOR_CARD_BG);
    canvas.drawFastHLine(0, 295, 480, canvas.color565(30, 40, 65));
    canvas.setTextColor(canvas.color565(80, 100, 130));
    canvas.setFont(&fonts::Font0);
    canvas.setTextDatum(middle_left);
    char fps_buf[64];
    snprintf(fps_buf, sizeof(fps_buf), " FPS: %d  |  Smart Building v1.0", fps);
    canvas.drawString(fps_buf, 4, 307);
}

void screens_render(BuildingState& state, int fps) {
    switch (current_screen) {
        case SCREEN_DASHBOARD:
            render_dashboard(state, fps);
            break;
        default:
            break;
    }
}

void screens_set(ScreenState s) {
    current_screen = s;
}
