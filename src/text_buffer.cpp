#include "text_buffer.h"
#include <BearLibTerminal.h>
#include "clipboard.h"
#include <ctype.h>
#include <vector>
#include <algorithm>
#include <iostream>

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif


void Action::undo(TextBuffer *buffer, Size bufferSize, bool lineNums) {
  switch (this->type) {
    case (ActionType::Insert):
      buffer->caretPos = this->caretPos;
      buffer->caretSelPos = this->caretSelPos + this->text.length();
      buffer->backspace(bufferSize, lineNums, false);
      break;
    case (ActionType::Delete):
      if (buffer->caretPos != buffer->caretSelPos)
           buffer->caretPos = min(this->caretPos, this->caretSelPos);
      else buffer->caretPos = this->caretPos;
      buffer->caretSelPos = buffer->caretPos;
      buffer->insert(this->text, bufferSize, lineNums, false);
      break;
  }
}

void Action::redo(TextBuffer *buffer, Size bufferSize, bool lineNums) {
  buffer->caretPos = this->caretPos;
  buffer->caretSelPos = this->caretSelPos;
  switch (this->type) {
    case (ActionType::Insert):
      buffer->insert(this->text, bufferSize, lineNums, false);
      break;
    case (ActionType::Delete):
      buffer->backspace(bufferSize, lineNums, false);
      break;
  }
}

void caretSeek(int amount, TextBuffer *textBuffer) {
  int caretPos = textBuffer->caretPos;
  for (int i = min(caretPos, caretPos+amount); 
       i < max(caretPos, caretPos + amount); 
       ++textBuffer->caretPos) {
    if (textBuffer->buffer[textBuffer->caretPos] == '\t') {
      i += 8 - textBuffer->getCaretPos(textBuffer->caretPos).x % 8;
    }
    else ++i;
  }
}

void ctrlright(TextBuffer *buf, bool shiftSelect) {
  if (buf->caretPos < buf->buffer.length()) {
    std::vector<int> places;
    std::vector<char> keychars = {
      ' ', '\\', '/', '<', '>', '.', ',', '{', '}', '(', ')', '\"', ';', '=', '-', '+', 
      '|', '&', '*', '!', '%', '[', ']', ':', '\n', '\t'
    };
    for (char c : keychars)
      for (int i = buf->caretPos+1; i <= buf->buffer.length(); ++i)
        if (i == buf->buffer.length() || isalpha(buf->buffer[i-1]) && std::count(keychars.begin(), keychars.end(), buf->buffer[i])) {
          places.push_back(i);
          break;
        }
    int orig_pos = buf->caretPos;
    for (int i : places)
      if (i < buf->caretPos || buf->caretPos == orig_pos)
        buf->caretPos = i;
    if (!shiftSelect)
      buf->caretSelPos = buf->caretPos;
  }
}

void ctrlleft(TextBuffer *buf, bool shiftSelect) {
  if (buf->caretPos > 0) {
    std::vector<int> places;
    std::vector<char> keychars = {
      ' ', '\\', '/', '<', '>', '.', ',', '{', '}', '(', ')', '\"', ';', '=', '-', '+', 
      '|', '&', '*', '!', '%', '[', ']', ':', '\n', '\t'
    };
    for (char c : keychars)
      for (int i = buf->caretPos-1; i >= 0; --i)
        if (i == 0 || isalpha(buf->buffer[i]) && std::count(keychars.begin(), keychars.end(), buf->buffer[i-1])) {
          places.push_back(i);
          break;
        }
    int orig_pos = buf->caretPos;
    for (int i : places)
      if (i > buf->caretPos || buf->caretPos == orig_pos)
        buf->caretPos = i;
    if (!shiftSelect) {
      buf->caretSelPos = buf->caretPos;
    }
  }
}

