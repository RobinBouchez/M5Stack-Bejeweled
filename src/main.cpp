#include <M5StickCPlus.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "EEPROM.h"
#include <vector>

#include "constants.h"

#include "menu.h"
#include "selector.h"
#include "jewel.h"
#include "gamemode.h"
#include "grid.h"


uint8_t jewel_width;
uint8_t jewel_height;

bool start_game = false;

uint8_t jewel_color_size = MAX_COLOR_SIZE;
uint8_t jewel_amount = MIN_JEWEL_AMOUNT;
uint16_t jewel_color[MAX_JEWEL_COLOR_AMOUNT]{RED, BLUE, GREEN, ORANGE, YELLOW, WHITE, PINK};

RTC_TimeTypeDef TimeStruct;

uint8_t moves_left = 60;
uint16_t score = 0;

int selectedItem = 0;
Jewel **pGrid;
Selector selector;

bool end_game = false;
bool can_move_selector = true;
bool game_is_ended = false;

void rotateSelector(void);
bool validSwap(Selector);
void setupLevel(uint8_t);

//--------------------------------------------
// UPDATE FUNCTION DECLARATIONS
//--------------------------------------------
void updateGame(void);
void updateMenu(int, int, char *[]);
void updateSelector(float, float, float);
void updateGamemode(int, int, char *[]);
int updateJewels(Jewel **, Jewel, const unsigned int);

//--------------------------------------------
// DRAW FUNCTION DECLARATIONS
//--------------------------------------------
void drawList(int, int, char *[], int);
void drawGrid(Jewel **);
void drawGame(void);
void drawScore(uint16_t);
void drawMoves(uint8_t);
void drawCell(uint8_t, uint8_t, JewelType, uint16_t);
void drawJewel(uint8_t, uint8_t, uint8_t, uint8_t, uint16_t);
void drawDiamond(uint8_t, uint8_t, uint8_t, uint8_t, uint16_t);
void drawSelector(Selector, uint8_t, uint8_t, uint16_t);
void drawNewGrid(Jewel **, Jewel **);

void setup()
{
  // initialize the M5StickC object
  M5.begin();
  M5.IMU.Init();
  M5.Axp.begin();
  M5.Rtc.begin();
  Serial.begin(115200);
  Serial.flush();
  EEPROM.begin(512);
  M5.Lcd.setTextSize(1);
  M5.Lcd.fillScreen(BLACK);
  setupLevel(level);
  TimeStruct.Seconds = 0;
  M5.Rtc.SetTime(&TimeStruct);
  drawList(GAME_MODE_X, GAME_MODE_Y, game_modes, GAME_MODE_ARR_SIZE);
}

// the loop routine runs over and over again forever
void loop()
{
  M5.update();
  if (game_is_ended) return;

  if (!start_game)
  {
    updateGamemode(GAME_MODE_X, GAME_MODE_Y, game_modes);
    return;
  }

  if (isInMenu)
  {
    updateMenu(MENU_X, MENU_Y, menu);
    return;
  }
  if (end_game)
  {
    M5.Lcd.fillScreen(BLACK);
    freeGrid(pGrid, jewel_amount, jewel_amount);
    game_is_ended = true;
    return;
  }

  updateGame();

  delay(300);
}

void rotateSelector(void)
{
  if (selector.rotation == horizontal)
  {
    if (selector.pos_1.y < jewel_amount - 1)
    {
      selector.pos_2.x = selector.pos_1.x;
      selector.pos_2.y = selector.pos_1.y + 1;
      selector_width = jewel_width;
      selector_height = jewel_height * 2;
      selector.rotation = vertical;
    }
  }
  else
  {
    if (selector.pos_1.x < jewel_amount - 1)
    {
      selector.pos_2.x = selector.pos_1.x + 1;
      selector.pos_2.y = selector.pos_1.y;
      selector_width = jewel_width * 2;
      selector_height = jewel_height;
      selector.rotation = horizontal;
    }
  }
}

uint16_t getRandColor(uint8_t color_size)
{
  return jewel_color[rand() % jewel_color_size];
}

