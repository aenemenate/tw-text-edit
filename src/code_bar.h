#pragma once

#include <vector>
#include <string>

class TextBuffer;

class CodeBracket {
  std::string name;
  bool folded;
public:
  int beginLine, endLine;
  CodeBracket(int _beginLine, int _endLine) : beginLine(_beginLine),
	endLine(_endLine), folded(false) {}
  bool IsFolded() { return folded; }
  void Fold() { folded = !folded; }
};

class CodeBar {
  bool showing = false;
  std::vector<CodeBracket> brackets;
  void UpdateBlocks(TextBuffer *buf);
public:
  void Update(TextBuffer *buf);
  bool IsBlockAtLine(int lineNumber);
  bool IsBlockFolded(int lineNumber);
  int  GetNextLine(int lineNumber);
  void FoldBracket(int lineNumber);
  bool GetShowing() { return showing; }
};