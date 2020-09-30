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
  int caret_sel_pos;    // caret_sel_pos
  std::string buffer;   // the buffer itself

  void backspace(Size size, bool lineNums);
  void insertChar(char c, Size size, bool lineNums);
  void moveCaret(Direction dir, Size size, bool shiftSel, bool lineNums);
  Point getCaretPos(int caret_pos);
  int findNewline(int n);
};

TextBuffer buildTextBuffer(std::string name, std::string filepath, bool isDirty, Point offs, Point pos, std::string buffer);

bool handleInputTextBuffer(TextBuffer *buf, int key, Size size, bool enterEscapes, bool lineNums); 

void drawTextBuffer(TextBuffer *buf, ColorPalette *colorPalette, Size size, bool lineNums);