#include "grid.h"

#include <stdlib.h>


Jewel **create_grid(uint8_t rows, uint8_t cols, uint8_t color_size, uint16_t(*color_func)(uint8_t))
{
  Jewel **new_grid = (Jewel **)malloc(rows * sizeof(Jewel*));

  for (int i = 0; i < rows; ++i)
  {
    new_grid[i] = (Jewel*)malloc(cols * sizeof(Jewel));
  }
  
  for (int i = 0; i < rows; ++i)
  {
    for (int j = 0; j < cols; ++j)
    {
      Jewel new_jewel;
      new_jewel.position.x = i;
      new_jewel.position.y = j;
      new_jewel.color = (*color_func)(color_size);
      new_jewel.type = jewel;
      new_grid[i][j] = new_jewel;
    }
  }

  return new_grid;
}

void freeGrid(Jewel** grid, uint8_t rows, uint8_t cols)
{
  for (int i = 0; i < rows; i++)
    free(grid[i]);

  free(grid);
}

Jewel** copyGrid(Jewel** grid, uint8_t rows, uint8_t cols) {
  Jewel **new_grid = (Jewel **)malloc(rows * sizeof(Jewel*));

  for (int i = 0; i < rows; ++i)
  {
    new_grid[i] = (Jewel*)malloc(cols * sizeof(Jewel));
  }
  
  for (int i = 0; i < rows; ++i)
  {
    for (int j = 0; j < cols; ++j)
    {
      Jewel new_jewel;
      new_jewel.position.x = i;
      new_jewel.position.y = j;
      new_jewel.color = grid[i][j].color;
      new_jewel.type = grid[i][j].type;
      new_grid[i][j] = new_jewel;
    }
  }

  return new_grid;
}