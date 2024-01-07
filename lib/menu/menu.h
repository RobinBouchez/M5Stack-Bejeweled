#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <stdbool.h>

#define BACK 1
#define SAVE 2
#define LOAD 3
#define RESET 0

#define MENUSIZE 4

#define MENU_X 20
#define MENU_Y 20

#define GAME_MODE_ARR_SIZE 2

#define MENU_TIME_OUT 10

bool isInMenu = false;

char* menu[MENUSIZE] = {"BACK", "SAVE", "LOAD", "RESET LEVEL"};
char* game_modes[GAME_MODE_ARR_SIZE] = {"EASY MODE", "CLASSIC MODE"};

#endif //MENU_H