void ctrlup(TextBuffer *buf, bool shiftSelect) {
  int i = -1;
  while (buf->caretPos + i >= 0) {
    if (buf->buffer.substr(buf->caretPos + i, 2) == "\n\n"
    &&  buf->caretPos + i + 1 != buf->caretPos)
      break;
    --i;
  }
  if (buf->caretPos + i < 0)
    buf->caretPos = 0;
  else
    buf->caretPos += i + 1;
  if (!shiftSelect)
    buf->caretSelPos = buf->caretPos;
}

void ctrldown(TextBuffer *buf, bool shiftSelect) {
  int i = 1;
  while (buf->caretPos + i <= buf->buffer.length()) {
    if (buf->buffer.substr(buf->caretPos + i, 2) == "\n\n"
    &&  buf->caretPos + i + 1 != buf->caretPos)
      break;
    ++i;
  }
  if (buf->caretPos + i >= buf->buffer.length()) 
       buf->caretPos = buf->buffer.length();
  else buf->caretPos += i + 1;
  if (!shiftSelect)
    buf->caretSelPos = buf->caretPos;
}

void TextBuffer::backspace(Size bufferSize, bool lineNums, bool undo) {
  int len = caretPos == caretSelPos ? 1 : max(caretPos, caretSelPos) - min(caretPos, caretSelPos);
  if (undo) {
    undos.push_back({ 
      ActionType::Delete, 
      buffer.substr(min(caretPos, caretSelPos), len),
      caretPos,
      caretSelPos
    });
    redos.clear();
  }
  std::string first_half, second_half;
  if (caretPos == caretSelPos) {
    first_half = buffer.substr(0, max(0, caretPos-1));
    second_half = buffer.substr(caretPos, buffer.length() + 1 - caretPos);
    moveCaret(Direction::Left, bufferSize, false, lineNums);
  }
  else if (caretPos > caretSelPos) {
    first_half = buffer.substr(0, max(0, caretSelPos));
    second_half = buffer.substr(caretPos, buffer.length() + 1 - caretPos);
    caretPos = caretSelPos;
  }
  else {
    first_half = buffer.substr(0, max(0, caretPos));
    second_half = buffer.substr(caretSelPos, buffer.length() + 1 - caretSelPos);
  }
  buffer = first_half + second_half;
  cachedXPos = getCaretPos(caretPos).x;
  isDirty = true;
  caretSelPos = caretPos;
}

void TextBuffer::insert(std::string str, Size bufferSize, bool lineNums, bool undo) {
  if (caretPos != caretSelPos)
    backspace(bufferSize, lineNums, true);
  if (undo) {
    undos.push_back({ 
      ActionType::Insert, 
      str,
      caretPos,
      caretSelPos
    });
    redos.clear();
  }
  std::string first_half = buffer.substr(0, caretPos);
  std::string second_half = buffer.substr(caretPos, buffer.length() + 1 - caretPos);
  buffer = first_half + str + second_half;
  caretPos += str.length() - 1;
  moveCaret(Direction::Right, bufferSize, false, lineNums);
  cachedXPos = getCaretPos(caretPos).x;
  isDirty = true;
}

void TextBuffer::paste(Size bufferSize, bool lineNums) {
  if (caretPos != caretSelPos)
    backspace(bufferSize, lineNums, true);
  std::string clipboard = GetClipboardContents();
  insert(clipboard, bufferSize, lineNums, true);
}

void TextBuffer::copy(Size bufferSize, bool lineNums) {
  if (caretPos != caretSelPos) {
    std::string text_to_copy = buffer.substr(min(caretPos,caretSelPos), max(caretPos,caretSelPos) - min(caretPos,caretSelPos));
    PushTextToClipboard(text_to_copy);
  }
}

void TextBuffer::cut(Size bufferSize, bool lineNums) {
  if (caretPos != caretSelPos) {
    std::string text_to_copy = buffer.substr(min(caretPos,caretSelPos), max(caretPos,caretSelPos) - min(caretPos,caretSelPos));
    PushTextToClipboard(text_to_copy);
    backspace(bufferSize, lineNums, true);
  }
}

