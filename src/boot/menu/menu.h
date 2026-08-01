#pragma once
#include <stdbool.h>

// Lmao this shits cursed
typedef struct MenuItem MenuItem; // Forward declare for MenuItemOnPressed
typedef void (*MenuItemOnPressed)(MenuItem* this);
typedef struct MenuItem {
    bool isLastItem;
    MenuItemOnPressed onPressed;
    const char* fmt;
    const char* str; // A single string usable for formatting w/ %s
} MenuItem;

typedef struct Menu {
    MenuItem* items;
    const char* str; // A single string usable for formatting initalSplash with %s
} Menu;

void displayMenu(Menu* men, const char* initialSplash);
