#ifndef SELECTOR_H
#define SELECTOR_H

#include <stdint.h>

#define MIN_TILT_X 0.15
#define MIN_TILT_Y 0.20
#define MAX_TILT_Y 0.65

float acc_x = 0.f;
float acc_y = 0.f;
float acc_z = 0.f;

uint8_t selector_width;
uint8_t selector_height;

#endif //SELECTOR_H