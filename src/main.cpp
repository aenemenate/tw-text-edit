#include "../include/BearLibTerminal.h"

#include <string>
#include <vector>
#include <algorithm>
#define string       std::string
#define vector       std::vector

struct EditorData;

// Data types

struct Size {
    int width, height;
};

struct Point {
  int x, y;  
};

enum Direction { Up, Down, Left, Right };

// ACTIONS

void NewFile(EditorData*);
void GetOpenFile(EditorData*);
void OpenFile(string name, EditorData*);
void CloseFile(EditorData*);

// COLOR PALETTE

struct Color {
  string fg, bg;
};

struct ColorPalette {
  Color menuBarColor;
  Color menuBarHighlightedColor;
  Color menuBarDropdownColor;
  Color menuBarDropdownHighlightedColor;
  Color bufferListColor;
  Color bufferListHighlightedColor;
  Color bufferListActiveColor;
  Color workspaceDefaultColor;
};

void buildColorPalette(ColorPalette *colorPalette) {
  colorPalette->menuBarColor                    = {"darker 0,43,54", "dark 101,123,131"};
  colorPalette->menuBarHighlightedColor         = {"darker 0,43,54", "101,123,131"};
  colorPalette->menuBarDropdownColor            = {"darker 0,43,54", "101,123,131"};
  colorPalette->menuBarDropdownHighlightedColor = {"147,161,161", "101,123,131"};
  colorPalette->bufferListColor                 = {"darker 0,43,54", "dark 101,123,131"};
  colorPalette->bufferListHighlightedColor      = {"darker 0,43,54", "light 101,123,131"};
  colorPalette->bufferListActiveColor           = {"147,161,161", "0,43,54"};
  colorPalette->workspaceDefaultColor           = {"253,246,227", "0,43,54"};
}

// TEXT BUFFER

struct TextBuffer {
  string name;
  Point offs;
  Point pos;
  int cached_x_pos;
  int caret_pos;
  string buffer;
  void moveCaret(Direction dir, Size size);
  Point getCaretPos();
  int findNewline(int n);
};

void TextBuffer::moveCaret(Direction dir, Size size) {
  Point temp_pos;
  Point caret_pos_check;
  int caret_pos_temp;
  switch (dir) {
    case Direction::Left: 
      caret_pos -= 1; 
      if (caret_pos < 0)
        caret_pos = 0;
      cached_x_pos = getCaretPos().x;
      break;
    case Direction::Right: 
      caret_pos += 1;
      if (caret_pos > buffer.length())
        caret_pos = buffer.length();
      cached_x_pos = getCaretPos().x;
      break;
    case Direction::Up:
      temp_pos = getCaretPos();
      caret_pos_temp = findNewline(std::max(0, temp_pos.y-1));
      caret_pos = caret_pos_temp + cached_x_pos;
      caret_pos_check = getCaretPos();
      if (caret_pos_check.y != std::max(0, temp_pos.y-1))
        caret_pos = findNewline(std::max(0, temp_pos.y))-1;
      break;
    case Direction::Down:
      temp_pos = getCaretPos();
      caret_pos_temp = findNewline(temp_pos.y+1);
      caret_pos = caret_pos_temp + cached_x_pos;
      caret_pos_check = getCaretPos();
      if (caret_pos != buffer.length() && caret_pos_check.y != std::max(0, temp_pos.y+1)) {
        caret_pos = findNewline(std::max(0, temp_pos.y+2))-1;
      }
      if (caret_pos <= 0) {
        caret_pos = buffer.length();
      }
      break;
  }
  Point temp = getCaretPos();
  if (temp.x < std::abs(this->offs.x))
    this->offs.x = -temp.x;
  if (temp.x >= std::abs(this->offs.x) + size.width)
    this->offs.x = - (temp.x - size.width)-1;
  if (temp.y < std::abs(this->offs.y))
    this->offs.y = -temp.y;
  if (temp.y >= std::abs(this->offs.y) + size.height)
    this->offs.y = - (temp.y - size.height)-1;
}

