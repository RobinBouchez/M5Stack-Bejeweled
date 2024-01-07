#define LEVEL_1 1
#define LEVEL_2 2
#define LEVEL_3 3
#define LEVEL_4 4


#define GAME_MODE_X 10
#define GAME_MODE_Y 40


enum gameMode
{
  EASY = 1,
  HARD = 0
};

uint8_t level = START_LEVEL_INDEX;
uint8_t level_multiplier = 1;
uint8_t score_multiplier = 10;

gameMode game_mode;