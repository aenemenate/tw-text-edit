#include "text_buffer.h"
#include "../include/BearLibTerminal.h"
#include "color_palette.h"
#include "clipboard.h"

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

void caretSeek(int amount, TextBuffer *textBuffer) {
  int caret_pos = textBuffer->caret_pos;
  int x = 0;
  for (int i = 0; i < amount; ++x) {
    if (textBuffer->buffer[caret_pos + x] == '\t') {
      i += 8 - textBuffer->getCaretPos(caret_pos).x % 8;
    }
    else ++i;
    textBuffer->caret_pos++;
  }
}

void TextBuffer::backspace(Size size, bool lineNums) {
  std::string first_half, second_half;
  if (caret_pos == caret_sel_pos) {
    first_half = buffer.substr(0, max(0, caret_pos-1));
    second_half = buffer.substr(caret_pos, buffer.length() + 1 - caret_pos);
    moveCaret(Direction::Left, size, false, lineNums);
  }
  else if (caret_pos > caret_sel_pos) {
    first_half = buffer.substr(0, max(0, caret_sel_pos));
    second_half = buffer.substr(caret_pos, buffer.length() + 1 - caret_pos);
    caret_pos = caret_sel_pos;
  }
  else {
    first_half = buffer.substr(0, max(0, caret_pos));
    second_half = buffer.substr(caret_sel_pos, buffer.length() + 1 - caret_sel_pos);
  }
  buffer = first_half + second_half;
  cached_x_pos = getCaretPos(caret_pos).x;
  isDirty = true;
  caret_sel_pos = caret_pos;
}

void TextBuffer::insertChar(char c, Size size, bool lineNums) {
  if (caret_pos != caret_sel_pos)
    backspace(size, lineNums);
  std::string first_half = buffer.substr(0, caret_pos);
  std::string second_half = buffer.substr(caret_pos, buffer.length() + 1 - caret_pos);
  buffer = first_half + c + second_half;
  moveCaret(Direction::Right, size, false, lineNums);
  cached_x_pos = getCaretPos(caret_pos).x;
  isDirty = true;
}

void TextBuffer::paste(Size size, bool lineNums) {
  if (caret_pos != caret_sel_pos)
    backspace(size, lineNums);
  std::string first_half = buffer.substr(0, caret_pos);
  std::string second_half = buffer.substr(caret_pos, buffer.length() + 1 - caret_pos);
  std::string clipboard = GetClipboardContents();
  buffer = first_half + clipboard + second_half;
  caretSeek(clipboard.length() - 1, this);
  moveCaret(Direction::Right, size, false, lineNums);
  cached_x_pos = getCaretPos(caret_pos).x;
  isDirty = true;
}

void TextBuffer::copy(Size size, bool lineNums) {
  if (caret_pos != caret_sel_pos) {
    std::string text_to_copy = buffer.substr(min(caret_pos,caret_sel_pos), max(caret_pos,caret_sel_pos) - min(caret_pos,caret_sel_pos));
    PushTextToClipboard(text_to_copy);
  }
}

void TextBuffer::cut(Size size, bool lineNums) {
  if (caret_pos != caret_sel_pos) {
    std::string text_to_copy = buffer.substr(min(caret_pos,caret_sel_pos), max(caret_pos,caret_sel_pos) - min(caret_pos,caret_sel_pos));
    PushTextToClipboard(text_to_copy);
    backspace(size, lineNums);
  }
}

void TextBuffer::moveCaret(Direction dir, Size size, bool shiftSelect, bool lineNums) {
  Point temp_pos;
  Point caret_pos_check;
  int caret_pos_temp;
  switch (dir) {
    case Direction::Left: 
      caret_pos -= 1; 
      if (caret_pos < 0)
        caret_pos = 0;
      cached_x_pos = getCaretPos(caret_pos).x;
      break;
    case Direction::Right: 
      caret_pos += 1;
      if (caret_pos > buffer.length())
        caret_pos = buffer.length();
      cached_x_pos = getCaretPos(caret_pos).x;
      break;
    case Direction::Up:
      temp_pos = getCaretPos(caret_pos);
      caret_pos_temp = findNewline(max(0, temp_pos.y-1));
      caret_pos = caret_pos_temp;
      caretSeek(cached_x_pos, this);
      caret_pos_check = getCaretPos(caret_pos);
      if (caret_pos_check.y != max(0, temp_pos.y-1))
        caret_pos = findNewline(max(0, temp_pos.y))-1;
      if (caret_pos < 0)
        caret_pos = 0;
      break;
    case Direction::Down:
      temp_pos = getCaretPos(caret_pos);
      caret_pos_temp = findNewline(temp_pos.y+1);
      caret_pos = caret_pos_temp;
      caretSeek(cached_x_pos, this);
      caret_pos_check = getCaretPos(caret_pos);
      if (caret_pos != buffer.length() && caret_pos_check.y != max(0, temp_pos.y+1))
        caret_pos = findNewline(max(0, temp_pos.y+2))-1;
      if (caret_pos <= 0)
        caret_pos = buffer.length();
      break;
  }
  Point temp = getCaretPos(caret_pos);
  if (temp.x < std::abs(this->offs.x))
    this->offs.x = -temp.x;
  if (temp.x >= std::abs(this->offs.x) + size.width - (lineNums ? 4 : 0))
    this->offs.x = - (temp.x - size.width + (lineNums ? 4 : 0))-1;
  if (temp.y < std::abs(this->offs.y))
    this->offs.y = -temp.y;
  if (temp.y >= std::abs(this->offs.y) + size.height)
    this->offs.y = - (temp.y - size.height)-1;
  
  if (!shiftSelect)
    caret_sel_pos = caret_pos;
}

