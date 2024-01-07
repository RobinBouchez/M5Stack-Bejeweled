
#ifndef SELECTOR_H
#define SELECTOR_H

//#include <stdint.h>
#include "jewel.h"

#define MIN_TILT_X 0.15
#define MIN_TILT_Y 0.20
#define MAX_TILT_Y 0.65

typedef enum 
{
  vertical,
  horizontal
} Rotation;

typedef struct
{
  Jewelpos pos_1;
  Jewelpos pos_2;
  Rotation rotation;
} Selector;

float acc_x = 0.f;
float acc_y = 0.f;
float acc_z = 0.f;

void swapJewel(Jewel**, Selector);

#endif //SELECTOR_H