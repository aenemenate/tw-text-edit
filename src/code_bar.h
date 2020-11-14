#pragma once

#include <vector>
#include <string>

class TextBuffer;

class CodeBracket {
  std::string name;
  bool folded;
public:
  int beginLine, endLine;
  CodeBracket(int _beginLine, int _endLine, std::string _name) : beginLine(_beginLine),
	endLine(_endLine), name(_name), folded(false) {}
  std::string GetName() { return name; }
  bool IsFolded() { return folded; }
  void Fold() { folded = !folded; }
};

class CodeBar {
  bool showing = false;
  std::vector<CodeBracket> brackets;
  void UpdateBlocks(TextBuffer *buf);
  bool DeleteBracketAtPosition(int beginLine);
  bool DeleteBracketAtName(std::string name);
  bool BracketOfNameExists(std::string name);
  bool BracketAtPositionIsSameLength(int beginLine, int endLine);
public:
  void Update(TextBuffer *buf);
  bool BracketOfPositionExists(int beginLine);
  bool IsBlockAtLine(int lineNumber);
  bool IsBlockFolded(int lineNumber);
  int  GetNextLine(int lineNumber);
  void FoldBracket(int lineNumber);
  void FoldAll();
  bool GetShowing() { return showing; }
};