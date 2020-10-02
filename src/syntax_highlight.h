#pragma once
#include "../include/BearLibTerminal.h"
#include <vector>
#define vector std::vector

class TextBuffer;

struct Syntax {
  vector<color_t> bufVec;
  void updateSyntax(TextBuffer *buf, std::string filename);
};