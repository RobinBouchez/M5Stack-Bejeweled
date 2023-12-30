#include <M5StickCPlus.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "EEPROM.h"
#include <vector>
// #include "../lib/grid/grid.h"
// #include "../lib/menu/menu.h"
#include "../lib/jewel/jewel.h"
#include "../lib/selector/selector.h"



u_int8_t padding_x = 20;
u_int8_t padding_y = 80;
u_int8_t cursor_offset = 20;
u_int8_t selectedItem = 0;
u_int8_t offset = 0;
u_int8_t jewel_width = 12;
u_int8_t jewel_height = 12;
u_int8_t selector_x = 0;
u_int8_t selector_y = 0;
u_int8_t selector_width = jewel_width * 2;
u_int8_t selector_height = jewel_height;

const u_int8_t jewel_color_size = 7;
const u_int8_t jewel_amount = 8;
unsigned int jewel_color[jewel_color_size]{RED, BLUE, WHITE, GREEN, ORANGE, YELLOW, PINK};

float pitch = 0.f;
float roll = 0.f;
float yaw = 0.f;

RTC_TimeTypeDef TimeStruct;
Jewel ***pGrid;

bool isInMenu = false;
const u_int8_t menuSize = 4;
char *menu[menuSize]{"BACK", "SAVE", "LOAD", "RESET LEVEL"};

unsigned int moves_left = 30;
unsigned int score = 0;

void displayMenu(void);
void drawGame(void);
void selectMenu(void);
Jewel ***create_grid(void);
void drawSelector(void);
bool end_game = false;
void swapJewel(Jewel*, Jewel*);
bool game_is_ended = false;
void rotateSelector(void);
bool valid_swap(Selector*);
void freeGrid(void);
void moveSelector(float, float, float);
bool can_move_selector = false;
u_int8_t selector_delay = 10; // 10 seconds
void delete_jewel(void);
unsigned int destroy_jewel(Selector*);

Selector *create_selector(void)
{
  Selector *s = (Selector *)malloc(sizeof(Selector));
  s->jewel1 = (Jewel *)malloc(sizeof(Jewel));
  s->jewel2 = (Jewel *)malloc(sizeof(Jewel));

  s->jewel1 = pGrid[3][1];
  s->jewel2 = pGrid[4][1];

  selector_x = padding_x + s->jewel1->x * jewel_width;
  selector_y = padding_y + s->jewel1->y * jewel_height;

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
  selector = create_selector();
  drawGame();
}

// the loop routine runs over and over again forever
void loop()
{
  M5.update();

  if (isInMenu)
  {
    selectMenu();
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
      swapJewel(selector->jewel1, selector->jewel2);
      //score = destroy_jewel(selector);
      moves_left--;
      //delete_jewel();
      drawGame();
    }
  }
  else if (M5.BtnA.wasPressed())
  {
    rotateSelector();
    drawGame();
  }
  else if (M5.BtnB.wasPressed())
  {
    selectedItem = offset = 0;
    displayMenu();
    isInMenu = true;
  }
  if (!isInMenu)
    drawSelector();

  M5.IMU.getAhrsData(&pitch, &roll, &yaw);
  moveSelector(pitch, roll, yaw);

  end_game = (moves_left == 0);
  delay(300);
}

