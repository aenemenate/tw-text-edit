#include "code_bar.h"

#include "text_buffer.h"

bool CodeBar::BracketOfNameExists(std::string name) {
  for (int i = 0; i < brackets.size(); ++i)
    if (brackets[i].GetName() == name)
      return true;
  return false;
}

bool CodeBar::BracketOfPositionExists(int beginLine) {
  for (int i = 0; i < brackets.size(); ++i)
    if (brackets[i].beginLine == beginLine)
      return true;
  return false;
}

bool CodeBar::BracketAtPositionIsSameLength(int beginLine, int endLine) {
  for (int i = 0; i < brackets.size(); ++i)
    if (brackets[i].beginLine == beginLine) {
      if (brackets[i].endLine == endLine)
        return true;
    }
  return false;
}

bool CodeBar::DeleteBracketAtPosition(int beginLine) {
  bool folded = false;
  std::vector<CodeBracket> _brackets;
  for (int i = 0; i < brackets.size(); ++i) {
    if (brackets[i].beginLine != beginLine)
      _brackets.push_back(brackets[i]);
    else if (brackets[i].IsFolded())
      folded = true;
  }
  brackets = _brackets;
  return folded;
}

bool CodeBar::DeleteBracketAtName(std::string name) {
  bool folded = false;
  std::vector<CodeBracket> _brackets;
  for (int i = 0; i < brackets.size(); ++i) {
    if (brackets[i].GetName() != name)
      _brackets.push_back(brackets[i]);
    else if (brackets[i].IsFolded())
      folded = true;
  }
  brackets = _brackets;
  return folded;
}

void CodeBar::UpdateBlocks(TextBuffer *buf) {
  int scope = 0;
  std::string name;
  int beginLine, endLine;
  int curLine = 0;
  int lastReturnIndex = 0;
  for (int i = 0; i < buf->buffer.length(); ++i) {
    if (buf->buffer[i] == '\n') {
      lastReturnIndex = i;
      ++curLine;
    }
    if (buf->buffer[i] == '{') {
      if (scope == 0) {
	name = buf->buffer.substr(lastReturnIndex, i - lastReturnIndex);
	beginLine = curLine;
      }
      ++scope;
    }
    if (scope >= 1 && buf->buffer[i] == '}') {
      --scope;
      if (scope == 0) {
        endLine = curLine;
	if (endLine != beginLine) { 
	  if (!BracketOfNameExists(name) && !BracketOfPositionExists(beginLine))
	    brackets.push_back(CodeBracket(beginLine, endLine, name));
	  else if (BracketOfPositionExists(beginLine)) {
	    if (!BracketOfNameExists(name)) {
	      bool folded = DeleteBracketAtPosition(beginLine);
	      brackets.push_back(CodeBracket(beginLine, endLine, name));
	      if (folded) brackets[brackets.size()-1].Fold();
	    }
	    else if (!BracketAtPositionIsSameLength(beginLine, endLine)) {
	      DeleteBracketAtPosition(beginLine);
	      brackets.push_back(CodeBracket(beginLine, endLine, name));
	    }
	  }
	  else if (!BracketOfPositionExists(beginLine) && BracketOfNameExists(name)) {
	    bool folded = DeleteBracketAtName(name);
	    brackets.push_back(CodeBracket(beginLine, endLine, name));
	    if (folded) brackets[brackets.size()-1].Fold();
	  }
	}
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