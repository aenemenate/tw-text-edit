#include "menu_bar.h"
#include "editor_actions.h"
#include "../include/BearLibTerminal.h"
#include "editor_data.h"

int MenuDropdown::getDropdownWidth() {
    int biggest_opt = 0;
    for (int i = 0; i < options.size(); ++i) {
      int option_size = options[i].name.length();
      biggest_opt = option_size > biggest_opt ? option_size : biggest_opt;
    }
    return biggest_opt;
}

void buildMenuBar(MenuBar *menuBar) {
  menuBar->clicked_opt = -1;
  menuBar->options.push_back({"File", {{"New   (Ctrl + N)", NewFile}, {"Open  (Ctrl + O)", GetOpenFile}, {"Save  (Ctrl + S)", nullptr}, {"Close (Ctrl + C)", CloseFile}}});
  menuBar->options.push_back({"Edit", {{"Undo  (Ctrl + U)", nullptr}, {"Redo  (Ctrl + R)", nullptr}, {"Cut   (Ctrl + X)", nullptr}, {"Copy  (Ctrl + C)", nullptr}, {"Paste (Ctrl + V)", nullptr}, {"Find  (Ctrl + F)", nullptr}}});
  menuBar->options.push_back({"Options", {{"Settings (Ctrl + P)", nullptr}/*, "Commands (Ctrl + M)"*/}});
}

void drawMenuBarDropdown(MenuBar *menuBar, ColorPalette *colorPalette, Size termSize) {
  terminal_color(colorPalette->menuBarDropdownColor.fg.c_str());
  terminal_bkcolor(colorPalette->menuBarDropdownColor.bg.c_str());
  int start_x = 0;
  for (int i = 0; i < menuBar->clicked_opt; ++i) {
    start_x += menuBar->options[i].name.length() + 1;
  }
  for (int j = 0; j < menuBar->options[menuBar->clicked_opt].options.size(); ++j) {
    if (terminal_state(TK_MOUSE_Y) == 1 + j && terminal_state(TK_MOUSE_X) >= start_x 
    && terminal_state(TK_MOUSE_X) < start_x + menuBar->options[menuBar->clicked_opt].getDropdownWidth()) {
      terminal_color(colorPalette->menuBarDropdownHighlightedColor.fg.c_str());
      terminal_bkcolor(colorPalette->menuBarDropdownHighlightedColor.bg.c_str());
    }
    else {
      terminal_color(colorPalette->menuBarDropdownColor.fg.c_str());
      terminal_bkcolor(colorPalette->menuBarDropdownColor.bg.c_str());
    }
    int x_end = start_x + terminal_print(start_x, j+1, menuBar->options[menuBar->clicked_opt].options[j].name.c_str()).width;
    for (int i = x_end; i < start_x + menuBar->options[menuBar->clicked_opt].getDropdownWidth(); ++i)
      terminal_put(i, j+1, ' ');
  }
}

void drawMenuBar(MenuBar *menuBar, ColorPalette *colorPalette, Size termSize) {
  terminal_color(colorPalette->menuBarColor.fg.c_str());
  terminal_bkcolor(colorPalette->menuBarColor.bg.c_str());
// bar bg
  for (int i = 0; i < termSize.width; ++i)
    terminal_put(i, 0, ' ');
// options
  int cur_x = 0;
  bool mouse_y_on_bar = terminal_state(TK_MOUSE_Y) == 0;
  int mouse_x = terminal_state(TK_MOUSE_X);
  for (int i = 0; i < menuBar->options.size(); ++i) {
    if (mouse_y_on_bar && mouse_x >= cur_x && mouse_x < cur_x + menuBar->options[i].name.length()
    ||  menuBar->clicked_opt == i) {
      terminal_bkcolor(colorPalette->menuBarHighlightedColor.bg.c_str());
      terminal_color(colorPalette->menuBarHighlightedColor.fg.c_str());
    }
    cur_x += terminal_print(cur_x, 0, menuBar->options[i].name.c_str()).width + 1;
    terminal_color(colorPalette->menuBarColor.fg.c_str());
    terminal_bkcolor(colorPalette->menuBarColor.bg.c_str());
  }
// dropdown
  if (menuBar->clicked_opt != -1) {
      drawMenuBarDropdown(menuBar, colorPalette, termSize);
  }
  // set colors back to defaults
  terminal_color(colorPalette->workspaceDefaultColor.fg.c_str());
  terminal_bkcolor(colorPalette->workspaceDefaultColor.bg.c_str());
}

bool clickedMenuBarDropdown(EditorData* editorData, Size termSize) {
  bool return_value = false;
  int start_x = 0;
  for (int i = 0; i < editorData->menuBar.clicked_opt; ++i) {
    start_x += editorData->menuBar.options[i].name.length() + 1;
  }
  MenuDropdown *dropdown = &(editorData->menuBar.options[editorData->menuBar.clicked_opt]);
  for (int j = 0; j < dropdown->options.size(); ++j) {
    if (terminal_state(TK_MOUSE_Y) == 1 + j && terminal_state(TK_MOUSE_X) >= start_x 
    && terminal_state(TK_MOUSE_X) < start_x + dropdown->getDropdownWidth()
    && dropdown->options[j].callback_f != nullptr) {
      dropdown->options[j].callback_f(editorData);
      return_value = true;
    }
  }
  return return_value;
}

bool handleInputMenuBar(EditorData *editorData, int key, Size termSize) {
  bool return_value = false;
  bool mouse_y_on_bar = terminal_state(TK_MOUSE_Y) == 0;
  int mouse_x = terminal_state(TK_MOUSE_X);
  int cur_x = 0;
  if (key == (TK_MOUSE_LEFT|TK_KEY_RELEASED)) {
    if (editorData->menuBar.clicked_opt != -1) {
      return_value = clickedMenuBarDropdown(editorData, termSize);
    }
    for (int i = 0; i < editorData->menuBar.options.size(); ++i) {
      if (mouse_y_on_bar && mouse_x >= cur_x && mouse_x < cur_x + editorData->menuBar.options[i].name.length()) {
        editorData->menuBar.clicked_opt = i;
        return true;
      }
      else
        editorData->menuBar.clicked_opt = -1;
      cur_x += editorData->menuBar.options[i].name.length() + 1;
    }
  }
  return return_value;
}