Point TextBuffer::getCaretPos(int caret_pos) {
    Point pos = {0,0};
    for (int i = 0; i < buffer.length() + 1; ++i) {
      if (caret_pos == i)
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

TextBuffer buildTextBuffer(std::string name, std::string filepath, bool isDirty, 
                     Point offs, Point pos, std::string buffer) {
  TextBuffer textBuffer;
  textBuffer.name 	= name;
  textBuffer.filepath 	= filepath;
  textBuffer.isDirty 	= isDirty;
  textBuffer.offs 	= offs;
  textBuffer.pos	= pos;
  textBuffer.
    cached_x_pos 	= 0;
  textBuffer.caret_pos = 0;
  textBuffer.
    caret_sel_pos = 0;
  textBuffer.buffer	= buffer;
  return textBuffer;
}

bool handleInputTextBuffer(TextBuffer *buf, int key, Size size, bool enterEscapes, bool lineNums) {
  if (terminal_state(TK_CONTROL)) {
    if (key == TK_V) buf->paste(size, lineNums);
    if (key == TK_C) buf->copy(size, lineNums);
    if (key == TK_X) buf->cut(size, lineNums);
    return false;
  }
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
  }
  if (key == TK_RETURN || key == TK_ENTER || key == TK_KP_ENTER) {
    if (enterEscapes) return true;
    buf->insertChar('\n', size, lineNums);
  }
  else if (key == TK_TAB)
    buf->insertChar('\t', size, lineNums);
  else if (key == TK_BACKSPACE && buf->buffer.length() > 0) {
    buf->backspace(size, lineNums);
  }
  else if (terminal_check(TK_WCHAR)) {
    buf->insertChar((char)terminal_state(TK_WCHAR), size, lineNums);
  }
  return false;
}

void drawTextBuffer(TextBuffer *buf, ColorPalette *colorPalette, Size size, bool lineNums) {
  terminal_color(colorPalette->workspaceDefaultColor.fg.c_str());
  terminal_bkcolor(colorPalette->workspaceDefaultColor.bg.c_str());
  for (int i = buf->pos.x; i < buf->pos.x + size.width; ++i)
    for (int j = buf->pos.y; j < buf->pos.y + size.height; ++j) {
      terminal_put(i, j, ' ');
    }
  int y = 0;
  int x = 0;
  if (lineNums) {
    terminal_bkcolor(std::string{"dark " + colorPalette->workspaceDefaultColor.bg}.c_str());
    terminal_clear_area(0, buf->pos.y, 4, 1);
    terminal_print(0, buf->pos.y, std::to_string(std::abs(buf->offs.y) + 1).c_str());
    terminal_bkcolor(colorPalette->workspaceDefaultColor.bg.c_str());
  }
  int i = 0;
  for (char c : buf->buffer) {
    if ((i >= buf->caret_sel_pos && i <= buf->caret_pos) || (i >= buf->caret_pos && i <= buf->caret_sel_pos)) 
      terminal_bkcolor("dark orange");
    else terminal_bkcolor(colorPalette->workspaceDefaultColor.bg.c_str());
    if (c != '\n' && c != '\t' && x + buf->offs.x < size.width - (lineNums ? 4 : 0) && x + buf->offs.x >= 0 && y + buf->offs.y < size.height && y + buf->offs.y >= 0)
      terminal_put((lineNums ? 4 : 0) + buf->pos.x + buf->offs.x + x, buf->pos.y + buf->offs.y + y, c);
    if (c == '\n') {
      ++y;
      x = 0;
      terminal_put((lineNums ? 4 : 0) + buf->pos.x + buf->offs.x + x, buf->pos.y + buf->offs.y + y, ' ');
      if (lineNums) {
        terminal_bkcolor(std::string{"dark " + colorPalette->workspaceDefaultColor.bg}.c_str());
        terminal_clear_area(0, buf->pos.y + y, 4, 1);
        terminal_print(0, buf->pos.y + y, std::to_string(std::abs(buf->offs.y) + 1 + y).c_str());
        terminal_bkcolor(colorPalette->workspaceDefaultColor.bg.c_str());
      }
    }
    else if (c == '\t') {
      for (int cx = x; cx <= x + 8 - x % 8; cx++)
        terminal_put((lineNums ? 4 : 0) + buf->pos.x + buf->offs.x + x, buf->pos.y + buf->offs.y + y, ' ');
      x += 8 - x % 8;
    }
    else ++x;
    ++i;
  }

  terminal_color(colorPalette->workspaceDefaultColor.bg.c_str());
  terminal_bkcolor(colorPalette->workspaceDefaultColor.fg.c_str());
  Point caret_pos = buf->getCaretPos(buf->caret_pos);
  char prev_char = terminal_pick((lineNums ? 4 : 0) + buf->pos.x + caret_pos.x + buf->offs.x, buf->pos.y + caret_pos.y + buf->offs.y);
  terminal_put((lineNums ? 4 : 0) + buf->pos.x + caret_pos.x + buf->offs.x, buf->pos.y + caret_pos.y + buf->offs.y, prev_char);

  terminal_color(colorPalette->workspaceDefaultColor.fg.c_str());
  terminal_bkcolor(colorPalette->workspaceDefaultColor.bg.c_str());
}