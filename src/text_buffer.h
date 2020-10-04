#pragma once

#include "base_types.h"
#include "syntax_highlight.h"
#include <string>

class ColorPalette;

enum ActionType {
  Insert,
  Delete
};

struct Action {
  ActionType type;
  std::string text;
  int caretPos;
  int caretSelPos;
  void undo(TextBuffer*,Size,bool lineNums);
  void redo(TextBuffer*,Size,bool lineNums);
};

struct TextBuffer {
  std::string name;     // file name, including extension
  std::string filePath; // file path, without a '\\' at the end
  bool isDirty;         // is the file unsaved
  Point offs;           // draw offs
  Point pos;            // draw pos
  int cachedXPos;       // x pos to jump to when go to new line
  int caretPos;         // caret_pos (in terms of string index)
  int caretSelPos;      // caret_sel_pos
  std::string buffer;   // the buffer itself
  std::string findText;
  Syntax syntax;
  vector<Action> undos;
  vector<Action> redos;

  void backspace(Size size, bool lineNums, bool undo);
  void insert(std::string str, Size size, bool lineNums, bool undo);
  void paste(Size size, bool lineNums);
  void copy(Size size, bool lineNums);
  void cut(Size size, bool lineNums);
  void findNext(Size size, bool lineNums);
  void findPrev(Size size, bool lineNums);
  void undo(Size size, bool lineNums);
  void redo(Size size, bool lineNums);
  void setOffs(Size size, bool lineNums);
  void moveCaret(Direction dir, Size size, bool shiftSel, bool lineNums);
  void ctrlMoveCaret(Direction dir, Size size, bool shiftSel, bool lineNums);
  Point getCaretPos(int caret_pos);
  int findNewline(int n);
};

TextBuffer buildTextBuffer(std::string name, std::string filepath, bool isDirty, Point offs, Point pos, std::string buffer);

bool handleInputTextBuffer(TextBuffer *buf, int key, Size size, bool enterEscapes, bool lineNums); 

void drawTextBuffer(TextBuffer *buf, Size size, bool lineNums);