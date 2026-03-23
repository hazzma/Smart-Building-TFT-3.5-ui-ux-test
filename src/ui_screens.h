#ifndef UI_SCREENS_H
#define UI_SCREENS_H

#include "data.h"

enum ScreenState {
    SCREEN_DASHBOARD = 0,
    SCREEN_SETTINGS = 1,
    SCREEN_SLAVE_MANAGER = 2,
    SCREEN_TOUCH_CALIB = 3
};

void screens_init();
void screens_render(BuildingState& state, int fps);
void screens_set(ScreenState s);

#endif