Point TextBuffer::getCaretPos() {
    Point pos = {0,0};
    for (int i = 0; i < buffer.length() + 1; ++i) {
        if (caret_pos == i)
          return pos;
        pos.x += 1;
        if (i < buffer.length() && buffer[i] == '\n') {
            pos.y += 1;
            pos.x = 0;
        }
    }
  return {0,0};
}

int TextBuffer::findNewline(int n) {
  int c = 0;
  if (n == 0) return 0;
  for (int p = 0; p < buffer.length(); ++p) {
      if (buffer[p] == '\n') c++;
      if (c == n) return p+1;
  }
  return 0;
}

bool handleInputTextBuffer(TextBuffer *buf, int key, Size size, bool enter_escapes) {
  switch (key) {
      case (TK_RIGHT):
        buf->moveCaret(Direction::Right, size);
        break;
      case (TK_LEFT):
        buf->moveCaret(Direction::Left, size);
        break;
      case (TK_DOWN):
        buf->moveCaret(Direction::Down, size);
        break;
      case (TK_UP):
        buf->moveCaret(Direction::Up, size);
        break;
  }
  if (key == TK_RETURN || key == TK_ENTER || key == TK_KP_ENTER) {
    if (enter_escapes) return true;
// No distinction between confirmation and interruption
    string first_half = buf->buffer.substr(0, buf->caret_pos);
    string second_half = buf->buffer.substr(buf->caret_pos, buf->buffer.length()+1 - buf->caret_pos);
    buf->buffer = first_half + "\n" + second_half;
    buf->moveCaret(Direction::Right, size);
    buf->cached_x_pos = buf->getCaretPos().x;
  }
  else if (key == TK_BACKSPACE && buf->buffer.length() > 0) {
// Remove one character
    string first_half = buf->buffer.substr(0, std::max(0, buf->caret_pos-1));
    string second_half = buf->buffer.substr(buf->caret_pos, buf->buffer.length()+1 - buf->caret_pos);
    buf->buffer = first_half + second_half;
    buf->moveCaret(Direction::Left, size);
    buf->cached_x_pos = buf->getCaretPos().x;
  }
  else if (terminal_check(TK_WCHAR)) {
// Append one character
    string first_half = buf->buffer.substr(0, buf->caret_pos);
    string second_half = buf->buffer.substr(buf->caret_pos, buf->buffer.length()+1 - buf->caret_pos);
    buf->buffer = first_half + (char)terminal_state(TK_WCHAR) + second_half;
    buf->moveCaret(Direction::Right, size);
    buf->cached_x_pos = buf->getCaretPos().x;
  }
  return false;
}

void drawTextBuffer(TextBuffer *buf, ColorPalette *colorPalette, Size size) {
  terminal_color(colorPalette->workspaceDefaultColor.fg.c_str());
  terminal_bkcolor(colorPalette->workspaceDefaultColor.bg.c_str());
  for (int i = buf->pos.x; i < buf->pos.x + size.width; ++i)
    for (int j = buf->pos.y; j < buf->pos.y + size.height; ++j) {
      terminal_put(i, j, ' ');
    }
  terminal_print(buf->pos.x + buf->offs.x, buf->pos.y + buf->offs.y, buf->buffer.c_str());

  terminal_color(colorPalette->workspaceDefaultColor.bg.c_str());
  terminal_bkcolor(colorPalette->workspaceDefaultColor.fg.c_str());
  Point caret_pos = buf->getCaretPos();
  char prev_char = terminal_pick(buf->pos.x + caret_pos.x + buf->offs.x, buf->pos.y + caret_pos.y + buf->offs.y);
  terminal_put(buf->pos.x + caret_pos.x + buf->offs.x, buf->pos.y + caret_pos.y + buf->offs.y, prev_char);
}

// MENU BAR

struct MenuFunction {
  string name;
  void (*callback_f)(EditorData*);
};

struct MenuDropdown {
  string name;
  vector<MenuFunction> options;
  int getDropdownWidth();
};

int MenuDropdown::getDropdownWidth() {
    int biggest_opt = 0;
    for (int i = 0; i < options.size(); ++i) {
      int option_size = options[i].name.length();
      biggest_opt = option_size > biggest_opt ? option_size : biggest_opt;
    }
    return biggest_opt;
}

