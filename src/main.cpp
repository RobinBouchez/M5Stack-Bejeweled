#include <M5StickCPlus.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "EEPROM.h"
#include <vector>
// #include "../lib/grid/grid.h"
#include "../include/menu.h"
#include "../include/gamemode.h"

#include "../lib/jewel/jewel.h"
// #include "../lib/selector/selector.h"

#define VARIANTSIZE 2
char *variants[VARIANTSIZE]{"EASY", "HARD"};

enum Rotation
{
  vertical,
  horizontal
};

enum gameMode
{
  EASY = 1,
  HARD = 0
};

gameMode game_mode;

typedef struct Jewelpos
{
  u_int x : 4;
  u_int y : 4;
};

enum jewelType{jewel, diamond};
struct jewel
{
  Jewelpos position;
  u_int16_t color;
  jewelType type;
};
typedef struct jewel Jewel;

u_int8_t level = 0;
struct selectionRect
{
  Jewelpos pos_1;
  Jewelpos pos_2;
  Rotation rotation;
};
typedef struct selectionRect Selector;

Selector selector;

#define MIN_TILT_X 0.15
#define MIN_TILT_Y 0.20
#define MAX_TILT_Y 0.65

u_int8_t padding_x = 20;
u_int8_t padding_y = 80;
u_int8_t cursor_offset = 20;
u_int8_t jewel_width = 12;
u_int8_t jewel_height = 12;
u_int8_t selector_x = 0;
u_int8_t selector_y = 0;
u_int8_t selector_width = jewel_width * 2;
u_int8_t selector_height = jewel_height;
int selectedItem = 0;
u_int8_t level_multiplier = 1;
u_int8_t score_multiplier = 10;

bool start_game = false;
#define MAX_JEWEL_AMOUNT 8
#define MAX_JEWEL_COLOR_AMOUNT MAX_JEWEL_AMOUNT - 1
#define MIN_JEWEL_AMOUNT 4
u_int8_t jewel_color_size = 7;
u_int8_t jewel_amount = MIN_JEWEL_AMOUNT;
u_int16_t jewel_color[MAX_JEWEL_COLOR_AMOUNT]{RED, BLUE, GREEN, WHITE, ORANGE, YELLOW, PINK};

float acc_x = 0.f;
float acc_y = 0.f;
float acc_z = 0.f;

RTC_TimeTypeDef TimeStruct;
Jewel ***pGrid;


u_int8_t moves_left = 60;
u_int16_t score = 0;

#define JEWEL_THRESHOLD 3

void drawList(char *[], int);
void drawGame(void);
void selectMenu(char *[]);
Jewel ***create_grid(void);
void drawSelector(void);
bool end_game = false;
void swapJewel(void);
bool game_is_ended = false;
void rotateSelector(void);
bool validSwap(Selector selector);
void freeGrid(void);
void moveSelector(float, float, float);
bool can_move_selector = true;
u_int8_t selector_delay = 10; // 10 seconds
int deleteJewel(Jewel *jewel, const unsigned int color);
void drawModeSelection(void);
void selectVariants(char *[]);
void checkJewels(Jewel *jewel, const unsigned int color);
void drawScore(void);
void drawDiamond(u_int8_t, u_int8_t);
// Selector *create_selector(void)
// {
//   Selector *s = (Selector *)malloc(sizeof(Selector));
//   // s->x1 = (u_int8_t *)malloc(sizeof(Jewel));
//   // s->jewel2 = (Jewel *)malloc(sizeof(Jewel));
//   // s->x1 = (u_int8_t)malloc(sizeof(u_int8_t));
//   // s->x2 = (u_int8_t)malloc(sizeof(u_int8_t));
//   // s->y1 = (u_int8_t)malloc(sizeof(u_int8_t));
//   // s->y2 = (u_int8_t)malloc(sizeof(u_int8_t));
//   // s->rotation = (u_int8_t)malloc(sizeof(u_int8_t));

//   // s->x1 =
//   //  s->jewel1 = pGrid[3][1];
//   //  s->jewel2 = pGrid[4][1];
//   s->rotation = horizontal;

//   return s;
// }

