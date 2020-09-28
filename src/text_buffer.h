#pragma once

#include <string>
#include "base_types.h"

class ColorPalette;

struct TextBuffer {
  std::string name;
  Point offs;
  Point pos;
  int cached_x_pos;
  int caret_pos;
  std::string buffer;
  void moveCaret(Direction dir, Size size);
  Point getCaretPos();
  int findNewline(int n);
};

bool handleInputTextBuffer(TextBuffer *buf, int key, Size size, bool enter_escapes); 

void drawTextBuffer(TextBuffer *buf, ColorPalette *colorPalette, Size size);