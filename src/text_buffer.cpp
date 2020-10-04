#include "text_buffer.h"
#include "../include/BearLibTerminal.h"
#include "clipboard.h"
#include <ctype.h>
#include <vector>

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif


void Action::undo(TextBuffer *buffer, Size size, bool lineNums) {
  switch (this->type) {
    case (ActionType::Insert):
      buffer->caretPos = this->caretPos;
      buffer->caretSelPos = this->caretPos + this->text.length();
      buffer->backspace(size, lineNums, false);
      break;
    case (ActionType::Delete):
      buffer->caretPos = this->caretPos;
      buffer->caretSelPos = this->caretPos;
      buffer->insert(this->text, size, lineNums, false);
      break;
  }
}

void Action::redo(TextBuffer *buffer, Size size, bool lineNums) {
  switch (this->type) {
    case (ActionType::Insert):
      buffer->caretPos = this->caretPos;
      buffer->caretSelPos = this->caretPos;
      buffer->insert(this->text, size, lineNums, false);
      break;
    case (ActionType::Delete):
      buffer->caretPos = this->caretPos;
      buffer->caretSelPos = this->caretPos + this->text.length();
      buffer->backspace(size, lineNums, false);
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
  if (buf->caretPos < buf->buffer.size() - 1) {
    vector<int> places;
    vector<char> keychars = {
      ' ', '\\', '/', '<', '>', '.', ',', '{', '}', '(', ')', '\"', ';', '=', '-', '+', 
      '|', '&', '*', '!', '%', '[', ']', ':', '\n', '\t'
    };
    for (char c : keychars)
      for (int i = buf->caretPos+1; i < buf->buffer.length(); ++i)
        if (i == buf->buffer.length() - 1 || isalpha(buf->buffer[i]) && std::count(keychars.begin(), keychars.end(), buf->buffer[i-1])) {
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
    vector<int> places;
    vector<char> keychars = {
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


void TextBuffer::backspace(Size size, bool lineNums, bool undo) {
  int len = caretPos == caretSelPos ? 1 : max(caretPos, caretSelPos) - min(caretPos, caretSelPos);
  if (undo) {
    undos.push_back({ 
      ActionType::Delete, 
      buffer.substr(min(caretPos, caretSelPos), len),
      min(caretPos, caretSelPos)
    });
    redos.clear();
  }
  std::string first_half, second_half;
  if (caretPos == caretSelPos) {
    first_half = buffer.substr(0, max(0, caretPos-1));
    second_half = buffer.substr(caretPos, buffer.length() + 1 - caretPos);
    moveCaret(Direction::Left, size, false, lineNums);
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

void TextBuffer::insert(std::string str, Size size, bool lineNums, bool undo) {
  if (caretPos != caretSelPos)
    backspace(size, lineNums, true);
  if (undo) {
    undos.push_back({ 
      ActionType::Insert, 
      str,
      caretPos
    });
    redos.clear();
  }
  std::string first_half = buffer.substr(0, caretPos);
  std::string second_half = buffer.substr(caretPos, buffer.length() + 1 - caretPos);
  buffer = first_half + str + second_half;
  caretPos += str.length() - 1;
  moveCaret(Direction::Right, size, false, lineNums);
  cachedXPos = getCaretPos(caretPos).x;
  isDirty = true;
}

void TextBuffer::paste(Size size, bool lineNums) {
  if (caretPos != caretSelPos)
    backspace(size, lineNums, true);
  std::string clipboard = GetClipboardContents();
  insert(clipboard, size, lineNums, true);
}

void TextBuffer::copy(Size size, bool lineNums) {
  if (caretPos != caretSelPos) {
    std::string text_to_copy = buffer.substr(min(caretPos,caretSelPos), max(caretPos,caretSelPos) - min(caretPos,caretSelPos));
    PushTextToClipboard(text_to_copy);
  }
}

void TextBuffer::cut(Size size, bool lineNums) {
  if (caretPos != caretSelPos) {
    std::string text_to_copy = buffer.substr(min(caretPos,caretSelPos), max(caretPos,caretSelPos) - min(caretPos,caretSelPos));
    PushTextToClipboard(text_to_copy);
    backspace(size, lineNums, true);
  }
}

void TextBuffer::findNext(Size size, bool lineNums) {
  if (findText == "")
    return;
  int i = 1;
  while (caretPos + i < buffer.length()) {
    if (buffer.substr(caretPos + i, findText.length()) == findText
    &&  caretPos + i != caretPos)
      break;
    ++i;
  }
  if (caretPos + i < buffer.length())
    caretPos += i;
  caretSelPos = caretPos;
  caretSelPos += findText.length();
  setOffs(size, lineNums);
}

void TextBuffer::findPrev(Size size, bool lineNums) {
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
  setOffs(size, lineNums);
}

void TextBuffer::undo(Size size, bool lineNums) {
  if (undos.size() > 0) {
    undos.back().undo(this, size, lineNums);
    redos.push_back(undos.back());
    undos.pop_back();
  }
}

void TextBuffer::redo(Size size, bool lineNums) {
  if (redos.size() > 0) {
    redos.back().redo(this, size, lineNums);
    undos.push_back(redos.back());
    redos.pop_back();
  }
}

void TextBuffer::setOffs(Size size, bool lineNums) {
  Point temp = getCaretPos(caretPos);
  Point temp2 = getCaretPos(caretSelPos);
  if (temp2.x < std::abs(this->offs.x))
    this->offs.x = -temp2.x;
  if (temp2.x >= std::abs(this->offs.x) + size.width - (lineNums ? 4 : 0))
    this->offs.x = - (temp2.x - size.width + (lineNums ? 4 : 0))-1;
  if (temp2.y < std::abs(this->offs.y))
    this->offs.y = -temp2.y;
  if (temp2.y >= std::abs(this->offs.y) + size.height)
    this->offs.y = - (temp2.y - size.height)-1;
  if (temp.x < std::abs(this->offs.x))
    this->offs.x = -temp.x;
  if (temp.x >= std::abs(this->offs.x) + size.width - (lineNums ? 4 : 0))
    this->offs.x = - (temp.x - size.width + (lineNums ? 4 : 0))-1;
  if (temp.y < std::abs(this->offs.y))
    this->offs.y = -temp.y;
  if (temp.y >= std::abs(this->offs.y) + size.height)
    this->offs.y = - (temp.y - size.height)-1;
}

void TextBuffer::moveCaret(Direction dir, Size size, bool shiftSelect, bool lineNums) {
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
  setOffs(size, lineNums);
}

void TextBuffer::ctrlMoveCaret(Direction dir, Size size, bool shiftSel, bool lineNums) {
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
  this->setOffs(size, lineNums);
}

Point TextBuffer::getCaretPos(int caretPos) {
    Point pos = {0,0};
    for (int i = 0; i < buffer.length() + 1; ++i) {
      if (caretPos == i)
        return pos;
      if (i < buffer.length() && buffer[i] == '\n') {
        pos.y += 1;
        pos.x = 0;
      }
      else if (i < buffer.length() && buffer[i] == '\t')
        pos.x += 8 - pos.x % 8;
      else
        pos.x += 1;
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
  textBuffer.syntax.updateSyntax(&textBuffer, name);
  return textBuffer;
}

bool handleInputTextBuffer(TextBuffer *buf, int key, Size size, bool enterEscapes, bool lineNums) {
  if (terminal_state(TK_CONTROL)) {
    if (key == TK_V) buf->paste(size, lineNums);
    if (key == TK_C) buf->copy(size, lineNums);
    if (key == TK_X) buf->cut(size, lineNums);
    if (key == TK_RIGHT)
      buf->ctrlMoveCaret(Direction::Right, size, terminal_state(TK_SHIFT), lineNums);
    if (key == TK_LEFT)
      buf->ctrlMoveCaret(Direction::Left, size, terminal_state(TK_SHIFT), lineNums);
    if (key == TK_UP)
      buf->ctrlMoveCaret(Direction::Up, size, terminal_state(TK_SHIFT), lineNums);
    if (key == TK_DOWN)
      buf->ctrlMoveCaret(Direction::Down, size, terminal_state(TK_SHIFT), lineNums);
    return false;
  }
  else if (terminal_state(TK_ALT)) {
    if (key == TK_RIGHT || key == TK_LEFT) {
      if (key == TK_RIGHT)
        buf->findNext(size, lineNums);
      if (key == TK_LEFT)
        buf->findPrev(size, lineNums);
    }
    return false;
  }
  int scrollMult = 4;
  int newLines = 0;
  int pos = 0;
  int start_pos;
  switch (key) {
      case (TK_RIGHT):
	buf->moveCaret(Direction::Right, size, terminal_state(TK_SHIFT), lineNums);
	break;
      case (TK_LEFT):
	buf->moveCaret(Direction::Left, size, terminal_state(TK_SHIFT), lineNums);
	break;
      case (TK_DOWN):
	buf->moveCaret(Direction::Down, size, terminal_state(TK_SHIFT), lineNums);
	break;
      case (TK_UP):
	buf->moveCaret(Direction::Up, size, terminal_state(TK_SHIFT), lineNums);
	break;
      case (TK_RETURN):
      case (TK_KP_ENTER):
	if (enterEscapes) return true;
	buf->insert("\n", size, lineNums, true);
	break;
      case (TK_TAB):
	buf->insert("\t", size, lineNums, true);
	break;
      case (TK_BACKSPACE):
	buf->backspace(size, lineNums, true);
	break;
      case (TK_HOME):
	buf->caretPos = buf->findNewline(buf->getCaretPos(buf->caretPos).y);
	if (!terminal_state(TK_SHIFT))
	  buf->caretSelPos = buf->caretPos;
        buf->cachedXPos = 0;
	buf->setOffs(size, lineNums);
	break;
      case (TK_END):
        start_pos = buf->caretPos;
	buf->caretPos = buf->findNewline(buf->getCaretPos(buf->caretPos).y+1)-1;
        if (buf->caretPos < start_pos)
          buf->caretPos = buf->buffer.length();
	if (!terminal_state(TK_SHIFT))
	  buf->caretSelPos = buf->caretPos;
	buf->cachedXPos = buf->getCaretPos(buf->caretPos).x;
        buf->setOffs(size, lineNums);
	break;
      case (TK_PAGEUP):
	buf->offs.y += size.height;
	if (buf->offs.y > 0) buf->offs.y = 0;
	break;
      case (TK_PAGEDOWN):
	buf->offs.y -= size.height;
	break;
      case (TK_MOUSE_SCROLL):
        while ((pos = buf->buffer.find('\n', pos)) != std::string::npos) {
          ++newLines;
          ++pos;
        }
        if (terminal_check(TK_MOUSE_LEFT)) {
	  buf->offs.y -= terminal_state(TK_MOUSE_WHEEL) * scrollMult;
          break;
        }
        buf->caretPos = buf->findNewline(buf->getCaretPos(buf->caretPos).y - 
                           ((terminal_state(TK_MOUSE_WHEEL) < 0) ? (scrollMult) : 
                             ((buf->getCaretPos(buf->caretPos).y < newLines - scrollMult) ? 
                               -scrollMult : -(newLines - buf->getCaretPos(buf->caretPos).y))));
	buf->caretSelPos = buf->caretPos;
	buf->cachedXPos = buf->getCaretPos(buf->caretPos).x;
        buf->setOffs(size, lineNums);
	break;
      default:
        break;
  }
  if (terminal_state(TK_MOUSE_Y) >= buf->pos.y
  &&  terminal_state(TK_MOUSE_X) >= buf->pos.x
  &&  terminal_state(TK_MOUSE_Y) < buf->pos.y + size.height
  &&  terminal_state(TK_MOUSE_X) < buf->pos.x + size.width)
  {
  if (key == TK_MOUSE_LEFT) {
    while ((pos = buf->buffer.find('\n', pos)) != std::string::npos) {
      ++newLines;
      ++pos;
    }
    if (terminal_state(TK_MOUSE_Y) - buf->pos.y - buf->offs.y > newLines)
      buf->caretPos = buf->buffer.length();
    else {
      start_pos = buf->findNewline(terminal_state(TK_MOUSE_Y) - buf->pos.y - buf->offs.y);
      int act_pos = start_pos;
      for (buf->caretPos = start_pos; act_pos < start_pos + terminal_state(TK_MOUSE_X) - ((lineNums) ? 4 : 0) - buf->offs.x - buf->pos.x; ++buf->caretPos) {
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
    while ((pos = buf->buffer.find('\n', pos)) != std::string::npos) {

      ++newLines;
      ++pos;

    }
    if (terminal_state(TK_MOUSE_Y) - buf->pos.y - buf->offs.y > newLines)
      buf->caretPos = buf->buffer.length();
    else {
    start_pos = buf->findNewline(terminal_state(TK_MOUSE_Y) - buf->pos.y - buf->offs.y);
    int act_pos = start_pos;
    for (buf->caretPos = start_pos; act_pos < start_pos + terminal_state(TK_MOUSE_X) - ((lineNums) ? 4 : 0) - buf->offs.x - buf->pos.x; ++buf->caretPos) {
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
    while ((pos = buf->buffer.find('\n', pos)) != std::string::npos) {

      ++newLines;
      ++pos;

    }
    if (terminal_state(TK_MOUSE_Y) - buf->pos.y - buf->offs.y > newLines)
      buf->caretPos = buf->buffer.length();
    else {
    if (buf->offs.y > 0) buf->offs.y = 0;
    start_pos = buf->findNewline(terminal_state(TK_MOUSE_Y) - buf->pos.y - buf->offs.y);
    int act_pos = start_pos;
    for (buf->caretPos = start_pos; act_pos < start_pos + terminal_state(TK_MOUSE_X) - ((lineNums) ? 4 : 0) - buf->offs.x - buf->pos.x; ++buf->caretPos) {
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
    buf->insert(std::string{ (char)terminal_state(TK_WCHAR) }, size, lineNums, true);
  return false;
}

void drawTextBuffer(TextBuffer *buf, Size size, bool lineNums) {
  color_t term_color;
  color_t term_bkcolor;
  if (buf->syntax.bufVec.size() != buf->buffer.length()) {
    buf->syntax.updateSyntax(buf, buf->name);
  }
  terminal_color(term_color = color_from_name("workspacedefaultfg"));
  terminal_bkcolor(term_bkcolor = color_from_name("workspacedefaultbk"));
  for (int i = buf->pos.x; i < buf->pos.x + size.width; ++i)
    for (int j = buf->pos.y; j < buf->pos.y + size.height; ++j) {
      terminal_put(i, j, ' ');
    }
  int y = 0;
  int x = 0;
  if (lineNums) {
    terminal_bkcolor(term_bkcolor = color_from_name("dark workspacedefaultbk"));
    terminal_clear_area(0, buf->pos.y, 4, 1);
    terminal_print(0, buf->pos.y, std::to_string(std::abs(buf->offs.y) + 1).c_str());
    terminal_bkcolor(term_bkcolor = color_from_name("workspacedefaultbk"));
  }
  int i = 0;
  for (char c : buf->buffer) {
    if (y + buf->offs.y >= size.height)
      break;
    if (y + buf->offs.y >= 0) {
      color_t *color = &(buf->syntax.bufVec[i]);
      if (((i >= buf->caretSelPos && i < buf->caretPos) || (i >= buf->caretPos && i < buf->caretSelPos)) 
      &&    buf->caretPos != buf->caretSelPos) {
        terminal_bkcolor(term_bkcolor = color_from_name("workspacehlbk"));
      }
      if (i == buf->caretPos && buf->caretPos > buf->caretSelPos
      ||  i == buf->caretSelPos && buf->caretSelPos > buf->caretPos)
        terminal_bkcolor(term_bkcolor = color_from_name("workspacedefaultbk"));
      
      if (term_color != *color) terminal_color(term_color = *color);
      if (c != '\n' && c != '\t' 
      &&  x + buf->offs.x < size.width - (lineNums ? 4 : 0) && x + buf->offs.x >= 0 
      &&  y + buf->offs.y < size.height)
        terminal_put((lineNums ? 4 : 0) + buf->pos.x + buf->offs.x + x, buf->pos.y + buf->offs.y + y, c);
    }
    if (c == '\n') {
      if (buf->buffer[i-1] == '\n')
        terminal_put((lineNums ? 4 : 0) + buf->pos.x + buf->offs.x + x, buf->pos.y + buf->offs.y + y, ' ');
      ++y;
      x = 0;
      if (lineNums) {
        terminal_bkcolor(term_bkcolor = color_from_name("dark workspacedefaultbk"));
        terminal_clear_area(0, buf->pos.y + y, 4, 1);
        terminal_print(0, buf->pos.y + y, std::to_string(std::abs(buf->offs.y) + 1 + y).c_str());
        terminal_bkcolor(term_bkcolor = color_from_name("workspacedefaultbk"));
      }
    }
    else if (c == '\t') {
      for (int cx = x; cx <= x + 8 - x % 8; cx++)
        terminal_put((lineNums ? 4 : 0) + buf->pos.x + buf->offs.x + cx, buf->pos.y + buf->offs.y + y, ' ');
      x += 8 - x % 8;
    }
    else ++x;
    ++i;
  }

  terminal_color(color_from_name("workspacedefaultbk"));
  terminal_bkcolor(color_from_name("workspacedefaultfg"));
  Point caretPos = buf->getCaretPos(buf->caretPos);
  char prev_char = terminal_pick((lineNums ? 4 : 0) + buf->pos.x + caretPos.x + buf->offs.x, buf->pos.y + caretPos.y + buf->offs.y);
  terminal_put((lineNums ? 4 : 0) + buf->pos.x + caretPos.x + buf->offs.x, buf->pos.y + caretPos.y + buf->offs.y, prev_char);

  terminal_color(color_from_name("workspacedefaultfg"));
  terminal_bkcolor(color_from_name("workspacedefaultbk"));
}