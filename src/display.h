#ifndef DISPLAY_H
#define DISPLAY_H

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// -------------------------------------------------------------------------
// UI Design Language (FSD 5.1)
// -------------------------------------------------------------------------
#define COLOR_BG_MAIN       0x0863      // #0A0E1A (Dark blue)
#define COLOR_ACCENT_MAIN   0x06BF      // #00D4FF (Cyan elektrik)
#define COLOR_ACCENT_SEC    0x041F      // #0A84FF (Bright blue)
#define COLOR_CARD_BG       0x10D5      // #141A2E (Dark blue medium)
#define COLOR_TEXT_MAIN     0xE7BD      // #E8F4FD (White bluish)
#define COLOR_TEXT_SEC      0x6C55      // #6B8CAE (Grey blue)
#define COLOR_STAT_ON       0x0652      // #00C896 (Teal green)
#define COLOR_STAT_OFF      0x31D3      // #2C3A4A (Dark grey)
#define COLOR_STAT_ERR      0xFA69      // #FF4D4D (Red)
#define COLOR_STAT_WARN     0xFDC0      // #FFB800 (Amber yellow)

// -------------------------------------------------------------------------
// LovyanGFX Configuration (FSD 4.2)
// -------------------------------------------------------------------------
class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ILI9488     _panel_instance;
    lgfx::Bus_Parallel8     _bus_instance;

public:
    LGFX(void) {
        {
            auto cfg = _bus_instance.config();
            cfg.port = 0;
            cfg.freq_write = 30000000;   // High Speed Para8
            cfg.pin_wr  = 15;
            cfg.pin_rd  = 14;
            cfg.pin_rs  = 16;
            cfg.pin_d0  = 1;
            cfg.pin_d1  = 2;
            cfg.pin_d2  = 3;
            cfg.pin_d3  = 4;
            cfg.pin_d4  = 5;
            cfg.pin_d5  = 6;
            cfg.pin_d6  = 7;
            cfg.pin_d7  = 8;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs           = 17;
            cfg.pin_rst          = 48;
            cfg.panel_width      = 320;
            cfg.panel_height     = 480;
            cfg.offset_x         = 0;
            cfg.offset_y         = 0;
            cfg.offset_rotation  = 0;
            cfg.bus_shared       = false;
            _panel_instance.config(cfg);
        }

        setPanel(&_panel_instance);
    }
};

extern LGFX lcd;

void display_init();
void display_brightness(uint8_t val);

#endif
