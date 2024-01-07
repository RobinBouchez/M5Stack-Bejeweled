#include "selector.h"

void swapJewel(Jewel** grid, Selector selector)
{
  uint16_t temp = grid[selector.pos_1.x][selector.pos_1.y].color;
  grid[selector.pos_1.x][selector.pos_1.y].color = grid[selector.pos_2.x][selector.pos_2.y].color;
  grid[selector.pos_2.x][selector.pos_2.y].color = temp;
}