#include <M5StickCPlus.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "EEPROM.h"
#include <vector>
// #include "../lib/grid/grid.h"
#include "menu.h"
#include "selector.h"
#include "jewel.h"
#include "../include/gamemode.h"

// #include "../lib/jewel/jewel.h"
//  #include "../lib/selector/selector.h"

#define VARIANTSIZE 2
char *variants[VARIANTSIZE]{"EASY", "HARD"};

Selector selector;

#define START_LEVEL_INDEX 0
uint8_t level = START_LEVEL_INDEX;


uint8_t padding_x = 10;
uint8_t padding_y = 80;
uint8_t cursor_offset = 20;
uint8_t jewel_width = 12;
uint8_t jewel_height = 12;

int selectedItem = 0;
uint8_t level_multiplier = 1;
uint8_t score_multiplier = 10;

bool start_game = false;
#define MAX_JEWEL_AMOUNT 8
#define MAX_JEWEL_COLOR_AMOUNT MAX_JEWEL_AMOUNT - 1
#define MIN_JEWEL_AMOUNT 4
uint8_t jewel_color_size = 7;
uint8_t jewel_amount = MIN_JEWEL_AMOUNT;
uint16_t jewel_color[MAX_JEWEL_COLOR_AMOUNT]{RED, BLUE, GREEN, ORANGE, YELLOW, WHITE, PINK};

uint8_t selector_width = jewel_width * 2;
uint8_t selector_height = jewel_height;

RTC_TimeTypeDef TimeStruct;
Jewel **pGrid;

uint8_t moves_left = 60;
uint16_t score = 0;

#define JEWEL_THRESHOLD 3

void drawList(char *[], int);
void drawGame(void);
void selectMenu(char *[]);
Jewel **create_grid(void);
void drawSelector(void);
bool end_game = false;

bool game_is_ended = false;
void rotateSelector(void);
bool validSwap(Selector selector);
void freeGrid(void);
void moveSelector(float, float, float);
bool can_move_selector = true;
uint8_t selector_delay = 10; // 10 seconds
int deleteJewel(Jewel**, Jewel, const unsigned int);
void drawModeSelection(void);
void selectVariants(char *[]);
void drawScore(void);
void drawDiamond(uint8_t, uint8_t);

void setupLevel(uint8_t level_index)
{
  if (level_index != START_LEVEL_INDEX && jewel_amount != MAX_JEWEL_AMOUNT)
  {
    jewel_amount++;
  }
  jewel_color_size = jewel_amount - 1;

  jewel_width = (M5.Lcd.width() - padding_x * 2) / jewel_amount;
  jewel_height = jewel_width;

  selector.rotation = horizontal;
  selector_width = jewel_width * 2;
  selector_height = jewel_height;

  level_multiplier = (level_index + 1);

  pGrid = create_grid();

  selector.pos_1.x = 1;
  selector.pos_2.x = 2;
  selector.pos_1.y = 1;
  selector.pos_2.y = 1;

  moves_left = ((jewel_amount * 2) / level_multiplier) * score_multiplier;
  drawGame();
}

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
  M5.Rtc.SetTime(&TimeStruct);
  // selector = create_selector();

  selector.pos_1.x = 1;
  selector.pos_2.x = 2;
  selector.pos_1.y = 1;
  selector.pos_2.y = 1;

  drawList(variants, VARIANTSIZE);
}

void printColor(unsigned int color)
{
  if (color == YELLOW)
  {
    Serial.print("Yellow");
  }
  else if (color == ORANGE)
  {
    Serial.print("Orange");
  }
  else if (color == BLACK)
  {
    Serial.print("Black");
  }
  else if (color == GREEN)
  {
    Serial.print("Green");
  }
  else if (color == RED)
  {
    Serial.print("Red");
  }
  else if (color == BLUE)
  {
    Serial.print("Blue");
  }
  else if (color == WHITE)
  {
    Serial.print("White");
  }
  else if (color == PURPLE)
  {
    Serial.print("Purple");
  }
  else if (color == PINK)
  {
    Serial.print("Pink");
  }
  else
  {
    Serial.print("Not a color");
  }
}

