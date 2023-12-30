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
struct selectionRect
{
  u_int8_t x1;
  u_int8_t y1;
  u_int8_t x2;
  u_int8_t y2;
  Rotation rotation;
};
typedef struct selectionRect Selector;

Selector selector;

#define MIN_TILT_X 0.15
#define MIN_TILT_Y 0.35
#define MAX_TILT_Y 0.75

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

bool start_game = false;

const u_int8_t jewel_color_size = 7;
const u_int8_t jewel_amount = 8;
unsigned int jewel_color[jewel_color_size]{RED, BLUE, WHITE, GREEN, ORANGE, YELLOW, PINK};

float acc_x = 0.f;
float acc_y = 0.f;
float acc_z = 0.f;

RTC_TimeTypeDef TimeStruct;
Jewel ***pGrid;

u_int8_t moves_left = 60;
u_int16_t score = 0;

void drawList(char *[], int);
void drawGame(void);
void selectMenu(char *[]);
Jewel ***create_grid(void);
void drawSelector(void);
bool end_game = false;
void swapJewel(void);
bool game_is_ended = false;
void rotateSelector(void);
bool valid_swap(Selector);
void freeGrid(void);
void moveSelector(float, float, float);
bool can_move_selector = true;
u_int8_t selector_delay = 10; // 10 seconds
// void delete_jewel(void);
int delete_jewel(Jewel* jewel, const unsigned int color);
unsigned int destroy_jewel(Selector);
void drawModeSelection(void);
void selectVariants(char *[]);

Selector *create_selector(void)
{
  Selector *s = (Selector *)malloc(sizeof(Selector));
  // s->x1 = (u_int8_t *)malloc(sizeof(Jewel));
  // s->jewel2 = (Jewel *)malloc(sizeof(Jewel));
  // s->x1 = (u_int8_t)malloc(sizeof(u_int8_t));
  // s->x2 = (u_int8_t)malloc(sizeof(u_int8_t));
  // s->y1 = (u_int8_t)malloc(sizeof(u_int8_t));
  // s->y2 = (u_int8_t)malloc(sizeof(u_int8_t));
  //s->rotation = (u_int8_t)malloc(sizeof(u_int8_t));

  //s->x1 = 
  // s->jewel1 = pGrid[3][1];
  // s->jewel2 = pGrid[4][1];
  s->rotation = horizontal;

  return s;
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

  M5.Rtc.SetTime(&TimeStruct);

  pGrid = create_grid();
  //selector = create_selector();
  selector.rotation = horizontal;
  selector.x1 = 3;
  selector.x2 = 4;
  selector.y1 = 1;
  selector.y2 = 1;
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
    if (valid_swap(selector))
    {
      swapJewel();

      score = destroy_jewel(selector);
      // printColor(selector->jewel1->color);
      // printColor(selector->jewel2->color);
      moves_left--;
      score += delete_jewel(pGrid[selector.x1][selector.y1], pGrid[selector.x1][selector.y1]->color) * level_multiplier;
      score += delete_jewel(pGrid[selector.x2][selector.y2], pGrid[selector.x2][selector.y2]->color) * level_multiplier;
      // delete_jewel();
      drawGame();
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
  delay(320);
}

void moveSelector(float p, float r, float y)
{
  u_int8_t old_x = selector.x1;
  u_int8_t old_y = selector.y1;

  if (!can_move_selector)
    return;

  if (p > MIN_TILT_X)
  {
    if (selector.x1 > 0)
    {
      selector.x1 -= 1;
    }
  }
  else if (p < -MIN_TILT_X)
  {
    if (selector.x1 < jewel_amount - 1)
    {
      selector.x1 += 1;
    }
  }

  if (r > MAX_TILT_Y)
  {
    if (selector.y1 < jewel_amount - 1)
    {
      selector.y1 += 1;
    }
  }
  else if (r < MIN_TILT_Y)
  {
    if (selector.y1 > 0)
    {
      selector.y1 -= 1;
    }
  }

  if (selector.x1 == old_x && selector.y1 == old_y)
    return;

  if (selector.rotation == horizontal)
  {
    selector.x2 = selector.x1 + 1;
    selector.y2 = selector.y1;
  }
  else
  {
    selector.x2 = selector.x1;
    selector.y2 = selector.y1 + 1;
  }

  if (selector.rotation == horizontal)
  {
    if (selector.x1 <= 0 || selector.x2 <= 1)
    {
      selector.x1 = 0;
    }
    else if (selector.x1 >= jewel_amount - 2)
    {
      selector.x1 = jewel_amount - 2;
    }

    if (selector.y1 <= 0 || selector.y1 == 255)
    {
      selector.y1 = 0;
    }
    if (selector.y1 >= jewel_amount - 1)
    {
      selector.y1 = jewel_amount - 1;
    }
  }
  else // vertical
  {
    if (selector.x1 <= 0 || selector.x1 == 255)
    {
      selector.x1 = 0;
    }
    if (selector.x1 >= jewel_amount - 1)
    {
      selector.x1 = jewel_amount - 1;
    }

    if (selector.y1 <= 0 || selector.y2 <= 1)
    {
      selector.y1 = 0;
    }
    else if (selector.y1 >= jewel_amount - 2)
    {
      selector.y1 = jewel_amount - 2;
    }
    else if (selector.y1 >= jewel_amount - 1)
    {
      selector.y1 = jewel_amount - 1;
    }
  }

  if (selector.rotation == horizontal)
  {
    selector.x2 = selector.x1 + 1;
    selector.y2 = selector.y1;
  }
  else
  {
    selector.x2 = selector.x1;
    selector.y2 = selector.y1 + 1;
  }
  // printColor(selector->jewel1->color);
  // printColor(selector->jewel2->color);

  // selector->jewel1->color = pGrid[selector->jewel1->x][selector->jewel1->y]->color;
  // selector->jewel2->color = pGrid[selector->jewel2->x][selector->jewel2->y]->color;
  // printColor(selector->jewel1->color);
  // printColor(selector->jewel2->color);
  printColor(pGrid[selector.x1][selector.y1]->color);
    printColor(pGrid[selector.x2][selector.y2]->color);
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
    selector_width = jewel_width;
    selector_height = jewel_height * 2;
    selector.rotation = vertical;
  }
  else
  {
    selector_width = jewel_width * 2;
    selector_height = jewel_height;
    selector.rotation = horizontal;
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
      new_jewel->x = i;
      new_jewel->y = j;
      new_jewel->color = jewel_color[rand() % jewel_color_size];
      new_grid[i][j] = new_jewel;
    }
  }

  return new_grid;
}
void drawJewel(const u_int8_t x, const u_int8_t y)
{
  M5.Lcd.fillRect(padding_x + (x * jewel_width), padding_y + (y * jewel_height), jewel_width, jewel_height, pGrid[x][y]->color);
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
  // Serial.print("Jewel1 x = ");
  // Serial.print(selector->jewel1->x);
  // Serial.print(" y = ");
  // Serial.print(selector->jewel1->y);
  // Serial.print("  color =  ");
  // printColor(selector->jewel1->color);
  // Serial.println("  ");
  // Serial.print("Jewel2: x =  ");
  // Serial.print(selector->jewel2->x);
  // Serial.print(" y =  ");
  // Serial.print(selector->jewel2->y);
  // Serial.print("  color =  ");
  // printColor(selector->jewel2->color);
  // Serial.println("_______________________");

  drawSelectorBounds(selector.x1, selector.y1);
  drawSelectorBounds(selector.x2, selector.y2);
  M5.Lcd.drawRect(padding_x + selector.x1 * jewel_width, padding_y + selector.y1 * jewel_height, selector_width, selector_height, BLACK);
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
  }
  return selection;
}

