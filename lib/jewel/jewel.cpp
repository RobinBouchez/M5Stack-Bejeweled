#include "jewel.h"


void swapJewel(Jewel** grid, Selector selector)
{
  uint16_t temp_color = grid[selector.pos_1.x][selector.pos_1.y].color;
  JewelType temp_type = grid[selector.pos_1.x][selector.pos_1.y].type;
  grid[selector.pos_1.x][selector.pos_1.y].color = grid[selector.pos_2.x][selector.pos_2.y].color;
  grid[selector.pos_1.x][selector.pos_1.y].type = grid[selector.pos_2.x][selector.pos_2.y].type;
  grid[selector.pos_2.x][selector.pos_2.y].color = temp_color;
  grid[selector.pos_2.x][selector.pos_2.y].type = temp_type;
}