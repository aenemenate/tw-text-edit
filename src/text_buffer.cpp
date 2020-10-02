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

void caretSeek(int amount, TextBuffer *textBuffer) {
  int caret_pos = textBuffer->caret_pos;
  for (int i = min(caret_pos, caret_pos+amount); 
       i < max(caret_pos, caret_pos + amount); 
       ++textBuffer->caret_pos) {
    if (textBuffer->buffer[textBuffer->caret_pos] == '\t') {
      i += 8 - textBuffer->getCaretPos(textBuffer->caret_pos).x % 8;
    }
    else ++i;
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

void TextBuffer::findNext(Size size, bool lineNums) {
  if (find_text == "")
    return;
  int i = 1;
  while (caret_pos + i < buffer.length()) {
    if (buffer.substr(caret_pos + i, find_text.length()) == find_text
    &&  caret_pos + i != caret_pos)
      break;
    ++i;
  }
  if (caret_pos + i < buffer.length())
    caret_pos += i;
  caret_sel_pos = caret_pos;
  caret_sel_pos += find_text.length();
  setOffs(size, lineNums);
}

void TextBuffer::findPrev(Size size, bool lineNums) {
  if (find_text == "")
    return;
  int i = -1;
  while (caret_pos + i >= 0) {
    if (buffer.substr(caret_pos + i, find_text.length()) == find_text
    &&  caret_pos + i != caret_pos)
      break;
    --i;
  }
  if (caret_pos + i >= 0)
    caret_pos += i;
  caret_sel_pos = caret_pos;
  caret_sel_pos += find_text.length();
  setOffs(size, lineNums);
}

void TextBuffer::setOffs(Size size, bool lineNums) {
  Point temp = getCaretPos(caret_pos);
  Point temp2 = getCaretPos(caret_sel_pos);
  if (temp.x < std::abs(this->offs.x))
    this->offs.x = -temp.x;
  if (temp.x >= std::abs(this->offs.x) + size.width - (lineNums ? 4 : 0))
    this->offs.x = - (temp.x - size.width + (lineNums ? 4 : 0))-1;
  if (temp.y < std::abs(this->offs.y))
    this->offs.y = -temp.y;
  if (temp.y >= std::abs(this->offs.y) + size.height)
    this->offs.y = - (temp.y - size.height)-1;
  if (temp2.x < std::abs(this->offs.x))
    this->offs.x = -temp2.x;
  if (temp2.x >= std::abs(this->offs.x) + size.width - (lineNums ? 4 : 0))
    this->offs.x = - (temp2.x - size.width + (lineNums ? 4 : 0))-1;
  if (temp2.y < std::abs(this->offs.y))
    this->offs.y = -temp2.y;
  if (temp2.y >= std::abs(this->offs.y) + size.height)
    this->offs.y = - (temp2.y - size.height)-1;
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
  if (!shiftSelect)
    caret_sel_pos = caret_pos;
  setOffs(size, lineNums);
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
  textBuffer.caret_pos 	= 0;
  textBuffer.
    caret_sel_pos 	= 0;
  textBuffer.buffer	= buffer;
  textBuffer.find_text   = "";
  return textBuffer;
}

std::vector<color_t> bufVec;

// "syntax highlighting"
void updateBufVec(TextBuffer *buf) {
  bufVec.resize(buf->buffer.length());
  int ix = 0;
  color_t fg_color = color_from_name("workspacedefaultfg");
  for (auto c : buf->buffer) {
    bufVec[ix] = fg_color;
    ++ix;
  }
  if (buf->name.find(".cpp", 0) != std::string::npos 
  ||  buf->name.find(".h", 0)   != std::string::npos 
  ||  buf->name.find(".hpp", 0) != std::string::npos
  ||  buf->name.find(".cxx", 0)	!= std::string::npos) {
  // prepocessor directives
  std::vector<std::string> keywords = { "#include", "#pragma once", "#pragma" };
  for (int i = 0; i < keywords.size(); ++i) {
    size_t c = -1;
    while (buf->buffer.find(keywords[i], c+1) != std::string::npos) {
      c = buf->buffer.find(keywords[i], c+1);
      for (int j = c; j < c + keywords[i].length(); ++j) {
        bufVec[j] = color_from_name("cppdirectives");
      }
    }
  }
// purple (actions)
  keywords = { 
    "#ifndef", "for", "#define", "#endif", "if", "continue", "break", 
    "using",  "while", "static", "do", "goto", "throw", "return", 
    "else", "switch", "case", "try", "catch", "delete", "default", 
    "new" };
  for (int i = 0; i < keywords.size(); ++i) {
    std::vector<char> check_chars_end = {'(', ' ', '\n', '\t', ';', '{', '}', ':'};
    std::vector<char> check_chars_front = {' ', '\n', '\t', ';', '{', '}'};
    size_t c = -1;
    while (buf->buffer.find(keywords[i], c+1) != std::string::npos) {
      c = buf->buffer.find(keywords[i], c+1);
      if ((c+keywords[i].length() >= buf->buffer.length() || std::count(check_chars_end.begin(), check_chars_end.end(), buf->buffer[c+keywords[i].length()]))
      &&  (c-1 < 0 || std::count(check_chars_front.begin(), check_chars_front.end(), buf->buffer[c-1]))) {
        for (int j = c; j < c + keywords[i].length(); ++j)
          bufVec[j] = color_from_name("cppstatements");
      }
    }
  }
// blue (data)
  keywords = { 
    "sizeof", "int", "bool", "enum", "static_cast", "template", 
    "typedef", "typeid", "typename", "mutable", "namespace", "static", 
    "dynamic_cast", "struct", "inline", "volatile", "char", "union", 
    "extern", "class", "auto", "this", "long", "double", "float","const", 
    "unsigned", "protected", "private", "public", "virtual", "void", 
    "NULL", "nullptr", "true", "false", "friend" };
  for (int i = 0; i < keywords.size(); ++i) {
    std::vector<char> check_chars_end = {'>', '<', '=', ')', '(', '[', '-', '{', ' ', ';', '.', '&', ':', ',', '\n', '\t'};
    std::vector<char> check_chars_front = {'<', '=', ' ', '(', ')', ',', ';', '{', '*', '\n', '\t'};
    size_t c = -1;
    while (buf->buffer.find(keywords[i], c+1) != std::string::npos) {
      c = buf->buffer.find(keywords[i], c+1);
      if ((c+keywords[i].length() >= buf->buffer.length() || std::count(check_chars_end.begin(), check_chars_end.end(), buf->buffer[c+keywords[i].length()]))
      &&  (c-1 < 0 || std::count(check_chars_front.begin(), check_chars_front.end(), buf->buffer[c-1]))) {
        for (int j = c; j < c + keywords[i].length(); ++j) {
          bufVec[j] = color_from_name("cppdefinitions");
        }
      }
    }
  }

// mathematical symbols
  keywords = { 
    "{", "}", "(", ")", "*", "&", "/", "+", "-", "=", "!", ">", "<", 
    ":", ",", "." 
  };
  for (int i = 0; i < keywords.size(); ++i) {
    size_t c = -1;
    while (buf->buffer.find(keywords[i], c+1) != std::string::npos) {
      c = buf->buffer.find(keywords[i], c+1);
      for (int j = c; j < c + keywords[i].length(); ++j) {
        bufVec[j] = color_from_name("cppsymbols");
      }
    }
  }

// yellow: chars
  ix = 0;
  bool toggleChar = false;
  for (auto c : buf->buffer) {
    fg_color = bufVec[ix];
    if (c == '\'' && !(buf->buffer[ix-1] == '\\' && buf->buffer[ix-2] != '\\')) {
      toggleChar = !toggleChar;
      fg_color = color_from_name("cppchars");
    }
    if (toggleChar)
      fg_color = color_from_name("cppchars");
    bufVec[ix] = {fg_color};
    ++ix;
  }
// green: comments, strings
  ix = 0;
  bool toggleString = false, toggleComment = false, toggleMultiLineComment = false;
  for (auto c : buf->buffer) {
    fg_color = bufVec[ix];
    if (c == '\"' && !(buf->buffer[ix-1] == '\\' && buf->buffer[ix-2] != '\\')) {
      toggleString = !toggleString;
      fg_color = color_from_name("cppstrcomments");
    }
    else if (c == '/' && ix < buf->buffer.length() - 1 && buf->buffer[ix+1] == '/' && toggleMultiLineComment == false)
      toggleComment = true;
    else if (c == '/' && ix < buf->buffer.length() - 1 && buf->buffer[ix+1] == '*')
      toggleMultiLineComment = true;
    if (toggleString)
      fg_color = color_from_name("cppstrcomments");
    if (toggleComment || toggleMultiLineComment) {
      if ((toggleComment && ix < buf->buffer.length() - 1 && buf->buffer[ix+1] == '\n') 
      || (toggleMultiLineComment && c == '/' && ix > 0 && buf->buffer[ix-1] == '*')) {  
        toggleComment = false;
        toggleMultiLineComment = false;
      }
      fg_color = color_from_name("cppstrcomments");
    }
    bufVec[ix] = fg_color;
    ++ix;
  }
  }
}

void ctrlright(TextBuffer *buf) {
  if (buf->caret_pos < buf->buffer.size() - 1) {
    std::vector<int> places;
    std::vector<char> keychars = {
      ' ', '\\', '/', '<', '>', '.', ',', '{', '}', '(', ')', '\"', ';', '=', '-', '+', 
      '|', '&', '*', '!', '%', '[', ']', ':', '\n', '\t'
    };
    for (char c : keychars)
      for (int i = buf->caret_pos+1; i < buf->buffer.length(); ++i)
        if (i == buf->buffer.length() - 1 || isalpha(buf->buffer[i]) && std::count(keychars.begin(), keychars.end(), buf->buffer[i-1])) {
          places.push_back(i);
          break;
        }
    int orig_pos = buf->caret_pos;
    for (int i : places)
      if (i < buf->caret_pos || buf->caret_pos == orig_pos)
        buf->caret_pos = i;
    if (!terminal_state(TK_SHIFT))
      buf->caret_sel_pos = buf->caret_pos;
  }
}

void ctrlleft(TextBuffer *buf) {
  if (buf->caret_pos > 0) {
    std::vector<int> places;
    std::vector<char> keychars = {
      ' ', '\\', '/', '<', '>', '.', ',', '{', '}', '(', ')', '\"', ';', '=', '-', '+', 
      '|', '&', '*', '!', '%', '[', ']', ':', '\n', '\t'
    };
    for (char c : keychars)
      for (int i = buf->caret_pos-1; i >= 0; --i)
        if (i == 0 || isalpha(buf->buffer[i]) && std::count(keychars.begin(), keychars.end(), buf->buffer[i-1])) {
          places.push_back(i);
          break;
        }
    int orig_pos = buf->caret_pos;
    for (int i : places)
      if (i > buf->caret_pos || buf->caret_pos == orig_pos)
        buf->caret_pos = i;
    if (!terminal_state(TK_SHIFT)) {
      buf->caret_sel_pos = buf->caret_pos;
    }
  }
}

void ctrlup(TextBuffer *buf) {
  int i = -1;
  while (buf->caret_pos + i >= 0) {
    if (buf->buffer.substr(buf->caret_pos + i, 2) == "\n\n"
    &&  buf->caret_pos + i + 1 != buf->caret_pos)
      break;
    --i;
  }
  if (buf->caret_pos + i < 0)
    buf->caret_pos = 0;
  else
    buf->caret_pos += i + 1;
  if (!terminal_state(TK_SHIFT))
    buf->caret_sel_pos = buf->caret_pos;
}

void ctrldown(TextBuffer *buf) {
  int i = 1;
  while (buf->caret_pos + i < buf->buffer.length()) {
    if (buf->buffer.substr(buf->caret_pos + i, 2) == "\n\n"
    &&  buf->caret_pos + i + 1 != buf->caret_pos)
      break;
    ++i;
  }
  if (buf->caret_pos + i >= buf->buffer.length()) 
       buf->caret_pos = buf->buffer.length();
  else buf->caret_pos += i + 1;
  if (!terminal_state(TK_SHIFT))
    buf->caret_sel_pos = buf->caret_pos;
}

bool handleInputTextBuffer(TextBuffer *buf, int key, Size size, bool enterEscapes, bool lineNums) {
  if (terminal_state(TK_CONTROL)) {
    if (key == TK_V) buf->paste(size, lineNums);
    if (key == TK_C) buf->copy(size, lineNums);
    if (key == TK_X) buf->cut(size, lineNums);
    if (key == TK_RIGHT || key == TK_LEFT
    ||  key == TK_DOWN  || key == TK_UP) {
      if (key == TK_RIGHT)
        ctrlright(buf);
      if (key == TK_LEFT)
        ctrlleft(buf);
      if (key == TK_DOWN)
	ctrldown(buf);
      if (key == TK_UP)
	ctrlup(buf);
      if (buf->caret_pos >= buf->buffer.length()) {
        if (buf->caret_pos == buf->caret_sel_pos)
          buf->caret_sel_pos = buf->buffer.length()-1;
        buf->caret_pos = buf->buffer.length()-1;
      }
      if (buf->caret_pos < 0) {
        if (buf->caret_pos == buf->caret_sel_pos)
          buf->caret_sel_pos = 0;
        buf->caret_pos = 0;
      }
      buf->setOffs(size, lineNums);
    }
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
	buf->insertChar('\n', size, lineNums);
	break;
      case (TK_TAB):
	buf->insertChar('\t', size, lineNums);
	break;
      case (TK_BACKSPACE):
	buf->backspace(size, lineNums);
	break;
      case (TK_HOME):
	buf->caret_pos = buf->findNewline(buf->getCaretPos(buf->caret_pos).y);
	if (!terminal_state(TK_SHIFT))
	  buf->caret_sel_pos = buf->caret_pos;
        buf->cached_x_pos = 0;
	buf->setOffs(size, lineNums);
	break;
      case (TK_END):
	buf->caret_pos = buf->findNewline(buf->getCaretPos(buf->caret_pos).y+1)-1;
	if (!terminal_state(TK_SHIFT))
	  buf->caret_sel_pos = buf->caret_pos;
	buf->cached_x_pos = buf->getCaretPos(buf->caret_pos).x;
        buf->setOffs(size, lineNums);
	break;
      case (TK_PAGEUP):
	buf->offs.y += size.height;
	if (buf->offs.y > 0) buf->offs.y = 0;
	break;
      case (TK_PAGEDOWN):
	buf->offs.y -= size.height;
	break;
      default:
        break;
  }
  if (terminal_check(TK_WCHAR))
    buf->insertChar((char)terminal_state(TK_WCHAR), size, lineNums);
  return false;
}

void drawTextBuffer(TextBuffer *buf, Size size, bool lineNums) {
  color_t term_color;
  color_t term_bkcolor;
  if (bufVec.size() != buf->buffer.length()) {
    updateBufVec(buf);
  }
  terminal_color(term_color = color_from_name("workspacedefaultfg"));
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
      color_t *color = &(bufVec[i]);
      if (((i >= buf->caret_sel_pos && i < buf->caret_pos) || (i >= buf->caret_pos && i < buf->caret_sel_pos)) 
      &&    buf->caret_pos != buf->caret_sel_pos) {
        terminal_bkcolor(term_bkcolor = color_from_name("workspacehlbk"));
      }
      if (i == buf->caret_pos && buf->caret_pos > buf->caret_sel_pos
      ||  i == buf->caret_sel_pos && buf->caret_sel_pos > buf->caret_pos)
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
  Point caret_pos = buf->getCaretPos(buf->caret_pos);
  char prev_char = terminal_pick((lineNums ? 4 : 0) + buf->pos.x + caret_pos.x + buf->offs.x, buf->pos.y + caret_pos.y + buf->offs.y);
  terminal_put((lineNums ? 4 : 0) + buf->pos.x + caret_pos.x + buf->offs.x, buf->pos.y + caret_pos.y + buf->offs.y, prev_char);

  terminal_color(color_from_name("workspacedefaultfg"));
  terminal_bkcolor(color_from_name("workspacedefaultbk"));
}