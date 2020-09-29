#include "text_buffer.h"
#include "../include/BearLibTerminal.h"
#include "color_palette.h"

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

void caretSeek(int amount, TextBuffer *textBuffer) {
  int caret_pos = textBuffer->caret_pos;
  int x = 0;
  for (int i = 0; i < amount; ++x) {
    if (textBuffer->buffer[caret_pos + x] == '\t') {
      i += 8 - textBuffer->getCaretPos().x % 8;
    }
    else ++i;
    textBuffer->caret_pos++;
  }
}

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
      caret_pos_temp = findNewline(max(0, temp_pos.y-1));
      caret_pos = caret_pos_temp;
      caretSeek(cached_x_pos, this);
      caret_pos_check = getCaretPos();
      if (caret_pos_check.y != max(0, temp_pos.y-1))
        caret_pos = findNewline(max(0, temp_pos.y))-1;
      if (caret_pos < 0)
        caret_pos = 0;
      break;
    case Direction::Down:
      temp_pos = getCaretPos();
      caret_pos_temp = findNewline(temp_pos.y+1);
      caret_pos = caret_pos_temp;
      caretSeek(cached_x_pos, this);
      caret_pos_check = getCaretPos();
      if (caret_pos != buffer.length() && caret_pos_check.y != max(0, temp_pos.y+1)) {
        caret_pos = findNewline(max(0, temp_pos.y+2))-1;
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
        if (i < buffer.length() && buffer[i] == '\n') {
          pos.y += 1;
          pos.x = 0;
        }
        else if (i < buffer.length() && buffer[i] == '\t') {
          pos.x += 8 - pos.x % 8;
        }
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

bool handleInputTextBuffer(TextBuffer *buf, int key, Size size, bool enter_escapes) {
  if (terminal_state(TK_CONTROL))
    return false;
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
    std::string first_half = buf->buffer.substr(0, buf->caret_pos);
    std::string second_half = buf->buffer.substr(buf->caret_pos, buf->buffer.length() + 1 - buf->caret_pos);
    buf->buffer = first_half + "\n" + second_half;
    buf->moveCaret(Direction::Right, size);
    buf->cached_x_pos = buf->getCaretPos().x;
  }
  else if (key == TK_TAB) {
// No distinction between confirmation and interruption
    std::string first_half = buf->buffer.substr(0, buf->caret_pos);
    std::string second_half = buf->buffer.substr(buf->caret_pos, buf->buffer.length() + 1 - buf->caret_pos);
    buf->buffer = first_half + "\t" + second_half;
    buf->moveCaret(Direction::Right, size);
    buf->cached_x_pos = buf->getCaretPos().x;
  }
  else if (key == TK_BACKSPACE && buf->buffer.length() > 0) {
// Remove one character
    std::string first_half = buf->buffer.substr(0, max(0, buf->caret_pos-1));
    std::string second_half = buf->buffer.substr(buf->caret_pos, buf->buffer.length() + 1 - buf->caret_pos);
    buf->buffer = first_half + second_half;
    buf->moveCaret(Direction::Left, size);
    buf->cached_x_pos = buf->getCaretPos().x;
  }
  else if (terminal_check(TK_WCHAR)) {
// Append one character
    std::string first_half = buf->buffer.substr(0, buf->caret_pos);
    std::string second_half = buf->buffer.substr(buf->caret_pos, buf->buffer.length() + 1 - buf->caret_pos);
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
  int y = 0;
  int x = 0;
  for (char c : buf->buffer) {
    if (c != '\n' && c != '\t' && x + buf->offs.x < size.width && x + buf->offs.x >= 0 && y + buf->offs.y < size.height && y + buf->offs.y >= 0)
      terminal_put(buf->pos.x + buf->offs.x + x, buf->pos.y + buf->offs.y + y, c);
    if (c == '\n') {
      y++;
      x = 0;
    }
    else if (c == '\t') {
      x += 8 - x % 8;
    }
    else x++;
  }

  terminal_color(colorPalette->workspaceDefaultColor.bg.c_str());
  terminal_bkcolor(colorPalette->workspaceDefaultColor.fg.c_str());
  Point caret_pos = buf->getCaretPos();
  char prev_char = terminal_pick(buf->pos.x + caret_pos.x + buf->offs.x, buf->pos.y + caret_pos.y + buf->offs.y);
  terminal_put(buf->pos.x + caret_pos.x + buf->offs.x, buf->pos.y + caret_pos.y + buf->offs.y, prev_char);

  terminal_color(colorPalette->workspaceDefaultColor.fg.c_str());
  terminal_bkcolor(colorPalette->workspaceDefaultColor.bg.c_str());
}

