#ifndef JEWEL_H
#define JEWEL_H

#include <stdint.h>

#define MAX_JEWEL_AMOUNT 8
#define MAX_JEWEL_COLOR_AMOUNT MAX_JEWEL_AMOUNT - 1
#define MIN_JEWEL_AMOUNT 4

typedef struct 
{
  unsigned int x : 4;
  unsigned int y : 4;
}Jewelpos;

typedef enum 
{
  jewel,
  diamond
} JewelType;

typedef struct
{
  Jewelpos position;
  uint16_t color;
  JewelType type;
} Jewel;


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

void swapJewel(Jewel**, Selector);

#endif //JEWEL_H