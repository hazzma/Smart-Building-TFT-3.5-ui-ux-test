#ifndef UI_SCREENS_H
#define UI_SCREENS_H

#include "data.h"

enum ScreenState {
    SCREEN_DASHBOARD = 0,
    SCREEN_SETTINGS = 1,
    SCREEN_WIFI_CONFIG = 2,
    SCREEN_KEYBOARD = 3,
    SCREEN_LAN_STATUS = 4,
    SCREEN_LAN_CONFIG = 5,
    SCREEN_SLAVE_MANAGER = 6,
    SCREEN_TOUCH_CALIB = 7,
    SCREEN_WIFI_SCAN = 8
};

void screens_init();
void screens_render(BuildingState& state, int fps);
void screens_set(ScreenState s);
void screens_handle_touch(BuildingState& state, int tx, int ty);

#endif