void setupLevel(uint8_t level_index)
{
  if (level_index != START_LEVEL_INDEX && jewel_amount != MAX_JEWEL_AMOUNT)
  {
    jewel_amount = level_index + MIN_JEWEL_AMOUNT;
  }
  jewel_color_size = jewel_amount - 1;

  jewel_width = (M5.Lcd.width() - PADDING_X * 2) / jewel_amount;
  jewel_height = jewel_width;

  selector.rotation = horizontal;
  selector_width = jewel_width * 2;
  selector_height = jewel_height;

  level_multiplier = (level_index + 1);

  pGrid = create_grid(jewel_amount, jewel_amount, jewel_color_size, &getRandColor);

  selector.pos_1.x = 1;
  selector.pos_2.x = 2;
  selector.pos_1.y = 1;
  selector.pos_2.y = 1;

  moves_left = ((jewel_amount * 2) / level_multiplier) * score_multiplier;
  drawGame();
}

void restartGame(void)
{
  score = 0;
  freeGrid(pGrid, jewel_amount, jewel_amount);
  pGrid = create_grid(jewel_amount, jewel_amount, jewel_color_size, &getRandColor);

  setupLevel(level);
}

void saveGame()
{
  int address = 0;
  uint8_t lower_score = score % 256;
  uint8_t upper_score = score >> 8;

  // save score
  EEPROM.writeByte(address, lower_score);
  address++;
  EEPROM.writeByte(address, upper_score);

  // save moves_left
  address++;
  EEPROM.writeByte(address, moves_left);

  // save gamemode
  address++;
  EEPROM.writeByte(address, game_mode);

  // save grid
  address++;
  for (int i = 0; i < jewel_amount; i++)
  {
    for (int j = 0; j < jewel_amount; j++)
    {
      uint8_t lower_color = pGrid[i][j].color % 256;
      uint8_t upper_color = pGrid[i][j].color >> 8;

      EEPROM.writeByte(address, lower_color);
      address++;
      EEPROM.writeByte(address, upper_color);
      address++;
    }
  }

  EEPROM.commit();
}

void loadGame()
{
  int address = 0;
  uint8_t lower_score = EEPROM.readByte(address);
  address++;
  uint8_t upper_score = EEPROM.readByte(address);
  score = (upper_score << 8) | lower_score;

  address++;
  moves_left = EEPROM.readByte(address);

  address++;
  uint8_t mode = game_mode;
  mode = EEPROM.readByte(address);
  if (mode == 1)
  {
    game_mode = EASY;
  }
  else
    game_mode = HARD;

  // Load grid
  address++;
  for (int i = 0; i < jewel_amount; i++)
  {
    for (int j = 0; j < jewel_amount; j++)
    {
      uint8_t lower_color = EEPROM.readByte(address);
      address++;
      uint8_t upper_color = EEPROM.readByte(address);
      pGrid[i][j].color = (upper_color << 8) | lower_color;
      address++;
    }
  }
}

//----------------------------------------------------------------
// SELECTING LOGIC & UPDATE DEFENITIONS
//----------------------------------------------------------------
int select(int x, int y, int idx, char *arr[], const int size)
{
  if (M5.BtnB.wasPressed())
  {
    TimeStruct.Seconds = 0;
    M5.Rtc.SetTime(&TimeStruct);
    M5.Lcd.fillScreen(BLACK);
    for (int i = 0; i < size; i++)
    {
      M5.Lcd.setTextColor(WHITE);
      if (i == idx)
      {
        M5.Lcd.setTextColor(BLUE);
      }
      M5.Lcd.setCursor(x, y + (y * i));
      M5.Lcd.printf(arr[i]);
    }
    idx = (idx + 1) % size;
    M5.Beep.tone(700, BEEP_DURATION);
  }
  return idx;
}