struct MenuBar {
  int clicked_opt;
  vector<MenuDropdown> options;
};

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

// BUFFER LIST

struct BufferList {
  int cur;
  vector<TextBuffer> textBuffers;
};

void buildBufferList(BufferList *bufferList) {
  bufferList->cur = 0;
  bufferList->textBuffers.push_back({"New 1", {0, 0}, {0, 2}, 0, 0, ""});
}

void handleInputBufferList(BufferList *bufferList, int key, Size termSize) {
// if buffer clicked, switch to it
  bool mouse_y_on_bar = terminal_state(TK_MOUSE_Y) == 1;
  int mouse_x = terminal_state(TK_MOUSE_X);
  int cur_x = 0;
  if (key == (TK_MOUSE_LEFT|TK_KEY_RELEASED)) {
    for (int i = 0; i < bufferList->textBuffers.size(); ++i) {
      if (mouse_y_on_bar && mouse_x >= cur_x && mouse_x < cur_x + bufferList->textBuffers[i].name.length()) {
        bufferList->cur = i;
        return;
      }
      cur_x += bufferList->textBuffers[i].name.length() + 1;
    }
  }
// if buffer clicked, go to it
  TextBuffer *currentBuffer = &(bufferList->textBuffers[bufferList->cur]);
  handleInputTextBuffer(currentBuffer, key, {termSize.width, termSize.height - currentBuffer->pos.y}, false);
}

void drawBufferList(BufferList *bufferList, ColorPalette *colorPalette, Size termSize) {
// draw cur buffer from y 2
  TextBuffer *currentBuffer = &(bufferList->textBuffers[bufferList->cur]);
  drawTextBuffer(currentBuffer, colorPalette, {termSize.width, termSize.height - currentBuffer->pos.y});
// draw buffer list at y 1
  terminal_color(colorPalette->bufferListColor.fg.c_str());
  terminal_bkcolor(colorPalette->bufferListColor.bg.c_str());
// bar bg
  for (int i = 0; i < termSize.width; ++i)
    terminal_put(i, 1, ' ');
// options
  int cur_x = 0;
  bool mouse_y_on_bar = terminal_state(TK_MOUSE_Y) == 1;
  int mouse_x = terminal_state(TK_MOUSE_X);
  for (int i = 0; i < bufferList->textBuffers.size(); ++i) {
    if (mouse_y_on_bar && mouse_x >= cur_x && mouse_x < cur_x + bufferList->textBuffers[i].name.length()) {
      terminal_bkcolor(colorPalette->bufferListHighlightedColor.bg.c_str());
      terminal_color(colorPalette->bufferListHighlightedColor.fg.c_str());
    }
    if (bufferList->cur == i) {
      terminal_bkcolor(colorPalette->bufferListActiveColor.bg.c_str());
      terminal_color(colorPalette->bufferListActiveColor.fg.c_str());
    }
    cur_x += terminal_print(cur_x, 1, bufferList->textBuffers[i].name.c_str()).width + 1;
    terminal_color(colorPalette->bufferListColor.fg.c_str());
    terminal_bkcolor(colorPalette->bufferListColor.bg.c_str());
  }
}

// Text Input Dropdown

enum TextAction {
  Open,
  Save,
  Find,
  FindAndReplace
};

struct TextDropdown {
  TextBuffer inputBuffer;
  TextAction action;
  bool showing;
};

void buildTextDropdown(TextDropdown *textDropdown, Size termSize) {
  textDropdown->inputBuffer = {"", {0, 0}, {std::max(0, termSize.width - 20), 1}, 0, 0, ""};
  textDropdown->showing = false;
}

void drawTextDropdown(TextDropdown *textDropdown, ColorPalette *colorPalette, Size termSize) {
  if (textDropdown->showing) {
    drawTextBuffer(&(textDropdown->inputBuffer), colorPalette, {20, 1});
  }
}

// GLOBALS

int next_file = 2;

struct EditorData {
  bool running;
  ColorPalette colorPalette;
  BufferList buffers;
  MenuBar menuBar;
  TextDropdown textDropdown;
};