void TextBuffer::findNext(Size bufferSize, bool lineNums) {
  if (findText == "")
    return;
  int i = 1;
  bool found = false;
  int old_caret_pos = caretPos;
  while (caretPos + i < buffer.length()) {
    if (buffer.substr(caretPos + i, findText.length()) == findText
    &&  caretPos + i != caretPos) {
      found = true;
      break;
    }
    ++i;
  }
  if (caretPos + i < buffer.length())
    caretPos += i;
  caretSelPos = caretPos;
  if (found || caretPos == old_caret_pos && caretSelPos + findText.length() <= buffer.length())
    caretSelPos += findText.length();
  setOffs(bufferSize, lineNums);
}

void TextBuffer::findPrev(Size bufferSize, bool lineNums) {
  if (findText == "")
    return;
  int i = -1;
  while (caretPos + i >= 0) {
    if (buffer.substr(caretPos + i, findText.length()) == findText
    &&  caretPos + i != caretPos)
      break;
    --i;
  }
  if (caretPos + i >= 0)
    caretPos += i;
  caretSelPos = caretPos;
  caretSelPos += findText.length();
  setOffs(bufferSize, lineNums);
}

void TextBuffer::undo(Size bufferSize, bool lineNums) {
  if (undos.size() > 0) {
    undos.back().undo(this, bufferSize, lineNums);
    redos.push_back(undos.back());
    undos.pop_back();
  }
}

void TextBuffer::redo(Size bufferSize, bool lineNums) {
  if (redos.size() > 0) {
    redos.back().redo(this, bufferSize, lineNums);
    undos.push_back(redos.back());
    redos.pop_back();
  }
}

void TextBuffer::setOffs(Size bufferSize, bool lineNums) {
  int padding = ((this->codeBar.GetShowing() ? 1 : 0) + (lineNums ? 4 : 0));
  Point temp = getCaretPos(caretPos, true);
  Point temp2 = getCaretPos(caretSelPos, true);
  if (temp2.x < std::abs(this->offs.x))
    this->offs.x = -temp2.x;
  if (temp2.x >= std::abs(this->offs.x) + bufferSize.width - padding)
    this->offs.x = - (temp2.x - bufferSize.width + padding)-1;
  if (temp2.y < std::abs(this->offs.y))
    this->offs.y = -temp2.y;
  if (temp2.y >= std::abs(this->offs.y) + bufferSize.height)
    this->offs.y = - (temp2.y - bufferSize.height)-1;
  if (temp.x < std::abs(this->offs.x))
    this->offs.x = -temp.x;
  if (temp.x >= std::abs(this->offs.x) + bufferSize.width - padding)
    this->offs.x = - (temp.x - bufferSize.width + padding)-1;
  if (temp.y < std::abs(this->offs.y))
    this->offs.y = -temp.y;
  if (temp.y >= std::abs(this->offs.y) + bufferSize.height)
    this->offs.y = - (temp.y - bufferSize.height)-1;
}

