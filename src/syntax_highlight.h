#pragma once

#include <vector>
#include <string>

#include "../include/BearLibTerminal.h"

class TextBuffer;

class Syntax {
  void highlightMultiLineComments(char openDelim, char closedDelim, TextBuffer *buf);
  void highlightSingleLineComments(TextBuffer *buf);
  void highlightStrings(char delim, TextBuffer *buf, char delimVariant = ' ');
  void highlightCharacters(TextBuffer *buf);
  void highlightKeywords(std::vector<std::string> keywords, std::vector<char> allowedBefore, 
			 std::vector<char> allowedAfter, std::string color, TextBuffer *buf);
  void highlightUnclosedBrackets(std::vector<char> opens, std::vector<char> closeds, TextBuffer *buf);

  void highlightCpp(TextBuffer *buf);
  void highlightOcaml(TextBuffer *buf);
  void highlightJavascript(TextBuffer *buf);
public:
  std::vector<color_t> bufVec;
  void updateSyntax(TextBuffer *buf, std::string filename);
};
