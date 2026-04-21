#ifndef DISPLAY_H
#define DISPLAY_H

#include <TFT_eSPI.h>

// -------------------------------------------------------------------------
// UI Design Language (FSD 5.1)
// -------------------------------------------------------------------------
#define COLOR_BG_MAIN       0x0002      // #000010 (Hampir Hitam pekat untuk kontras)
#define COLOR_ACCENT_MAIN   0x07FF      // #00FFFF (Cyan murni)
#define COLOR_ACCENT_SEC    0x03FF      // #007FFF (Azure vibrant)
#define COLOR_CARD_BG       0x0863      // #0A0E1A (Dark navy untuk card)
#define COLOR_TEXT_MAIN     0xFFFF      // Putih bersih
#define COLOR_TEXT_SEC      0xAD55      // Abu-abu kebiruan terang
#define COLOR_STAT_ON       0x07E0      // Hijau murni
#define COLOR_STAT_OFF      0x39E7      // Abu-abu gelap kontras
#define COLOR_STAT_ERR      0xF800      // Merah murni
#define COLOR_STAT_WARN     0xFD20      // Orange vibrant


extern TFT_eSPI tft;

void display_init();
void display_brightness(uint8_t val);

#endif
