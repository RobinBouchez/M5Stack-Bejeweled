#ifndef JEWEL_H
#define JEWEL_H

#include <stdint.h>

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


// #include <stdint.h>

// typedef struct 
// {
//   unsigned int x : 4;
//   unsigned int y : 4;
// } Jewelpos;

// typedef enum {jewel, diamond} jewelType;
// struct jewel
// {
//   Jewelpos position;
//   uint16_t color;
//   jewelType type;
// };
// typedef struct jewel Jewel;

// #define MAX_JEWEL_AMOUNT 8
// #define MAX_JEWEL_COLOR_AMOUNT MAX_JEWEL_AMOUNT - 1
// #define MIN_JEWEL_AMOUNT 4

// uint8_t jewel_color_size = 7;
// uint8_t jewel_amount = MIN_JEWEL_AMOUNT;

// uint8_t jewel_width = 12;
// uint8_t jewel_height = 12;

// uint8_t getJewelWidth() { 
//     return jewel_width; 
// }

// uint8_t getJewelHeight() {
//      return jewel_height; 
// }

#endif //JEWEL_H