void moveSelector(float p, float r, float y)
{
  if (!can_move_selector)
    return;
  if (selector_x < padding_x)
    selector_x = M5.Lcd.width() - padding_x;
  if (selector_x + selector_width > M5.Lcd.width() - padding_x)
    selector_x = padding_x;

  if (selector_y < padding_y)
    selector_y = M5.Lcd.height() - padding_y;
  if (selector_y + selector_height > M5.Lcd.height() - padding_y)
    selector_y = padding_y;
  
  if (p > .2f)
  {
    selector_x += jewel_width;
  }
  else if (p < -.2f)
  {
    selector_x -= jewel_width;
  }

  if (r > .2f)
  {
    selector_y -= jewel_height;
  }
  else if (r < -.2f)
  {
    selector_y += jewel_height;
  }
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
  if (selector_width == jewel_width * 2)
  {
    selector_width = jewel_width;
    selector_height = jewel_height * 2;
    selector->jewel2 = pGrid[selector->jewel2->x - 1][selector->jewel2->y + 1];
  }
  else
  {
    selector_width = jewel_width * 2;
    selector_height = jewel_height;
    selector->jewel2 = pGrid[selector->jewel2->x + 1][selector->jewel2->y - 1];
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

void drawSelector(void)
{
  M5.Lcd.fillRect(padding_x + (selector->jewel1->x - 1) * jewel_width, padding_y + selector->jewel1->y * jewel_height, jewel_width, jewel_height, pGrid[selector->jewel1->x - 1][selector->jewel1->y]->color);
  M5.Lcd.fillRect(padding_x + selector->jewel1->x * jewel_width, padding_y + (selector->jewel1->y - 1) * jewel_height, jewel_width, jewel_height, pGrid[selector->jewel1->x][selector->jewel1->y - 1]->color);
  M5.Lcd.drawRect(selector_x, selector_y, selector_width, selector_height, BLACK);
}

void restartGame(void)
{
  moves_left = 12;
  score = 0;
  freeGrid();
  pGrid = create_grid();
}

void selectMenu(void)
{
  int x, y;
  x = y = padding_x;
  if (M5.BtnB.wasPressed())
  {
    if (offset > padding_x * menuSize - 1)
    {
      offset = 0;
    }
    if (selectedItem >= menuSize)
    {
      selectedItem = 0;
    }
    if (selectedItem != 0)
    {
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setCursor(x, y + offset - padding_x);
      M5.Lcd.printf(menu[selectedItem - 1]);
    }
    if (selectedItem == 0)
    {
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setCursor(x, y + 60);
      M5.Lcd.printf(menu[menuSize - 1]);
    }

    M5.Lcd.setTextColor(RED);
    M5.Lcd.setCursor(x, y + offset);
    M5.Lcd.printf(menu[selectedItem]);
    offset += padding_x;
    selectedItem++;
  }
  if (M5.BtnA.wasPressed())
  {
    switch (selectedItem)
    {
    case 1:
      isInMenu = false;
      drawGame();
      break;
    case 4:
      restartGame();
      isInMenu = false;
      break;
    default:
      break;
    }
  }
}

void displayMenu(void)
{
  int x, y;
  x = y = padding_x;
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(x, y);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.printf("BACK");
  M5.Lcd.setCursor(x, y * 2);
  M5.Lcd.printf("SAVE");
  M5.Lcd.setCursor(x, y * 3);
  M5.Lcd.printf("LOAD");
  M5.Lcd.setCursor(x, y * 4);
  M5.Lcd.printf("RESET LEVEL");
}

void drawGrid(Jewel ***grid)
{
  float padding = (M5.Lcd.width() / 2) - (jewel_width * (jewel_amount / 2));
  for (int i = 0; i < jewel_amount; i++)
  {
    for (int j = 0; j < jewel_amount; j++)
    {
      M5.Lcd.fillRect(padding + pGrid[i][j]->x * jewel_width,
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

bool checkRowJewel(Jewel jewel, const unsigned int color)
{
  int right = 1;
  int left = 1;

  while (pGrid[jewel.x + right][jewel.y]->color == color)
  {
    right++;
  }

  if(right >= 3) {
    for (int i = 0; i < right; i++)
    {
      pGrid[jewel.x + i][jewel.y]->color = BLACK;
    }  
  }
  while (pGrid[jewel.x - left][jewel.y]->color == color)
  {
    left++;
  }
  if(left >= 3) {
    for (int i = 0; i < left; i++)
    {
      pGrid[jewel.x - i][jewel.y]->color = BLACK;
    }  
  }

  return (right + left >= 3);
}

bool checkColJewel(Jewel jewel, const unsigned int color)
{
  int top = 1;
  int bottom = 1;

  while (pGrid[jewel.x][jewel.y + top]->color == color)
  {
    top++;
  }

  while (pGrid[jewel.x][jewel.y - bottom]->color == color)
  {
    bottom++;
  }

  return (top + bottom >= 3);
}

void swapJewel(Jewel *jewel1, Jewel *jewel2)
{
  u_int8_t temp = jewel1->color;
  jewel1->color = jewel2->color;
  jewel2->color = temp;
}

bool valid_swap(Selector *s)
{
  return (checkRowJewel(*s->jewel1, s->jewel2->color) ||
          checkColJewel(*s->jewel2, s->jewel1->color)) ||
         (checkRowJewel(*s->jewel2, s->jewel1->color) ||
          checkColJewel(*s->jewel1, s->jewel2->color));
}

unsigned int destroy_jewel(Selector *selector)
{
  unsigned int score = 10;
  int jewel1_x = selector->jewel1->x;
  int jewel1_y = selector->jewel1->y;

  do
  {
    jewel1_x++;
  } while (pGrid[jewel1_x][jewel1_y]->color == selector->jewel2->color);

  do
  {
    jewel1_x--;
  } while (pGrid[jewel1_x][jewel1_y]->color == selector->jewel2->color);
  //  free(pGrid[jewel1_x][jewel1_y]);

  return score;
}

void delete_jewel()
{
  int jewel1_x = selector->jewel1->x;
  int jewel1_y = selector->jewel1->y;
  int jewel2_x = selector->jewel2->x;
  int jewel2_y = selector->jewel2->y;
  do
  {
    jewel1_x++;
    selector->jewel1->color = BLACK;
  } while (pGrid[jewel1_x][jewel1_y]->color == selector->jewel1->color);
  do
  {
    jewel2_x++;
    selector->jewel2->color = BLACK;
  } while (pGrid[jewel2_x][jewel2_y]->color == selector->jewel2->color);



}

void fall_bricks(Jewel* jwl, unsigned int amount, int xdir, int ydir) {
  for (int i = 0; i < amount; i++) {
    for(int j = 0; j < jewel_amount - jwl->y * ydir; j++) {
      pGrid[jwl->x + i * xdir][jwl->y + i * ydir]->color = pGrid[jwl->x + i * xdir + j][jwl->y + i * ydir + j]->color;
    }
  }
}