void TextBuffer::moveCaret(Direction dir, Size bufferSize, bool shiftSelect, bool lineNums) {
  Point temp_pos;
  Point caret_pos_check;
  int caret_pos_temp;
  switch (dir) {
    case Direction::Left: 
      caretPos -= 1; 
      if (caretPos < 0)
        caretPos = 0;
      cachedXPos = getCaretPos(caretPos).x;
      break;
    case Direction::Right: 
      caretPos += 1;
      if (caretPos > buffer.length())
        caretPos = buffer.length();
      cachedXPos = getCaretPos(caretPos).x;
      break;
    case Direction::Up:
      temp_pos = getCaretPos(caretPos);
      caret_pos_temp = findNewline(max(0, temp_pos.y-1));
      caretPos = caret_pos_temp;
      caretSeek(cachedXPos, this);
      caret_pos_check = getCaretPos(caretPos);
      if (caret_pos_check.y != max(0, temp_pos.y-1))
        caretPos = findNewline(max(0, temp_pos.y))-1;
      if (caretPos < 0)
        caretPos = 0;
      break;
    case Direction::Down:
      temp_pos = getCaretPos(caretPos);
      caret_pos_temp = findNewline(temp_pos.y+1);
      caretPos = caret_pos_temp;
      caretSeek(cachedXPos, this);
      caret_pos_check = getCaretPos(caretPos);
      if (caretPos != buffer.length() && caret_pos_check.y != max(0, temp_pos.y+1))
        caretPos = findNewline(max(0, temp_pos.y+2))-1;
      if (caretPos <= 0)
        caretPos = buffer.length();
      break;
  }
  if (!shiftSelect)
    caretSelPos = caretPos;
  setOffs(bufferSize, lineNums);
}

void TextBuffer::ctrlMoveCaret(Direction dir, Size bufferSize, bool shiftSel, bool lineNums) {
  switch (dir) {
    case (Direction::Up):
	ctrlup(this, shiftSel);
      break;
    case (Direction::Down):
	ctrldown(this, shiftSel);
      break;
    case (Direction::Left):
        ctrlleft(this, shiftSel);
      break;
    case (Direction::Right):
        ctrlright(this, shiftSel);
      break;
  }
  if (this->caretPos > this->buffer.length()) {
    if (this->caretPos == this->caretSelPos)
      this->caretSelPos = this->buffer.length();
    this->caretPos = this->buffer.length();
  }  
  if (this->caretPos < 0) {
    if (this->caretPos == this->caretSelPos)
      this->caretSelPos = 0;
    this->caretPos = 0;
  }
  this->cachedXPos = this->getCaretPos(this->caretPos).x;
  this->setOffs(bufferSize, lineNums);
}

Point TextBuffer::getCaretPos(int caretPos, bool skipFoldedBlocks) {
    Point pos = {0, 0};
    int nl = 0;
    int unfold_line;
    bool folded = false;
    for (int i = 0; i < buffer.length() + 1; ++i) {
      if (skipFoldedBlocks) {
	if (codeBar.IsBlockFolded(nl)) {
	  folded = true;
	  unfold_line = codeBar.GetNextLine(nl);
	}
	if (folded && unfold_line == nl+1) {
	  folded = false;
	}
      }
      if (caretPos == i)
        return pos;
      if (i < buffer.length() && buffer[i] == '\n') {
	++nl;
        if (!folded) pos.y += 1;
        pos.x = 0;
      }
      else if (i < buffer.length() && buffer[i] == '\t')
        pos.x += 8 - pos.x % 8;
      else
        pos.x += 1;
    }
  return {0,0};
}

int TextBuffer::findNewline(int n, bool skipFoldedBlocks, bool returnLineNumber) {
  int c = 0;
  int sc = 0;
  if (n == 0) return 0;
  int unfold_line;
  bool folded = false;
  for (int p = 0; p < buffer.length(); ++p) {
      if (codeBar.IsBlockFolded(c)) {
	folded = true;
	unfold_line = codeBar.GetNextLine(c);
      }
      if (folded && unfold_line == c+1) {
	folded = false;
      }
      if (buffer[p] == '\n') {
        if (!(skipFoldedBlocks && folded))
	  ++sc;
	++c;
      }
      if (sc == n) return (returnLineNumber ? (c) : (p+1));
  }
  return 0;
}

TextBuffer buildTextBuffer(std::string name, std::string filePath, bool isDirty, 
                     Point offs, Point pos, std::string buffer) {
  TextBuffer textBuffer;
  textBuffer.name 	= name;
  textBuffer.filePath 	= filePath;
  textBuffer.isDirty 	= isDirty;
  textBuffer.offs 	= offs;
  textBuffer.pos	= pos;
  textBuffer.
    cachedXPos 	= 0;
  textBuffer.caretPos 	= 0;
  textBuffer.
    caretSelPos 	= 0;
  textBuffer.buffer	= buffer;
  textBuffer.findText   = "";
  textBuffer.codeBar.Update(&textBuffer);
  return textBuffer;
}

