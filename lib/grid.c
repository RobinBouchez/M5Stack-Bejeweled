#include "stdlib.h"
#include "grid.h"

Grid*** create_grid(){
    Grid*** new_grid = (Grid***)malloc(jewel_amount * sizeof(Grid**));

    for (int i = 0; i < jewel_amount; i++)
    {
        new_grid[i] = (Grid**)malloc(jewel_amount * sizeof(Grid*));
    }
    
    for (int i = 0; i < jewel_amount; i++)
    {
        for (int j = 0; j < jewel_amount; j++)
        {
            Jewel* new_jewel = (Jewel*) malloc(sizeof(Jewel));
            new_grid[i][j] = new_jewel;
        }
        
    }
    return new_grid;
}

// void fill_grid(Grid*** grid, size_t type) {
//     for (int i = 0; i < jewel_amount; i++)
//     {
//         for (int j = 0; j < jewel_amount; j++)
//         {
//             grid[i][j] = type;
//         }
        
//     }
    
// }