// the loop routine runs over and over again forever
void loop()
{
  int old_score = score;
  M5.update();
  if (!start_game)
  {
    selectVariants(variants);
    return;
  }

  if (isInMenu)
  {
    selectMenu(menu);
    return;
  }

  if (game_is_ended)
    return;
  if (end_game)
  {
    M5.Lcd.fillScreen(BLACK);
    freeGrid();
    game_is_ended = true;
    return;
  }

  if (M5.BtnA.wasPressed() && M5.BtnB.wasPressed())
  {
    if (validSwap(selector))
    {
      swapJewel(pGrid, selector);
      moves_left--;
      drawGame();
      score += deleteJewel(pGrid, pGrid[selector.pos_1.x][selector.pos_1.y], pGrid[selector.pos_1.x][selector.pos_1.y].color) * level_multiplier;
      score += deleteJewel(pGrid, pGrid[selector.pos_2.x][selector.pos_2.y], pGrid[selector.pos_2.x][selector.pos_2.y].color) * level_multiplier;
    }
  }
  else if (M5.BtnA.wasPressed())
  {
    rotateSelector();
    drawSelector();
  }
  else if (M5.BtnB.wasPressed())
  {
    drawList(menu, MENUSIZE);
    isInMenu = true;
    return;
  }
  M5.IMU.getAccelData(&acc_x, &acc_y, &acc_z);
  moveSelector(acc_x, acc_y, acc_z);
  end_game = (moves_left == 0);
  if (score > level_multiplier * 200)
  {
    level++;
    setupLevel(level);
  };

  for (int i = 0; i < jewel_amount; i++)
  {
    for (int j = 0; j < jewel_amount; j++)
    {
      score += deleteJewel(pGrid, pGrid[i][j], pGrid[i][j].color);
    }
  }
  if (old_score != score)
  {
    drawScore();
  }
  delay(300);
}

void moveSelector(float p, float r, float y)
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

  drawSelector();
}