bool handleInputTextBuffer(TextBuffer *buf, int key, Size bufferSize, bool enterEscapes, bool lineNums) {
  if (terminal_state(TK_CONTROL)) {
    if (key == TK_V) buf->paste(bufferSize, lineNums);
    if (key == TK_C) buf->copy(bufferSize, lineNums);
    if (key == TK_X) buf->cut(bufferSize, lineNums);
    if (key == TK_A) {
      buf->caretSelPos = 0;
      buf->caretPos = buf->buffer.size();
      buf->setOffs(bufferSize, lineNums);
    }
    if (key == TK_RIGHT)
      buf->ctrlMoveCaret(Direction::Right, bufferSize, terminal_state(TK_SHIFT), lineNums);
    if (key == TK_LEFT)
      buf->ctrlMoveCaret(Direction::Left, bufferSize, terminal_state(TK_SHIFT), lineNums);
    if (key == TK_UP)
      buf->ctrlMoveCaret(Direction::Up, bufferSize, terminal_state(TK_SHIFT), lineNums);
    if (key == TK_DOWN)
      buf->ctrlMoveCaret(Direction::Down, bufferSize, terminal_state(TK_SHIFT), lineNums);
    if (key == TK_RBRACKET)
      buf->codeBar.FoldAll();
    if (key == TK_LBRACKET)
      buf->codeBar.FoldBracket(buf->getCaretPos(buf->caretPos).y);
    return false;
  }
  else if (terminal_state(TK_ALT)) {
    if (key == TK_RIGHT || key == TK_LEFT) {
      if (key == TK_RIGHT)
        buf->findNext(bufferSize, lineNums);
      if (key == TK_LEFT)
        buf->findPrev(bufferSize, lineNums);
    }
    return false;
  }
  int scrollMult = 4;
  int newLines = 0;
  int pos = 0;
  int start_pos;
  switch (key) {
    case (TK_RIGHT):
      buf->moveCaret(Direction::Right, bufferSize, terminal_state(TK_SHIFT), lineNums);
      break;
    case (TK_LEFT):
      buf->moveCaret(Direction::Left, bufferSize, terminal_state(TK_SHIFT), lineNums);
      break;
    case (TK_DOWN):
      buf->moveCaret(Direction::Down, bufferSize, terminal_state(TK_SHIFT), lineNums);
      break;
    case (TK_UP):
      buf->moveCaret(Direction::Up, bufferSize, terminal_state(TK_SHIFT), lineNums);
      break;
    case (TK_RETURN):
    case (TK_KP_ENTER):
      if (enterEscapes) return true;
      buf->insert("\n", bufferSize, lineNums, true);
      break;
    case (TK_TAB):
      buf->insert("\t", bufferSize, lineNums, true);
      break;
    case (TK_BACKSPACE):
      buf->backspace(bufferSize, lineNums, true);
      break;
    case (TK_HOME):
      buf->caretPos = buf->findNewline(buf->getCaretPos(buf->caretPos).y);
      if (!terminal_state(TK_SHIFT))
        buf->caretSelPos = buf->caretPos;
      buf->cachedXPos = 0;
      buf->setOffs(bufferSize, lineNums);
      break;
    case (TK_END):
      start_pos = buf->caretPos;
      buf->caretPos = buf->findNewline(buf->getCaretPos(buf->caretPos).y+1)-1;
      if (buf->caretPos < start_pos)
        buf->caretPos = buf->buffer.length();
      if (!terminal_state(TK_SHIFT))
        buf->caretSelPos = buf->caretPos;
      buf->cachedXPos = buf->getCaretPos(buf->caretPos).x;
      buf->setOffs(bufferSize, lineNums);
      break;
    case (TK_PAGEUP):
      buf->offs.y += bufferSize.height;
      if (buf->offs.y > 0) buf->offs.y = 0;
      break;
    case (TK_PAGEDOWN):
      buf->offs.y -= bufferSize.height;
      break;
    case (TK_MOUSE_SCROLL):
      buf->offs.y -= terminal_state(TK_MOUSE_WHEEL) * scrollMult;
      buf->offs.y = min(buf->offs.y, 0);
      break;
    default:
      break;
  }
  int padding = (buf->codeBar.GetShowing() ? 1 : 0) + (lineNums ? 4 : 0);
  if (buf->codeBar.GetShowing()
  &&  terminal_state(TK_MOUSE_Y) >= buf->pos.y
  &&  terminal_state(TK_MOUSE_Y) <  buf->pos.y + bufferSize.height
  &&  terminal_state(TK_MOUSE_X) == buf->pos.x + padding - 1) {
    if (key == (TK_MOUSE_LEFT|TK_KEY_RELEASED)) {
      int lineNum = buf->findNewline(terminal_state(TK_MOUSE_Y) - buf->pos.y - buf->offs.y, true, true);
      buf->codeBar.FoldBracket(lineNum);
    }
  }
  if (terminal_state(TK_MOUSE_Y) >= buf->pos.y
  &&  terminal_state(TK_MOUSE_X) >= buf->pos.x + padding
  &&  terminal_state(TK_MOUSE_Y) < buf->pos.y + bufferSize.height
  &&  terminal_state(TK_MOUSE_X) < buf->pos.x - padding + bufferSize.width)
  {
    bool folded = false;
    int unfold_line;
    int actNewLines = 0;
    while ((pos = buf->buffer.find('\n', pos)) != std::string::npos) {
      if (buf->codeBar.IsBlockFolded(actNewLines)) {
	folded = true;
	unfold_line = buf->codeBar.GetNextLine(actNewLines);
      }
      if (folded)
	if (unfold_line == actNewLines + 1)
	  folded = false;
      ++actNewLines;
      if (folded == false)
	++newLines;
      ++pos;
    }
    if (key == TK_MOUSE_LEFT) {
      if (terminal_state(TK_MOUSE_Y) - buf->pos.y - buf->offs.y > newLines)
        buf->caretPos = buf->buffer.length();
      else {
        start_pos = buf->findNewline(terminal_state(TK_MOUSE_Y) - buf->pos.y - buf->offs.y, true);
        int act_pos = start_pos;
        for (buf->caretPos = start_pos; act_pos < start_pos + terminal_state(TK_MOUSE_X) - padding - buf->offs.x - buf->pos.x; ++buf->caretPos) {
          if (buf->buffer.length() <= buf->caretPos) {
	    buf->caretPos = buf->buffer.length();
	    break;
          }
          if (buf->buffer[buf->caretPos] == '\n')
            break;
          if (buf->buffer[buf->caretPos] == '\t')
            act_pos += 8 - buf->getCaretPos(buf->caretPos).x % 8;
          else
            ++act_pos;
        }
      }
      buf->caretSelPos = buf->caretPos;
    }
    if (key == (TK_MOUSE_LEFT|TK_KEY_RELEASED)) {
      if (terminal_state(TK_MOUSE_Y) - buf->pos.y - buf->offs.y > newLines)
        buf->caretPos = buf->buffer.length();
      else {
        start_pos = buf->findNewline(terminal_state(TK_MOUSE_Y) - buf->pos.y - buf->offs.y, true);
        int act_pos = start_pos;
        for (buf->caretPos = start_pos; act_pos < start_pos + terminal_state(TK_MOUSE_X) - padding - buf->offs.x - buf->pos.x; ++buf->caretPos) {
          if (buf->buffer.length() <= buf->caretPos) {
	    buf->caretPos = buf->buffer.length();
	    break;
          }
          if (buf->buffer[buf->caretPos] == '\n')
            break;
          if (buf->buffer[buf->caretPos] == '\t')
            act_pos += 8 - buf->getCaretPos(buf->caretPos).x % 8;
          else
            ++act_pos;
        }
      }
    }
    if (terminal_check(TK_MOUSE_LEFT)) {
      if (terminal_state(TK_MOUSE_Y) - buf->pos.y - buf->offs.y > newLines)
        buf->caretPos = buf->buffer.length();
      else {
        if (buf->offs.y > 0) buf->offs.y = 0;
        start_pos = buf->findNewline(terminal_state(TK_MOUSE_Y) - buf->pos.y - buf->offs.y, true);
        int act_pos = start_pos;
        for (buf->caretPos = start_pos; act_pos < start_pos + terminal_state(TK_MOUSE_X) - padding - buf->offs.x - buf->pos.x; ++buf->caretPos) {
          if (buf->buffer.length() <= buf->caretPos) {
	    buf->caretPos = buf->buffer.length();
            break;
          }
          if (buf->buffer[buf->caretPos] == '\n')
            break;
          if (buf->buffer[buf->caretPos] == '\t')
            act_pos += 8 - buf->getCaretPos(buf->caretPos).x % 8;
          else
            ++act_pos;
        }
      }
    }
  }
  if (terminal_check(TK_WCHAR))
    buf->insert(std::string{ (char)terminal_state(TK_WCHAR) }, bufferSize, lineNums, true);
  return false;
}