void updateGamemode(int x, int y, char *modes[])
{
  selectedItem = select(x, y, selectedItem, modes, GAME_MODE_ARR_SIZE);
  if (M5.BtnA.wasPressed())
  {
    switch (selectedItem)
    {
    case EASY:
      start_game = true;
      drawGame();
      selectedItem = 0;
      game_mode = EASY;
      break;
    case HARD:
      start_game = true;
      selectedItem = 0;
      game_mode = HARD;
      level = MAX_JEWEL_AMOUNT - 1;
      setupLevel(level);
      break;
    default:
      break;
    }
  }
}

void updateMenu(int x, int y, char *menu[])
{
  if (TimeStruct.Seconds >= MENU_TIME_OUT)
  {
    isInMenu = false;
    drawGame();
    return;
  }
  selectedItem = select(x, y, selectedItem, menu, MENUSIZE);
  if (M5.BtnA.wasPressed())
  {
    M5.Beep.tone(1000, BEEP_DURATION);
    switch (selectedItem)
    {
    case BACK:
      break;
    case LOAD:
      loadGame();
      break;
    case SAVE:
      saveGame();
      break;
    case RESET:
      restartGame();
      break;
    default:
      break;
    }
    isInMenu = false;
    selectedItem = 0;
    drawGame();
  }
  M5.Rtc.GetTime(&TimeStruct);
}
void updateGrid() {
  for (int i = 0; i < jewel_amount; i++)
  {
    for (int j = 0; j < jewel_amount; j++)
    {
      score += updateJewels(pGrid, pGrid[i][j], pGrid[i][j].color);
      if (score > (2 * level_multiplier) * 100)
      {
        level++;
        setupLevel(level);
        return;
      };
    }
  }
}

void updateGame() {
  uint16_t old_score = score;
  if (M5.BtnA.wasPressed() && M5.BtnB.wasPressed())
  {
    if (validSwap(selector))
    {
      swapJewel(pGrid, selector);
      moves_left--;
      drawGame();
    }
  }
  else if (M5.BtnA.wasPressed())
  {
    rotateSelector();
    drawSelector(selector, selector_width, selector_height, BLACK);
  }
  else if (M5.BtnB.wasPressed())
  {
    isInMenu = true;
    return;
  }

  M5.IMU.getAccelData(&acc_x, &acc_y, &acc_z);
  updateSelector(acc_x, acc_y, acc_z);
  updateGrid();

  if (old_score != score)
  {
    drawScore(score);
  }

  TimeStruct.Seconds = 0;
  M5.Rtc.SetTime(&TimeStruct);
  end_game = (moves_left == 0);
}

void updateSelector(float p, float r, float y)
{
  Jewelpos old;
  old.x = selector.pos_1.x;
  old.y = selector.pos_1.y;

  if (!can_move_selector)
    return;

  if (p > MIN_TILT_X)
  {
    if (selector.pos_1.x > 0)
    {
      selector.pos_1.x -= 1;
    }
  }
  else if (p < -MIN_TILT_X)
  {
    if (selector.pos_1.x < jewel_amount - 1)
    {
      selector.pos_1.x += 1;
    }
  }

  if (r > MAX_TILT_Y)
  {
    if (selector.pos_1.y < jewel_amount - 1)
    {
      selector.pos_1.y += 1;
    }
  }
  else if (r < MIN_TILT_Y)
  {
    if (selector.pos_1.y > 0)
    {
      selector.pos_1.y -= 1;
    }
  }

  if ((uint)selector.pos_1.x == (uint)old.x && (uint)selector.pos_1.y == (uint)old.y)
    return;

  if (selector.rotation == horizontal)
  {
    selector.pos_2.x = selector.pos_1.x + 1;
    selector.pos_2.y = selector.pos_1.y;
  }
  else
  {
    selector.pos_2.x = selector.pos_1.x;
    selector.pos_2.y = selector.pos_1.y + 1;
  }

  if (selector.rotation == horizontal)
  {
    if (selector.pos_1.x <= 0 || selector.pos_2.x <= 1)
    {
      selector.pos_1.x = 0;
    }
    else if (selector.pos_1.x >= jewel_amount - 2)
    {
      selector.pos_1.x = jewel_amount - 2;
    }

    if (selector.pos_1.y <= 0 || selector.pos_1.y == 255)
    {
      selector.pos_1.y = 0;
    }
    if (selector.pos_1.y >= jewel_amount - 1)
    {
      selector.pos_1.y = jewel_amount - 1;
    }
  }
  else // vertical
  {
    if (selector.pos_1.x <= 0 || selector.pos_1.x == 255)
    {
      selector.pos_1.x = 0;
    }
    if (selector.pos_1.x >= jewel_amount - 1)
    {
      selector.pos_1.x = jewel_amount - 1;
    }

    if (selector.pos_1.y <= 0 || selector.pos_2.y <= 1)
    {
      selector.pos_1.y = 0;
    }
    else if (selector.pos_1.y >= jewel_amount - 2)
    {
      selector.pos_1.y = jewel_amount - 2;
    }
    else if (selector.pos_1.y >= jewel_amount - 1)
    {
      selector.pos_1.y = jewel_amount - 1;
    }
  }

  if (selector.rotation == horizontal)
  {
    selector.pos_2.x = selector.pos_1.x + 1;
    selector.pos_2.y = selector.pos_1.y;
  }
  else
  {
    selector.pos_2.x = selector.pos_1.x;
    selector.pos_2.y = selector.pos_1.y + 1;
  }

  drawSelector(selector, selector_width, selector_height, BLACK);
}