void selectVariants(char *modes[])
{
  selectedItem = select(selectedItem, modes, VARIANTSIZE);
  if (M5.BtnA.wasPressed())
  {
    switch (selectedItem)
    {
    case EASY:
      start_game = true;
      drawGame();
      selectedItem = 0;
      break;
    case HARD:
      start_game = true;
      drawGame();
      selectedItem = 0;
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
    switch (selectedItem)
    {
    case BACK:
      isInMenu = false;
      drawGame();
      break;
    case LOAD:
      loadGame();
      isInMenu = false;
      drawGame();
      break;
    case SAVE:
      saveGame();
      isInMenu = false;
      drawGame();
      break;
    case RESET:
      isInMenu = false;
      restartGame();
      break;
    default:
      break;
    }
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

void drawGrid(Jewel ***grid)
{
  float padding = (M5.Lcd.width() / 2) - (jewel_width * (jewel_amount / 2));
  for (int i = 0; i < jewel_amount; i++)
  {
    for (int j = 0; j < jewel_amount; j++)
    {
      M5.Lcd.fillRect(padding_x + pGrid[i][j]->x * jewel_width,
                      padding_y + pGrid[i][j]->y * jewel_height,
                      jewel_width, jewel_height, pGrid[i][j]->color);
    }
  }
}

void drawGame(void)
{
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(padding_x, cursor_offset);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.printf("Moves left: %i", moves_left);
  M5.Lcd.setCursor(padding_x, cursor_offset * 2);
  M5.Lcd.printf("Score: %i", score);

  drawGrid(pGrid);
  drawSelector();
}

void updateGrid(Jewel ***grid)
{
  for (int i = 0; i < jewel_amount; i++)
  {
    for (int j = 0; j < jewel_amount; j++)
    {
    }
  }
}

bool check_same_jewel(Jewel jewel, const unsigned int color)
{
  int same_jewel = 0;
  for (int i = 0; i < jewel_amount; i++)
  {
    if (same_jewel > 2)
      return true;
    if (pGrid[i][jewel.y]->color == color)
    {
      same_jewel++;
    }
    else
      same_jewel = 0;
  }
  for (int i = 0; i < jewel_amount; i++)
  {
    if (same_jewel > 2)
      return true;
    if (pGrid[jewel.x][i]->color == color)
    {
      same_jewel++;
    }
    else
      same_jewel = 0;
  }
  return false;
}

bool checkRowJewel(Jewel *jewel, const unsigned int color)
{
  u_int8_t right = 1;
  u_int8_t left = 1;

  while (pGrid[jewel->x + right][jewel->y]->color == color)
  {
    if (jewel->x + right < jewel_amount)
    {
      right++;
    }
    else
      break;
  }

  while (pGrid[jewel->x - left][jewel->y]->color == color)
  {
    if (jewel->x - left > 0)
    {
      left++;
    }
    else
      break;
  }
  return right + left >= 3;
}

bool checkColJewel(Jewel *jewel, const unsigned int color)
{
  u_int8_t top = 1;
  u_int8_t bottom = 1;

  while (pGrid[jewel->x][jewel->y + top]->color == color)
  {
    if (jewel->y + top < jewel_amount)
    {
      top++;
    }
    else
      break;
  }

  while (pGrid[jewel->x][jewel->y - bottom]->color == color)
  {
    if (jewel->y - bottom > 0)
    {
      bottom++;
    }
    else
      break;
  }
  return top + bottom >= 3;
}

void swapJewel(void)
{
  u_int16_t temp =   pGrid[selector.x1][selector.y1]->color;
  pGrid[selector.x1][selector.y1]->color = pGrid[selector.x2][selector.y2]->color;
  pGrid[selector.x2][selector.y2]->color = temp;
}

bool valid_swap(Selector s)
{
  return checkRowJewel(pGrid[s.x1][s.y1], pGrid[s.x2][s.y2]->color) ||
         checkColJewel(pGrid[s.x2][s.y2], pGrid[s.x1][s.y1]->color) ||
         checkRowJewel(pGrid[s.x2][s.y2], pGrid[s.x1][s.y1]->color) ||
         checkColJewel(pGrid[s.x1][s.y1], pGrid[s.x2][s.y2]->color);
}

unsigned int destroy_jewel(Selector selector)
{
  unsigned int score = 10;
  int jewel1_x = selector.x1;
  int jewel1_y = selector.y1;

  // do
  // {
  //   jewel1_x++;
  // } while (pGrid[jewel1_x][jewel1_y]->color == selector->jewel2->color);

  // do
  // {
  //   jewel1_x--;
  // } while (pGrid[jewel1_x][jewel1_y]->color == selector->jewel2->color);
  // //  free(pGrid[jewel1_x][jewel1_y]);

  return score;
}

// void delete_jewel()
// {
//   int jewel1_x = selector->jewel1->x;
//   int jewel1_y = selector->jewel1->y;
//   int jewel2_x = selector->jewel2->x;
//   int jewel2_y = selector->jewel2->y;
//   do
//   {
//     jewel1_x++;
//     selector->jewel1->color = BLACK;
//   } while (pGrid[jewel1_x][jewel1_y]->color == selector->jewel1->color);

//   do
//   {
//     jewel2_x++;
//     selector->jewel2->color = BLACK;
//   } while (pGrid[jewel2_x][jewel2_y]->color == selector->jewel2->color);

// }

void dropCol(int x, int y)
{
  for (int i = 0; i <= y; i++)
  {
    if (y - i <= 0)
    {
      pGrid[x][y - i]->color = jewel_color[rand() % jewel_color_size];
    }
    else
    {
      pGrid[x][y - i]->color = pGrid[x][y - i - 1]->color;
    }
  }
}

int delete_jewel(Jewel* jewel, const unsigned int color)
{
  int right = 1;
  int left = 1;

  while (pGrid[jewel->x + right][jewel->y]->color == color)
  {
    right++;
  }

  if (right >= 3)
  {
    for (int i = 0; i < right; i++)
    {
      pGrid[jewel->x + i][jewel->y]->color = BLACK;
      dropCol(jewel->x + i, jewel->y);
    }
  }

  while (pGrid[jewel->x - left][jewel->y]->color == color)
  {
    left++;
  }

  if (left >= 3)
  {
    for (int i = 0; i < left; i++)
    {
      pGrid[jewel->x - i][jewel->y]->color = BLACK;
      dropCol(jewel->x - i, jewel->y);
    }
  }
  return right + left;
}

void fall_bricks(Jewel *jwl, unsigned int amount, int xdir, int ydir)
{
  for (int i = 0; i < amount; i++)
  {
    for (int j = 0; j < jewel_amount - jwl->y * ydir; j++)
    {
      pGrid[jwl->x + i * xdir][jwl->y + i * ydir]->color = pGrid[jwl->x + i * xdir + j][jwl->y + i * ydir + j]->color;
    }
  }
}