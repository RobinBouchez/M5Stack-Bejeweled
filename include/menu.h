#include <stdint.h>

#define BACK 1
#define SAVE 2
#define LOAD 3
#define RESET 4

#define MENUSIZE 4

bool isInMenu = false;
char* menu[MENUSIZE]{"BACK", "SAVE", "LOAD", "RESET LEVEL"};