#pragma once
#include "../include/BearLibTerminal.h"
#include <vector>

class TextBuffer;

struct Syntax {
  std::vector<color_t> bufVec;
  void updateSyntax(TextBuffer *buf, std::string filename);
};