//----------------------------------------------------------------
// GAME LOGIC
//----------------------------------------------------------------

bool checkRowJewel(Jewel **grid, Jewel jewel, const uint16_t color)
{
  uint8_t right = 1;
  uint8_t left = 1;
  Serial.print("checkrow");

  while (jewel.position.x + right < jewel_amount && grid[jewel.position.x + right][jewel.position.y].color == color)
  {
    right++;
  }
  while (jewel.position.x - left > 0 && grid[jewel.position.x - left][jewel.position.y].color == color)
  {
    left++;
  }
  return right + left - 1 >= JEWEL_THRESHOLD;
}

bool checkColJewel(Jewel **grid, Jewel jewel, const uint16_t color)
{
  uint8_t top = 1;
  uint8_t bottom = 1;

  while (jewel.position.y + top < jewel_amount && grid[jewel.position.x][jewel.position.y + top].color == color)
  {
    top++;
  }
  while (jewel.position.y - bottom > 0 && grid[jewel.position.x][jewel.position.y - bottom].color == color)
  {

    bottom++;
  }
  return top + bottom - 1 >= JEWEL_THRESHOLD;
}

bool validSwap(Selector selector)
{
  if (game_mode == EASY)
  {
    return true;
  }
  else
  {
    return checkRowJewel(pGrid, pGrid[selector.pos_1.x][selector.pos_1.y], pGrid[selector.pos_2.x][selector.pos_2.y].color) ||
           checkColJewel(pGrid, pGrid[selector.pos_2.x][selector.pos_2.y], pGrid[selector.pos_1.x][selector.pos_1.y].color) ||
           checkRowJewel(pGrid, pGrid[selector.pos_2.x][selector.pos_2.y], pGrid[selector.pos_1.x][selector.pos_1.y].color) ||
           checkColJewel(pGrid, pGrid[selector.pos_1.x][selector.pos_1.y], pGrid[selector.pos_2.x][selector.pos_2.y].color);
  }
}

void dropCol(Jewel **grid, int x, int y)
{
  for (int i = 0; i <= y; i++)
  {
    grid[x][y - i].color = (y - i > 0) ? grid[x][y - i - 1].color : jewel_color[rand() % jewel_color_size];
  }
}

bool foundDiamond = false;
int findConsecutiveJewels(int x, int y, int directionX, int directionY, int jewel_amount, Jewel **grid, const unsigned int color)
{
  int count = 1;

  while ((x + directionX * count >= 0) && (x + directionX * count < jewel_amount) &&
         (y + directionY * count >= 0) && (y + directionY * count < jewel_amount) &&
         (grid[x + directionX * count][y + directionY * count].color == color))
  {
    // if (grid[x + directionX * count][y + directionY * count].type == diamond)
    // {
    //   grid[x + directionX * count][y + directionY * count].type = jewel;
    //   foundDiamond = true;
    // }
    count++;
  }

  return count;
}

