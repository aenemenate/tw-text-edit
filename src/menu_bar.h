#pragma once

#include <string>
#include <vector>
#include "base_types.h"

class EditorData;

struct MenuFunction {
  std::string name;
  void (*callback_f)(EditorData*);
};

struct MenuDropdown {
  std::string name;
  std::vector<MenuFunction> options;
  int getDropdownWidth();
};

struct MenuBar {
  int clicked_opt;
  std::vector<MenuDropdown> options;
};

void buildMenuBar(MenuBar *menuBar);

void drawMenuBar(MenuBar *menuBar, Size termSize);

bool clickedMenuBarDropdown(EditorData* editorData, Size termSize);

bool handleInputMenuBar(EditorData *editorData, int key, Size termSize);
