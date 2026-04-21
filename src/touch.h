#ifndef TOUCH_H
#define TOUCH_H

#include <Arduino.h>
#include <S3_Parallel_Touch.h>

// Touch Pin Mapping (S3 Parallel Shield)
#define TOUCH_XP  7    // Data D6
#define TOUCH_YP  15   // WR
#define TOUCH_XM  16   // RS / DC
#define TOUCH_YM  8    // Data D7

void touch_init();
bool touch_get_point(int &tx, int &ty);

#endif