int updateJewels(Jewel **grid, Jewel jewel, const unsigned int color)
{
  int right = findConsecutiveJewels(jewel.position.x, jewel.position.y, RIGHT, NO_DIRECTION, jewel_amount, grid, color);
  int left = findConsecutiveJewels(jewel.position.x, jewel.position.y, LEFT, NO_DIRECTION, jewel_amount, grid, color);
  int top = findConsecutiveJewels(jewel.position.x, jewel.position.y, NO_DIRECTION, TOP, jewel_amount, grid, color);
  int bottom = findConsecutiveJewels(jewel.position.x, jewel.position.y, NO_DIRECTION, BOTTOM, jewel_amount, grid, color);

  // if (foundDiamond && (right + left - 1 >= JEWEL_THRESHOLD || top + bottom - 1 >= JEWEL_THRESHOLD))
  // {
  //   for (int i = 0; i < jewel_amount - 1; i++)
  //   {
  //     dropCol(grid, i, jewel.position.y);
  //   }
  //     dropCol(grid, jewel.position.x, jewel_amount - 1);
  // }

  if (right + left > JEWEL_THRESHOLD)
  {
    Jewel **oldGrid = copyGrid(grid, jewel_amount, jewel_amount);
    for (int i = 1; i < right + left; i++)
    {
      for (int j = 0; j <= jewel_width; j++)
      {
        M5.Lcd.fillRect(PADDING_X + ((jewel.position.x - left + i) * jewel_width), PADDING_Y + (jewel.position.y * jewel_height), j, jewel_height, BLACK);
        delay(10);
      }
    }

    for (int i = 1; i < right + left; i++)
    {
      dropCol(grid, jewel.position.x - left + i, jewel.position.y);
    }

    if (right + left - 1 > JEWEL_THRESHOLD)
      grid[jewel.position.x - left + 1][jewel.position.y].type = diamond;
    drawNewGrid(oldGrid, grid);
    drawGame();

    return score_multiplier * (right + left - 1);
  }

  if (top + bottom > JEWEL_THRESHOLD)
  {
    Jewel **oldGrid = copyGrid(grid, jewel_amount, jewel_amount);
    for (int i = 0; i < top + bottom - 1; i++)
    {
      for (int j = 0; j <= jewel_height; j++)
      {
        M5.Lcd.fillRect(PADDING_X + (jewel.position.x * jewel_width), PADDING_Y + ((jewel.position.y - (bottom - 1) + i) * jewel_height), jewel_width, j, BLACK);
        delay(10);
      }
    }

    for (int i = 0; i < bottom + top - 1; i++)
    {
      dropCol(grid, jewel.position.x, jewel.position.y - (bottom - 1) + i);
    }

    drawNewGrid(oldGrid, grid);
    drawGame();
    if (top + bottom - 1 > JEWEL_THRESHOLD)
      grid[jewel.position.x - bottom + 1][jewel.position.y].type = diamond;
    return score_multiplier * (top + bottom - 1);
  }

  return 0;
}

//----------------------------------------------------------------
// Draw funtion definitions
//----------------------------------------------------------------

void drawGame(void)
{
  M5.Lcd.fillScreen(BLACK);
  drawScore(score);
  drawMoves(moves_left);
  drawGrid(pGrid);
  drawSelector(selector, selector_width, selector_height, BLACK);
}

void drawGrid(Jewel **grid)
{
  for (int i = 0; i < jewel_amount; i++)
  {
    for (int j = 0; j < jewel_amount; j++)
    {
      drawCell(i, j, grid[i][j].type, grid[i][j].color);
    }
  }
}

void drawNewGrid(Jewel **oldGrid, Jewel **newGrid)
{
  for (int i = 0; i < jewel_amount; i++)
  {
    for (int j = 0; j < jewel_amount; j++)
    {
      if (oldGrid[i][j].color != newGrid[i][j].color)
      {
        drawCell(i, j, newGrid[i][j].type, newGrid[i][j].color);
        delay(100);
      }
    }
  }
}

