#pragma once

#include "base_types.h"
#include "code_bar.h"
#include "syntax_highlight.h"
#include <string>

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
  CodeBar codeBar;
  std::vector<Action> undos;
  std::vector<Action> redos;

  void backspace(Size bufferSize, bool lineNums, bool undo);
  void insert(std::string str, Size bufferSize, bool lineNums, bool undo);
  void paste(Size bufferSize, bool lineNums);
  void copy(Size bufferSize, bool lineNums);
  void cut(Size bufferSize, bool lineNums);
  void findNext(Size bufferSize, bool lineNums);
  void findPrev(Size bufferSize, bool lineNums);
  void undo(Size bufferSize, bool lineNums);
  void redo(Size bufferSize, bool lineNums);
  void setOffs(Size bufferSize, bool lineNums);
  void moveCaret(Direction dir, Size bufferSize, bool shiftSel, bool lineNums);
  void ctrlMoveCaret(Direction dir, Size bufferSize, bool shiftSel, bool lineNums);
  Point getCaretPos(int caret_pos, bool skipFoldedBlocks = false);
  int findNewline(int n, bool skipFoldedBlocks = false, bool returnLineNumber = false);
};

TextBuffer buildTextBuffer(std::string name, std::string filepath, bool isDirty, Point offs, Point pos, std::string buffer);

bool handleInputTextBuffer(TextBuffer *buf, int key, Size bufferSize, bool enterEscapes, bool lineNums); 

void drawTextBuffer(TextBuffer *buf, Size bufferSize, bool lineNums);