EditorData editorData;

void buildEditorData(EditorData *editorData) {
  Size termSize = {terminal_state(TK_WIDTH),terminal_state(TK_HEIGHT)};
  editorData->running = true;
  buildMenuBar(&(editorData->menuBar));
  buildBufferList(&(editorData->buffers));
  buildColorPalette(&(editorData->colorPalette));
  buildTextDropdown(&(editorData->textDropdown), termSize);
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

bool handleInputTextDropdown(EditorData *editorData, int key, Size termSize) {
  if (editorData->textDropdown.showing) {
    bool finished = handleInputTextBuffer(&(editorData->textDropdown.inputBuffer), key, {20, 1}, true);
    if (finished) {
      editorData->textDropdown.showing = false;
      switch (editorData->textDropdown.action) {
        case TextAction::Open:
          // open file entered by user
          OpenFile(editorData->textDropdown.inputBuffer.buffer, editorData);
          break;
        case TextAction::Save:
          // save file as entered by user
          break;
        case TextAction::Find:
          // highlight found text, then enter mode where user can press left and right to jump to instances
          break;
        case TextAction::FindAndReplace:
          // prompt for replace text
          break;
      }
      return true;
    }
    // print any extra necessary text
  }
  return editorData->textDropdown.showing;
}

// ACTIONS

void NewFile(EditorData *editorData) {
  editorData->buffers.textBuffers.push_back({"New " + std::to_string(next_file), {0, 0}, {0, 2}, 0, 0, ""});
  ++next_file;
  editorData->buffers.cur = editorData->buffers.textBuffers.size() - 1;
}

void GetOpenFile(EditorData *editorData) {
  editorData->textDropdown.showing = true;
  editorData->textDropdown.action = TextAction::Open;
  editorData->textDropdown.inputBuffer.buffer = "";
  editorData->textDropdown.inputBuffer.caret_pos = 0;
  editorData->textDropdown.inputBuffer.cached_x_pos = 0;
  editorData->textDropdown.inputBuffer.offs = {0, 0};
}

void OpenFile(string name, EditorData *editorData) {
  // open file from file location and put it in a new buffer
}

void CloseFile(EditorData *editorData) {
  editorData->buffers.textBuffers.erase(editorData->buffers.textBuffers.begin() + editorData->buffers.cur, 
                                        editorData->buffers.textBuffers.begin() + editorData->buffers.cur + 1);
  ++editorData->buffers.cur;
  if (editorData->buffers.cur >= editorData->buffers.textBuffers.size()) {
    editorData->buffers.cur = editorData->buffers.textBuffers.size() - 1;
  }
  if (editorData->buffers.cur < 0) {
    editorData->buffers.cur = 0;
  }
}

// PROGRAM

void init() {
  terminal_open();
  terminal_set("window.size=60x40");
  terminal_refresh();
  buildEditorData(&editorData);
}

void handleInput(Size termSize) {
  int key = terminal_read();
  if (key == TK_CLOSE)
    editorData.running = false;
  if (!handleInputMenuBar(&editorData, key, termSize) && !handleInputTextDropdown(&editorData, key, termSize))
    handleInputBufferList(&(editorData.buffers), key, termSize);
}

void update(Size termSize) {
}

void draw(Size termSize) {
  terminal_clear();
  // draw buffer first so that menus and things will appear on top of it
  drawBufferList(&(editorData.buffers), &(editorData.colorPalette), termSize);
  drawMenuBar(&(editorData.menuBar), &(editorData.colorPalette), termSize);
  drawTextDropdown(&(editorData.textDropdown), &(editorData.colorPalette), termSize);
  terminal_refresh();
}

int WinMain() {
  init();
  Size termSize = {terminal_state(TK_WIDTH),terminal_state(TK_HEIGHT)};
  draw(termSize);
  while ((editorData.running)) {
    termSize = {terminal_state(TK_WIDTH),terminal_state(TK_HEIGHT)};
    update(termSize);
    draw(termSize);
    handleInput(termSize);
  }
  terminal_close();
}