void drawJewel(const uint8_t x, const uint8_t y, const uint8_t width, const uint8_t height, const uint16_t color)
{
  M5.Lcd.fillRect(PADDING_X + (x * width), PADDING_Y + (y * height), width, height, color);
}

void drawCell(const uint8_t x, const uint8_t y, const JewelType type, const uint16_t color)
{
  if (type == jewel)
  {
    drawJewel(x, y, jewel_width, jewel_height, color);
  }
  else
  {
    drawDiamond(x, y, jewel_width, jewel_height, color);
  }
}

void drawScore(uint16_t score)
{
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(PADDING_X, CURSOR_OFFSET * 2);
  M5.Lcd.fillRect(PADDING_X, CURSOR_OFFSET * 2, M5.Lcd.width(), PADDING_X, BLACK);
  M5.Lcd.printf("Score: %i", score);
}

void drawMoves(uint8_t moves)
{
  M5.Lcd.setCursor(PADDING_X, CURSOR_OFFSET);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.fillRect(PADDING_X, CURSOR_OFFSET, M5.Lcd.width(), PADDING_X, BLACK);
  M5.Lcd.printf("Moves left: %i", moves);
}

void drawList(const int x, const int y, char *arr[], const int size)
{
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(x, y);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);

  for (int i = 0; i < size; i++)
  {
    M5.Lcd.setCursor(x, y * (i + 1));
    M5.Lcd.printf(arr[i]);
  }
}

void drawDiamond(const uint8_t x, const uint8_t y, const uint8_t width, const uint8_t height, const uint16_t color)
{
  M5.Lcd.fillRect(PADDING_X + (x * width), PADDING_Y + (y * height), width, height, BLACK);
  M5.Lcd.fillTriangle(PADDING_X + (x * width), PADDING_Y + (y * height),
                      PADDING_X + ((x + 1) * width) - width / 2, PADDING_Y + ((y + 1) * height) - 1,
                      PADDING_X + ((x + 1) * width) - 1, PADDING_Y + (y * height),
                      color);
}

void drawSelectorBounds(const uint8_t x, const uint8_t y)
{
  // Draw the current jewel
  drawCell(x, y, pGrid[x][y].type, pGrid[x][y].color);

  // Right
  if (x + 1 < jewel_amount)
    drawCell(x + 1, y, pGrid[x + 1][y].type, pGrid[x + 1][y].color);

  // Left
  if (x - 1 >= 0)
    drawCell(x - 1, y, pGrid[x - 1][y].type, pGrid[x - 1][y].color);
  // Top
  if (y + 1 < jewel_amount)
    drawCell(x, y + 1, pGrid[x][y + 1].type, pGrid[x][y + 1].color);

  // Bottom
  if (y - 1 >= 0)
    drawCell(x, y - 1, pGrid[x][y - 1].type, pGrid[x][y - 1].color);

  if (y + 1 < jewel_amount && x + 1 < jewel_amount)
    drawCell(x + 1, y + 1, pGrid[x + 1][y + 1].type, pGrid[x + 1][y + 1].color);

  if (y - 1 >= 0 && x + 1 < jewel_amount)
    drawCell(x + 1, y - 1, pGrid[x + 1][y - 1].type, pGrid[x + 1][y - 1].color);

  if (x - 1 >= 0 && y - 1 >= 0)
    drawCell(x - 1, y - 1, pGrid[x - 1][y - 1].type, pGrid[x - 1][y - 1].color);

  if (x - 1 >= 0 && y + 1 < jewel_amount)
    drawCell(x - 1, y + 1, pGrid[x - 1][y + 1].type, pGrid[x - 1][y + 1].color);
}

void drawSelector(Selector selector, uint8_t width, uint8_t height, uint16_t color)
{
  drawSelectorBounds(selector.pos_1.x, selector.pos_1.y);
  drawSelectorBounds(selector.pos_2.x, selector.pos_2.y);
  M5.Lcd.drawRect(PADDING_X + selector.pos_1.x * jewel_width, PADDING_Y + selector.pos_1.y * jewel_height, selector_width, selector_height, color);
}
