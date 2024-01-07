#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <stdbool.h>

#define BACK 1
#define SAVE 2
#define LOAD 3
#define RESET 4
#define GAME 5

#define MENUSIZE 5

bool isInMenu = false;
char* menu[MENUSIZE] = {"BACK", "SAVE", "LOAD", "RESET LEVEL", "CHANGE GAME MODE"};

#endif //MENU_H