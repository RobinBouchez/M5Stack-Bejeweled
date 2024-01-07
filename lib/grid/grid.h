#ifndef GRID_H
#define GRID_H

#include "jewel.h"

Jewel** copyGrid(Jewel** grid, uint8_t rows, uint8_t cols);
Jewel **create_grid(uint8_t, uint8_t, uint8_t, uint16_t(*color_func)(uint8_t));
void freeGrid(Jewel**, uint8_t, uint8_t);

#endif //GRID_H