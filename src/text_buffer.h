#pragma once

#include <string>
#include "base_types.h"

class ColorPalette;

struct TextBuffer {
  std::string name;     // file name, including extension
  std::string filepath; // file path, without a '\\' at the end
  bool isDirty;         // is the file unsaved
  Point offs;           // draw offs
  Point pos;            // draw pos
  int cached_x_pos;     // x pos to jump to when go to new line
  int caret_pos;        // caret_pos (in terms of string index)
  std::string buffer;   // the buffer itself

  void moveCaret(Direction dir, Size size, bool lineNums);
  Point getCaretPos();
  int findNewline(int n);
};

bool handleInputTextBuffer(TextBuffer *buf, int key, Size size, bool enterEscapes, bool lineNums); 

void drawTextBuffer(TextBuffer *buf, ColorPalette *colorPalette, Size size, bool lineNums);