void setupLevel(u_int8_t level_index)
{
  if (jewel_amount != MAX_JEWEL_AMOUNT)
  {
    jewel_amount += level_index;
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
    Serial.println("Yellow");
  }
  else if (color == ORANGE)
  {
    Serial.println("Orange");
  }
  else if (color == BLACK)
  {
    Serial.println("Black");
  }
  else if (color == GREEN)
  {
    Serial.println("Green");
  }
  else if (color == RED)
  {
    Serial.println("Red");
  }
  else if (color == BLUE)
  {
    Serial.println("Blue");
  }
  else if (color == WHITE)
  {
    Serial.println("White");
  }
  else if (color == PURPLE)
  {
    Serial.println("Purple");
  }
  else if (color == PINK)
  {
    Serial.println("Pink");
  }
  else
  {
    Serial.println("Not a color");
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
      swapJewel();
      moves_left--;
      drawGame();
      score += deleteJewel(pGrid[selector.pos_1.x][selector.pos_1.y], pGrid[selector.pos_1.x][selector.pos_1.y]->color) * level_multiplier;
      score += deleteJewel(pGrid[selector.pos_2.x][selector.pos_2.y], pGrid[selector.pos_2.x][selector.pos_2.y]->color) * level_multiplier;
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
  if (score > level_multiplier * 100)
  {
    level++;
    setupLevel(level);
  };

  for (int i = 0; i < jewel_amount; i++)
  {
    for (int j = 0; j < jewel_amount; j++)
    {
      score += deleteJewel(pGrid[i][j], pGrid[i][j]->color);
    }
  }
  if(old_score != score) { 
    drawScore(); }
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

  if ((u_int)selector.pos_1.x == (u_int)old.x && (u_int)selector.pos_1.y == (u_int)old.y)
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
  {
    for (int j = 0; j < jewel_amount; j++)
    {
      free(pGrid[i][j]);
    }
  }

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

Jewel ***create_grid(void)
{
  Jewel ***new_grid = (Jewel ***)malloc(jewel_amount * sizeof(Jewel **));

  for (int i = 0; i < jewel_amount; ++i)
  {
    new_grid[i] = (Jewel **)malloc(jewel_amount * sizeof(Jewel *));
  }

  for (int i = 0; i < jewel_amount; ++i)
  {
    for (int j = 0; j < jewel_amount; ++j)
    {
      Jewel *new_jewel = (Jewel *)malloc(sizeof(Jewel));
      new_jewel->position.x = i;
      new_jewel->position.y = j;
      new_jewel->color = jewel_color[rand() % jewel_color_size];
      new_jewel->type = jewel;
      new_grid[i][j] = new_jewel;
    }
  }

  return new_grid;
}
void drawJewel(const u_int8_t x, const u_int8_t y)
{
  if(pGrid[x][y]->type == jewel) {
    M5.Lcd.fillRect(padding_x + (x * jewel_width), padding_y + (y * jewel_height), jewel_width, jewel_height, pGrid[x][y]->color);
  }
  else drawDiamond(x, y);
}
void drawSelectorBounds(const u_int8_t x, const u_int8_t y)
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
  Serial.print("Jewel1 x = ");
  Serial.print(selector.pos_1.x);
  Serial.print(" y = ");
  Serial.print(selector.pos_1.y);
  Serial.print("  color =  ");
  printColor(pGrid[selector.pos_1.x][selector.pos_1.y]->color);
  Serial.println("  ");
  Serial.print("Jewel2: x =  ");
  Serial.print(selector.pos_1.x);
  Serial.print(" y =  ");
  Serial.print(selector.pos_2.y);
  Serial.print("  color =  ");
  printColor(pGrid[selector.pos_2.x][selector.pos_2.y]->color);
  Serial.println("_______________________");

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

  address++;
  for (int i = 0; i < jewel_amount; i++)
  {
    for (int j = 0; j < jewel_amount; j++)
    {
      uint8_t lower_color = pGrid[i][j]->color % 256;
      uint8_t upper_color = pGrid[i][j]->color >> 8;

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
  for (int i = 0; i < jewel_amount; i++)
  {
    for (int j = 0; j < jewel_amount; j++)
    {
      uint8_t lower_color = EEPROM.readByte(address);
      address++;
      uint8_t upper_color = EEPROM.readByte(address);
      pGrid[i][j]->color = (upper_color << 8) | lower_color;
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

void drawDiamond(u_int8_t x, u_int8_t y){
  M5.Lcd.fillRect(padding_x + (x * jewel_width), padding_y + (y * jewel_height), jewel_width, jewel_height, BLACK);
  M5.Lcd.fillTriangle(padding_x + (x * jewel_width), padding_y + (y * jewel_height), 
                      padding_x + ((x + 1) * jewel_width) - jewel_width / 2, padding_y + ((y + 1) * jewel_height) - 1,
                      padding_x + ((x + 1) * jewel_width) - 1, padding_y + (y * jewel_height),
                      pGrid[x][y]->color);
}

void drawGrid(Jewel ***grid)
{
  for (int i = 0; i < jewel_amount; i++)
  {
    for (int j = 0; j < jewel_amount; j++)
    {
      M5.Lcd.fillRect(padding_x + pGrid[i][j]->position.x * jewel_width,
                      padding_y + pGrid[i][j]->position.y * jewel_height,
                      jewel_width, jewel_height, pGrid[i][j]->color);
    }
  }
}
void drawScore(void) {
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

bool checkRowJewel(Jewel *jewel, const u_int16_t color)
{
  u_int8_t right = 1;
  u_int8_t left = 1;

  if (jewel->position.x + right < jewel_amount)
  {
    while (pGrid[jewel->position.x + right][jewel->position.y]->color == color)
    {
      if (jewel->position.x + right < jewel_amount)
      {
        right++;
      }
      else
        break;
    }
  }

  if (jewel->position.x - left > 0)
  {
    while (pGrid[jewel->position.x - left][jewel->position.y]->color == color)
    {
      if (jewel->position.x - left > 0)
      {
        left++;
      }
      else
        break;
    }
  }
  return right + left - 1 >= JEWEL_THRESHOLD;
}

bool checkColJewel(Jewel *jewel, const u_int16_t color)
{
  u_int8_t top = 1;
  u_int8_t bottom = 1;
  if (jewel->position.y + top < jewel_amount)
  {
    while (pGrid[jewel->position.x][jewel->position.y + top]->color == color)
    {
      if (jewel->position.y + top < jewel_amount)
      {
        top++;
      }
      else
        break;
    }
  }
  if (jewel->position.y - bottom > 0)
  {
    while (pGrid[jewel->position.x][jewel->position.y - bottom]->color == color)
    {
      if (jewel->position.y - bottom > 0)
      {
        bottom++;
      }
      else
        break;
    }
  }
  return top + bottom - 1 >= JEWEL_THRESHOLD;
}

void swapJewel(void)
{
  u_int16_t temp = pGrid[selector.pos_1.x][selector.pos_1.y]->color;
  pGrid[selector.pos_1.x][selector.pos_1.y]->color = pGrid[selector.pos_2.x][selector.pos_2.y]->color;
  pGrid[selector.pos_2.x][selector.pos_2.y]->color = temp;

}

bool validSwap(Selector selector)
{
  if (game_mode == EASY)
  {
    return true;
  }
  else
  {
    return checkRowJewel(pGrid[selector.pos_1.x][selector.pos_1.y], pGrid[selector.pos_2.x][selector.pos_2.y]->color) ||
           checkColJewel(pGrid[selector.pos_2.x][selector.pos_2.y], pGrid[selector.pos_1.x][selector.pos_1.y]->color) ||
           checkRowJewel(pGrid[selector.pos_2.x][selector.pos_2.y], pGrid[selector.pos_1.x][selector.pos_1.y]->color) ||
           checkColJewel(pGrid[selector.pos_1.x][selector.pos_1.y], pGrid[selector.pos_2.x][selector.pos_2.y]->color);
  }
}

void dropCol(int x, int y)
{
  for (int i = 0; i <= y; i++)
  {
    pGrid[x][y - i]->color = (y - i > 0) ? pGrid[x][y - i - 1]->color : jewel_color[rand() % jewel_color_size];
  }
}

void checkJewels(Jewel *jewel, const unsigned int color){
  int right = 1;
  while ((jewel->position.x + right < jewel_amount) && (pGrid[jewel->position.x + right][jewel->position.y]->color == color))
  {
    right++;
  }

  int left = 1;
  while ((jewel->position.x - left > 0) && (pGrid[jewel->position.x - left][jewel->position.y]->color == color))
  {
    left++;
  }

  if (right + left - 1 >= JEWEL_THRESHOLD)
  {
    for (int i = 0; i < right; i++)
    {
      for (int j = 0; j < jewel_width; j++)
      {
        M5.Lcd.fillRect(padding_x + ((jewel->position.x + i) * jewel_width), padding_y + (jewel->position.y * jewel_height), j, jewel_height, BLACK);
        delay(20);
      }
    }
    for (int i = 0; i < right; i++)
    {
      dropCol(jewel->position.x + i, jewel->position.y);
    }


    for (int i = 0; i < left; i++)
    {
      for (int j = 0; j < jewel_width; j++)
      {
        M5.Lcd.fillRect(padding_x + ((jewel->position.x - i) * jewel_width), padding_y + (jewel->position.y * jewel_height), j, jewel_height, BLACK);
        delay(20);
      }
    }

    for (int i = 0; i < left; i++)
    {
      dropCol(jewel->position.x - i, jewel->position.y);
    }
  }
}

int deleteJewel(Jewel *jewel, const unsigned int color)
{
  int right = 1;
  while ((jewel->position.x + right < jewel_amount) && (pGrid[jewel->position.x + right][jewel->position.y]->color == color))
  {
    right++;
  }

  int left = 1;
  while ((jewel->position.x - left > 0) && (pGrid[jewel->position.x - left][jewel->position.y]->color == color))
  {
    left++;
  }

  int top = 1;
  while ((jewel->position.y + top < jewel_amount) && (pGrid[jewel->position.x][jewel->position.y + top]->color == color))
  {
    top++;
  }

  int bottom = 1;
  while ((jewel->position.y - bottom > 0) && (pGrid[jewel->position.x][jewel->position.y - bottom]->color == color))
  {
    bottom++;
  }


  if (right + left - 1 >= JEWEL_THRESHOLD)
  {
    for (int i = 0; i < right; i++)
    {
      for (int j = 0; j <= jewel_width; j++)
      {
        M5.Lcd.fillRect(padding_x + ((jewel->position.x + i) * jewel_width), padding_y + (jewel->position.y * jewel_height), j, jewel_height, BLACK);
        delay(20);
      }
    }
    for (int i = 0; i < right; i++)
    {
      dropCol(jewel->position.x + i, jewel->position.y);
    }


    for (int i = 0; i < left; i++)
    {
      for (int j = 0; j <= jewel_width; j++)
      {
        M5.Lcd.fillRect(padding_x + ((jewel->position.x - i) * jewel_width), padding_y + (jewel->position.y * jewel_height), j, jewel_height, BLACK);
        delay(20);
      }
    }

    for (int i = 0; i < left; i++)
    {
      dropCol(jewel->position.x - i, jewel->position.y);
    }
    drawGame();
    if(right + left - 1 > JEWEL_THRESHOLD) pGrid[jewel->position.x][jewel->position.y]->type = diamond;
    return (score_multiplier * (right + left - JEWEL_THRESHOLD)) * level_multiplier;
  }

  if (top + bottom - 1 >= JEWEL_THRESHOLD)
  {
    for (int i = 0; i < top; i++)
    {
      for (int j = 0; j <= jewel_height; j++)
      {
        M5.Lcd.fillRect(padding_x + (jewel->position.x * jewel_width), padding_y + ((jewel->position.y + i) * jewel_height), jewel_width, j, BLACK);
        delay(20);
      }
    }
    for (int i = 0; i < top; i++)
    {
      dropCol(jewel->position.x, jewel->position.y + i);
    }


    for (int i = 0; i < bottom; i++)
    {
      for (int j = 0; j <= jewel_height; j++)
      {
        M5.Lcd.fillRect(padding_x + (jewel->position.x * jewel_width), padding_y + ((jewel->position.y - i) * jewel_height), jewel_width, j, BLACK);
        delay(20);
      }
    }

    for (int i = 0; i < bottom; i++)
    {
      dropCol(jewel->position.x, jewel->position.y - i);
    }
    drawGame();

    if(top + bottom - 1 > JEWEL_THRESHOLD) pGrid[jewel->position.x][jewel->position.y]->type = diamond;
    return (score_multiplier * (top + bottom - JEWEL_THRESHOLD)) * level_multiplier;
  }

  return 0;
}