void drawTextBuffer(TextBuffer *buf, Size bufferSize, bool lineNums) {
  color_t term_color;
  color_t term_bkcolor;
  if (buf->syntax.bufVec.size() != buf->buffer.length()) {
    buf->syntax.updateSyntax(buf, buf->name);
    buf->codeBar.Update(buf);
  }
  bool cbShowing = buf->codeBar.GetShowing();
  terminal_color(term_color = color_from_name("workspacedefaultfg"));
  terminal_bkcolor(term_bkcolor = color_from_name("workspacedefaultbk"));
  for (int i = buf->pos.x; i < buf->pos.x + bufferSize.width; ++i)
    for (int j = buf->pos.y; j < buf->pos.y + bufferSize.height; ++j) {
      terminal_put(i, j, ' ');
    }
  int y = 0;
  int x = 0;
  bool in_fold = false;
  int fold_end;
  if (lineNums) {
    terminal_bkcolor(term_bkcolor = color_from_name("workspacedefaultbk"));
    terminal_clear_area(0, buf->pos.y, 4, 1);
    terminal_print(0, buf->pos.y, std::to_string(std::abs(buf->offs.y) + 1).c_str());
  }
  if (cbShowing) {
    terminal_bkcolor(term_bkcolor = color_from_name("workspacedefaultbk"));
    terminal_color(term_color = color_from_name("workspacedefaultfg"));
    terminal_print((lineNums ? 4 : 0), buf->pos.y + y, buf->codeBar.IsBlockAtLine(y + -(buf->offs.y)) ? (buf->codeBar.IsBlockFolded(y - buf->offs.y) ? "+" : "-") : "|");
    if (y > 0 && buf->codeBar.IsBlockFolded(y-1)) {
      in_fold = true;
      fold_end = buf->codeBar.GetNextLine(y-1) + 1;
    }
  }
  int padding = (cbShowing ? 1 : 0) + (lineNums ? 4 : 0);
  int i = 0;
  int temp_y_offs = 0;
  if (buf->buffer.length() == 0) {
    terminal_bkcolor(color_from_name("workspacedefaultfg"));
    terminal_color(color_from_name("workspacedefaultbk"));
    terminal_put(padding + buf->pos.x + buf->offs.x + x, buf->pos.y + buf->offs.y + y - temp_y_offs, ' ');
    terminal_bkcolor(term_bkcolor);
    terminal_color(term_color);
  }
  for (char c : buf->buffer) {
    if (in_fold) {
      if (y == fold_end)
        in_fold = false;
    }
    if (y + buf->offs.y - temp_y_offs >= bufferSize.height)
      break;
    if (y + buf->offs.y >= 0 && !in_fold) {
      color_t *color = &(buf->syntax.bufVec[i]);
      if (i + 1 == buf->caretPos && i == buf->buffer.length() - 1) {
        terminal_bkcolor(color_from_name("workspacedefaultfg"));
        terminal_color(color_from_name("workspacedefaultbk"));
        if (c != '\n')
          terminal_put(padding + buf->pos.x + buf->offs.x + x + 1, buf->pos.y + buf->offs.y + y - temp_y_offs, ' ');
	else 
          terminal_put(padding + buf->pos.x + buf->offs.x + 0, buf->pos.y + buf->offs.y + y - temp_y_offs + 1, ' ');
	terminal_bkcolor(term_bkcolor);
        terminal_color(term_color);
      }
      else if (i == buf->caretPos) {
        terminal_bkcolor(term_bkcolor = color_from_name("workspacedefaultfg"));
        terminal_color(term_color = color_from_name("workspacedefaultbk"));
        color = &term_color;
      }
      else if (((i >= buf->caretSelPos && i < buf->caretPos) || (i >= buf->caretPos && i < buf->caretSelPos)) && !(c == '\n' && i > 0 && buf->buffer[i-1] != '\n')) {
        terminal_bkcolor(term_bkcolor = color_from_name("workspacehlbk"));
      }
      else
        terminal_bkcolor(term_bkcolor = color_from_name("workspacedefaultbk"));
      
      if (term_color != *color) terminal_color(term_color = *color);
      if (c != '\n' && c != '\t' 
      &&  x + buf->offs.x < bufferSize.width - padding && x + buf->offs.x >= 0)
        terminal_put(padding + buf->pos.x + buf->offs.x + x, buf->pos.y + buf->offs.y + y - temp_y_offs, c);
    }
    if (c == '\n') {
      if (!in_fold)
        terminal_put(padding + buf->pos.x + buf->offs.x + x, buf->pos.y + buf->offs.y + y - temp_y_offs, ' ');
      ++y;
      if (in_fold) ++temp_y_offs;
      x = 0;
      if (lineNums) {
        terminal_color(term_color = color_from_name("workspacedefaultfg"));
        terminal_bkcolor(term_bkcolor = color_from_name("workspacedefaultbk"));
        terminal_clear_area(0, buf->pos.y + buf->offs.y + y - temp_y_offs, 4, 1);
        terminal_print(0, buf->pos.y + buf->offs.y + y - temp_y_offs, std::to_string(1 + y).c_str());
      }
      if (cbShowing) {
        terminal_bkcolor(term_bkcolor = color_from_name("workspacedefaultbk"));
        terminal_color(term_color = color_from_name("workspacedefaultfg"));
        terminal_print((lineNums ? 4 : 0), buf->pos.y + buf->offs.y + y - temp_y_offs, buf->codeBar.IsBlockAtLine(y) ? (buf->codeBar.IsBlockFolded(y) ? "+" : "-") : "|");
	if (y > 0 && buf->codeBar.IsBlockFolded(y-1)) {
          in_fold = true;
          fold_end = buf->codeBar.GetNextLine(y-1);
        }
      }
    }
    else if (c == '\t') {
      if (!in_fold)
        for (int cx = x; cx <= x + 8 - x % 8; cx++)
          terminal_put(padding + buf->pos.x + buf->offs.x + cx, buf->pos.y + buf->offs.y + y - temp_y_offs, ' ');
      x += 8 - x % 8;
    }
    else ++x;
    ++i;
  }
  terminal_color(color_from_name("workspacedefaultfg"));
  terminal_bkcolor(color_from_name("workspacedefaultbk"));
}
