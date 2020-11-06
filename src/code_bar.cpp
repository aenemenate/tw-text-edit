#include "code_bar.h"

#include "text_buffer.h"

void CodeBar::UpdateBlocks(TextBuffer *buf) {
  brackets.clear();
  int scope = 0;
  int beginLine, endLine;
  int curLine = 0;
  for (int i = 0; i < buf->buffer.length(); ++i) {
    if (buf->buffer[i] == '\n')
      ++curLine;
    if (buf->buffer[i] == '{') {
      if (scope == 0)
        beginLine = curLine;
      ++scope;
    }
    if (scope >= 1 && buf->buffer[i] == '}') {
      --scope;
      if (scope == 0) {
        endLine = curLine;
        if (endLine != beginLine)
	  brackets.push_back(CodeBracket(beginLine, endLine));
      }
    }
  }
}

void CodeBar::Update(TextBuffer *buf) {
  if (buf->name.find(".cpp", 0) != std::string::npos
  || (buf->name.find(".h", 0)   != std::string::npos 
     && buf->name.find(".html",0) == std::string::npos)
  || buf->name.find(".hpp", 0) != std::string::npos
  || buf->name.find(".cxx", 0)	!= std::string::npos
  || buf->name.find(".c", 0) != std::string::npos
  || buf->name.find(".js", 0) != std::string::npos) {
    showing = true;
    UpdateBlocks(buf);
  }
  else
    showing = false;
}

bool CodeBar::IsBlockAtLine(int lineNumber) {
  for (int i = 0; i < brackets.size(); ++i) {
    if (brackets[i].beginLine == lineNumber)
      return true;
  }
  return false;
}

bool CodeBar::IsBlockFolded(int lineNumber) {
  for (int i = 0; i < brackets.size(); ++i) {
    if (brackets[i].IsFolded() && brackets[i].beginLine == lineNumber)
      return true;
  }
  return false;
}

int CodeBar::GetNextLine(int lineNumber) {
  for (int i = i; i < brackets.size(); ++i) {
    if (brackets[i].IsFolded() && brackets[i].beginLine == lineNumber)
      return brackets[i].endLine;
  }
  return false;
}

void CodeBar::FoldBracket(int lineNumber) {
  for (int i = 0; i < brackets.size(); ++i) {
    if (brackets[i].beginLine == lineNumber)
      brackets[i].Fold();
  }
}