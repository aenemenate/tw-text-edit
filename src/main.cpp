#include "../include/BearLibTerminal.h"

#include <string>
#include <vector>
#include <algorithm>
#define string       std::string
#define vector       std::vector

// Data types

struct Size {
    int width, height;
};

struct Point {
  int x, y;  
};

enum Direction { Up, Down, Left, Right };

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
  colorPalette->menuBarDropdownHighlightedColor = {"", ""};
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

void handleInputTextBuffer(TextBuffer *buf, int key, Size size) {
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

struct MenuDropdown {
  string name;
  vector<string> options;
  int getDropdownWidth();
};

int MenuDropdown::getDropdownWidth() {
    int biggest_opt = 0;
    for (int i = 0; i < options.size(); ++i) {
      int option_size = options[i].length();
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
  menuBar->options.push_back({"File", {"New   (Ctrl + N)", "Open  (Ctrl + O)", "Save  (Ctrl + S)", "Close (Ctrl + C)"}});
  menuBar->options.push_back({"Edit", {"Undo  (Ctrl + U)", "Redo  (Ctrl + R)", "Cut   (Ctrl + X)", "Copy  (Ctrl + C)", "Paste (Ctrl + V)", "Find  (Ctrl + F)"}});
  menuBar->options.push_back({"Options", {"Themes   (Ctrl + T)", "Settings (Ctrl + P)"/*, "Commands (Ctrl + M)"*/}});
}

void handleInputMenuBar(MenuBar *menuBar, int key, Size term_size) {
  bool mouse_y_on_bar = terminal_state(TK_MOUSE_Y) == 0;
  int mouse_x = terminal_state(TK_MOUSE_X);
  int cur_x = 0;
  if (key == (TK_MOUSE_LEFT|TK_KEY_RELEASED)) {
    for (int i = 0; i < menuBar->options.size(); ++i) {
      if (mouse_y_on_bar && mouse_x >= cur_x && mouse_x < cur_x + menuBar->options[i].name.length()) {
        menuBar->clicked_opt = i;
        return;
      }
      else
        menuBar->clicked_opt = -1;
      cur_x += menuBar->options[i].name.length() + 1;
    }
  }
}

void drawMenuBarDropdown(MenuBar *menuBar, ColorPalette *colorPalette, Size term_size) {
  terminal_color(colorPalette->menuBarDropdownColor.fg.c_str());
  terminal_bkcolor(colorPalette->menuBarDropdownColor.bg.c_str());
  int start_x = 0;
  for (int i = 0; i < menuBar->clicked_opt; ++i) {
    start_x += menuBar->options[i].name.length() + 1;
  }
  for (int j = 0; j < menuBar->options[menuBar->clicked_opt].options.size(); ++j) {
      int x_end = start_x + terminal_print(start_x, j+1, menuBar->options[menuBar->clicked_opt].options[j].c_str()).width;
      for (int i = x_end; i < start_x + menuBar->options[menuBar->clicked_opt].getDropdownWidth(); ++i)
        terminal_put(i, j+1, ' ');
  }
}

void drawMenuBar(MenuBar *menuBar, ColorPalette *colorPalette, Size term_size) {
  terminal_color(colorPalette->menuBarColor.fg.c_str());
  terminal_bkcolor(colorPalette->menuBarColor.bg.c_str());
// bar bg
  for (int i = 0; i < term_size.width; ++i)
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
      drawMenuBarDropdown(menuBar, colorPalette, term_size);
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

void handleInputBufferList(BufferList *bufferList, int key, Size term_size) {
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
  handleInputTextBuffer(currentBuffer, key, {term_size.width, term_size.height - currentBuffer->pos.y});
}

void updateBufferList(BufferList *bufferList, Size term_size) {
// if buffer saved, update it
}

void drawBufferList(BufferList *bufferList, ColorPalette *colorPalette, Size term_size) {
// draw cur buffer from y 2
  TextBuffer *currentBuffer = &(bufferList->textBuffers[bufferList->cur]);
  drawTextBuffer(currentBuffer, colorPalette, {term_size.width, term_size.height - currentBuffer->pos.y});
// draw buffer list at y 1
  terminal_color(colorPalette->bufferListColor.fg.c_str());
  terminal_bkcolor(colorPalette->bufferListColor.bg.c_str());
// bar bg
  for (int i = 0; i < term_size.width; ++i)
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

// GLOBALS

struct EditorData {
  bool running;
  ColorPalette colorPalette;
  BufferList buffers;
  MenuBar menuBar;
};

EditorData editorData;

void buildEditorData(EditorData *editorData) {
  editorData->running = true;
  buildMenuBar(&(editorData->menuBar));
  buildBufferList(&(editorData->buffers));
  buildColorPalette(&(editorData->colorPalette));
}

// PROGRAM

void init() {
  terminal_open();
  terminal_set("window.size=60x40");
  terminal_refresh();
  buildEditorData(&editorData);
}

void handleInput(Size term_size) {
  int key = terminal_read();
  if (key == TK_CLOSE)
    editorData.running = false;
  handleInputMenuBar(&(editorData.menuBar), key, term_size);
  handleInputBufferList(&(editorData.buffers), key, term_size);
}

void update(Size term_size) {
  updateBufferList(&(editorData.buffers), term_size);
}

void draw(Size term_size) {
  terminal_clear();
  // draw buffer first so that menus and things will appear on top of it
  drawBufferList(&(editorData.buffers), &(editorData.colorPalette), term_size);
  drawMenuBar(&(editorData.menuBar), &(editorData.colorPalette), term_size);
  terminal_refresh();
}

int WinMain() {
  init();
  Size term_size = {terminal_state(TK_WIDTH),terminal_state(TK_HEIGHT)};
  draw(term_size);
  while ((editorData.running)) {
    term_size = {terminal_state(TK_WIDTH),terminal_state(TK_HEIGHT)};
    update(term_size);
    draw(term_size);
    handleInput(term_size);
  }
  terminal_close();
}