void freeGrid(void)
{
  for (int i = 0; i < jewel_amount; i++)
    free(pGrid[i]);

  free(pGrid);
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

Jewel **create_grid(void)
{
  Jewel **new_grid = (Jewel **)malloc(jewel_amount * sizeof(Jewel **));

  for (int i = 0; i < jewel_amount; ++i)
  {
    new_grid[i] = (Jewel*)malloc(jewel_amount * sizeof(Jewel));
  }

  for (int i = 0; i < jewel_amount; ++i)
  {
    for (int j = 0; j < jewel_amount; ++j)
    {
      Jewel new_jewel;
      new_jewel.position.x = i;
      new_jewel.position.y = j;
      new_jewel.color = jewel_color[rand() % jewel_color_size];
      new_jewel.type = jewel;
      new_grid[i][j] = new_jewel;
    }
  }

  return new_grid;
}

void drawJewel(const uint8_t x, const uint8_t y)
{
  if (pGrid[x][y].type == jewel)
  {
    M5.Lcd.fillRect(padding_x + (x * jewel_width), padding_y + (y * jewel_height), jewel_width, jewel_height, pGrid[x][y].color);
  }
  else
    drawDiamond(x, y);
}

void drawSelectorBounds(const uint8_t x, const uint8_t y)
{
  // Draw the current jewel
  drawJewel(x, y);

  // Right
  if (x + 1 <= jewel_amount - 1)
    drawJewel(x + 1, y);
  // Left
  if (x - 1 >= 0)
    drawJewel(x - 1, y);
  // Top
  if (y + 1 <= jewel_amount - 1)
    drawJewel(x, y + 1);
  // Bottom
  if (y - 1 >= 0)
    drawJewel(x, y - 1);

  if (y + 1 <= jewel_amount - 1 && x + 1 <= jewel_amount - 1)
    drawJewel(x + 1, y + 1);

  if (y - 1 >= 0 && x + 1 <= jewel_amount - 1)
    drawJewel(x + 1, y - 1);

  if (x - 1 >= 0 && y - 1 >= 0)
    drawJewel(x - 1, y - 1);

  if (x - 1 >= 0 && y + 1 <= jewel_amount - 1)
    drawJewel(x - 1, y + 1);
}

void drawSelector(void)
{
  // Serial.print("Jewel1 x = ");
  // Serial.print(selector.pos_1.x);
  // Serial.print(" y = ");
  // Serial.print(selector.pos_1.y);
  // Serial.print("  color =  ");
  // printColor(pGrid[selector.pos_1.x][selector.pos_1.y].color);
  // Serial.println("  ");
  // Serial.print("Jewel2: x =  ");
  // Serial.print(selector.pos_1.x);
  // Serial.print(" y =  ");
  // Serial.print(selector.pos_2.y);
  // Serial.print("  color =  ");
  // printColor(pGrid[selector.pos_2.x][selector.pos_2.y].color);
  // Serial.println("_______________________");

  drawSelectorBounds(selector.pos_1.x, selector.pos_1.y);
  drawSelectorBounds(selector.pos_2.x, selector.pos_2.y);
  M5.Lcd.drawRect(padding_x + selector.pos_1.x * jewel_width, padding_y + selector.pos_1.y * jewel_height, selector_width, selector_height, BLACK);
}

void restartGame(void)
{
  moves_left = 12;
  freeGrid();
  pGrid = create_grid();
  M5.IMU.Init();
  drawGame();
}
int select(int idx, char *arr[], const int size)
{
  int selection = idx;
  int x, y;
  x = y = padding_x;
  if (M5.BtnB.wasPressed())
  {
    for (int i = 0; i < size; i++)
    {
      M5.Lcd.setTextColor(WHITE);
      if (i == selection)
      {
        M5.Lcd.setTextColor(RED);
      }
      M5.Lcd.setCursor(x, y + padding_x * i);
      M5.Lcd.printf(arr[i]);
    }
    selection = (selection + 1) % size;
    M5.Beep.tone(700, 100);
  }
  return selection;
}

void selectVariants(char *modes[])
{
  selectedItem = select(selectedItem, modes, VARIANTSIZE);
  if (M5.BtnA.wasPressed())
  {
    M5.Beep.tone(1000, 100);
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
      drawGame();
      selectedItem = 0;
      game_mode = HARD;
      break;
    default:
      break;
    }
  }
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

void selectMenu(char *menu[])
{
  selectedItem = select(selectedItem, menu, MENUSIZE);
  if (M5.BtnA.wasPressed())
  {
    M5.Beep.tone(1000, 100);
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
}
void drawList(char *arr[], const int size)
{
  int x, y;
  x = y = padding_x;
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

void drawDiamond(uint8_t x, uint8_t y)
{
  M5.Lcd.fillRect(padding_x + (x * jewel_width), padding_y + (y * jewel_height), jewel_width, jewel_height, BLACK);
  M5.Lcd.fillTriangle(padding_x + (x * jewel_width), padding_y + (y * jewel_height),
                      padding_x + ((x + 1) * jewel_width) - jewel_width / 2, padding_y + ((y + 1) * jewel_height) - 1,
                      padding_x + ((x + 1) * jewel_width) - 1, padding_y + (y * jewel_height),
                      pGrid[x][y].color);
}

void drawGrid(Jewel **grid)
{
  for (int i = 0; i < jewel_amount; i++)
  {
    for (int j = 0; j < jewel_amount; j++)
    {
      if (pGrid[i][j].type == diamond)
      {
        drawDiamond(i, j);
      }
      else
      {
        M5.Lcd.fillRect(padding_x + pGrid[i][j].position.x * jewel_width,
                        padding_y + pGrid[i][j].position.y * jewel_height,
                        jewel_width, jewel_height, pGrid[i][j].color);
      }
    }
  }
}

void drawScore(void)
{
  M5.Lcd.setCursor(padding_x, cursor_offset);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.fillRect(padding_x, cursor_offset, M5.Lcd.width(), padding_x, BLACK);
  M5.Lcd.printf("Moves left: %i", moves_left);
  M5.Lcd.setCursor(padding_x, cursor_offset * 2);
  M5.Lcd.fillRect(padding_x, cursor_offset * 2, M5.Lcd.width(), padding_x, BLACK);
  M5.Lcd.printf("Score: %i", score);
}

void drawGame(void)
{
  M5.Lcd.fillScreen(BLACK);
  drawScore();
  drawGrid(pGrid);
  drawSelector();
}

bool checkRowJewel(Jewel** grid, Jewel jewel, const uint16_t color)
{
  uint8_t right = 1;
  uint8_t left = 1;

  if (jewel.position.x + right < jewel_amount)
  {
    while (grid[jewel.position.x + right][jewel.position.y].color == color)
    {
      if (jewel.position.x + right < jewel_amount)
      {
        right++;
      }
      else
        break;
    }
  }

  if (jewel.position.x - left > 0)
  {
    while (grid[jewel.position.x - left][jewel.position.y].color == color)
    {
      if (jewel.position.x - left > 0)
      {
        left++;
      }
      else
        break;
    }
  }
  return right + left - 1 >= JEWEL_THRESHOLD;
}

bool checkColJewel(Jewel** grid, Jewel jewel, const uint16_t color)
{
  uint8_t top = 1;
  uint8_t bottom = 1;
  if (jewel.position.y + top < jewel_amount)
  {
    while (grid[jewel.position.x][jewel.position.y + top].color == color)
    {
      if (jewel.position.y + top < jewel_amount)
      {
        top++;
      }
      else
        break;
    }
  }
  if (jewel.position.y - bottom > 0)
  {
    while (grid[jewel.position.x][jewel.position.y - bottom].color == color)
    {
      if (jewel.position.y - bottom > 0)
      {
        bottom++;
      }
      else
        break;
    }
  }
  return top + bottom - 1 >= JEWEL_THRESHOLD;
}

// void swapJewel(Jewel** grid, Selector selector)
// {
//   uint16_t temp = grid[selector.pos_1.x][selector.pos_1.y].color;
//   grid[selector.pos_1.x][selector.pos_1.y].color = grid[selector.pos_2.x][selector.pos_2.y].color;
//   grid[selector.pos_2.x][selector.pos_2.y].color = temp;
// }

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
    if (grid[x + directionX * count][y + directionY * count].type == diamond) {
      grid[x + directionX * count][y + directionY * count].type = jewel;
      foundDiamond = true;
    }
    count++;
  }

  return count;
}

int deleteJewel(Jewel** grid, Jewel jewel, const unsigned int color)
{

  int right = findConsecutiveJewels(jewel.position.x, jewel.position.y, 1, 0, jewel_amount, grid, color);
  int left = findConsecutiveJewels(jewel.position.x, jewel.position.y, -1, 0, jewel_amount, grid, color);
  int top = findConsecutiveJewels(jewel.position.x, jewel.position.y, 0, 1, jewel_amount, grid, color);
  int bottom = findConsecutiveJewels(jewel.position.x, jewel.position.y, 0, -1, jewel_amount, grid, color);

    // if (foundDiamond && (right + left - 1 >= JEWEL_THRESHOLD || top + bottom - 1 >= JEWEL_THRESHOLD))
    // {
    //   for (int i = 0; i < jewel_amount - 1; i++)
    //   {
    //     dropCol(grid, i, jewel.position.y);
    //   }
    //     dropCol(grid, jewel.position.x, jewel_amount - 1);
    // }

  if (right + left - 1 >= JEWEL_THRESHOLD)
  {
    for (int i = 0; i < right; i++)
    {
      for (int j = 0; j <= jewel_width; j++)
      {
        M5.Lcd.fillRect(padding_x + ((jewel.position.x + i) * jewel_width), padding_y + (jewel.position.y * jewel_height), j, jewel_height, BLACK);
        delay(20);
      }
    }
    for (int i = 0; i < right; i++)
    {
      // for (int j = 0; j <= jewel.position.y; j++)
      // {
        dropCol(grid, jewel.position.x + i, jewel.position.y);
        //grid[jewel.position.x + i][jewel.position.y - j].color = (jewel.position.y - j != 0) ? grid[jewel.position.x + i][jewel.position.y - j - 1].color : jewel_color[rand() % jewel_color_size];
    }

    for (int i = 1; i < left; i++)
    {
      for (int j = 0; j <= jewel_width; j++)
      {
        M5.Lcd.fillRect(padding_x + ((jewel.position.x - i) * jewel_width), padding_y + (jewel.position.y * jewel_height), j, jewel_height, BLACK);
        delay(20);
      }
    }

    for (int i = 1; i < left; i++)
    {
      // for (int j = 0; j <= jewel.position.y; j++)
      // {
      //   //printColor(grid[jewel.position.x - i][jewel.position.y - j].color);
      //   grid[jewel.position.x - i][jewel.position.y - j].color = (jewel.position.y - j != 0) ? grid[jewel.position.x - i][jewel.position.y - 1 - j].color : jewel_color[rand() % jewel_color_size];
      // }
      dropCol(grid, jewel.position.x - i, jewel.position.y);
    }

  //  Serial.println("AFTER "); 
  //   for (int i = 0; i < jewel_amount; i++) {
  //     for (int j = 0; j < jewel_amount; j++) {
 
  //       //printColor(grid[i][j].color);
  //       Serial.print(" "); 
  //     }
  //     Serial.println(" "); 
  //   }
    drawGame();
    if (right + left - 1 > JEWEL_THRESHOLD)
      grid[jewel.position.x][jewel.position.y].type = diamond;
    return (score_multiplier * (right + left - JEWEL_THRESHOLD)) * level_multiplier;
  }


  if (top + bottom - 1 >= JEWEL_THRESHOLD)
  {
    for (int i = 0; i < top; i++)
    {
      for (int j = 0; j <= jewel_height; j++)
      {
        M5.Lcd.fillRect(padding_x + (jewel.position.x * jewel_width), padding_y + ((jewel.position.y + i) * jewel_height), jewel_width, j, BLACK);
        delay(20);
      }
    }
    for (int i = 0; i < top; i++)
    {
      dropCol(grid, jewel.position.x, jewel.position.y + i);
    }

    for (int i = 1; i < bottom; i++)
    {
      for (int j = 0; j <= jewel_height; j++)
      {
        M5.Lcd.fillRect(padding_x + (jewel.position.x * jewel_width), padding_y + ((jewel.position.y - i) * jewel_height), jewel_width, j, BLACK);
        delay(20);
      }
    }

    for (int i = 1; i < bottom; i++)
    {
      dropCol(grid, jewel.position.x, jewel.position.y - i);
    }
    drawGame();

    if (top + bottom - 1 > JEWEL_THRESHOLD)
      grid[jewel.position.x][jewel.position.y].type = diamond;
    return (score_multiplier * (top + bottom - JEWEL_THRESHOLD)) * level_multiplier;
  }

  return 0;
}