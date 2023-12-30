#ifndef JEWEL_H
#define JEWEL_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct jewel
{
  u_int8_t x;
  u_int8_t y;
  u_int16_t color;
};
typedef struct jewel Jewel;


#endif JEWEL_H