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
public:
  void Update(TextBuffer *buf);
  bool BracketOfNameExists(std::string name);
  bool BracketOfPositionExists(int beginLine);
  void DeleteBracketAtPosition(int beginLine);
  void DeleteBracketAtName(std::string name);
  bool IsBlockAtLine(int lineNumber);
  bool IsBlockFolded(int lineNumber);
  int  GetNextLine(int lineNumber);
  void FoldBracket(int lineNumber);
  bool GetShowing() { return showing; }
};