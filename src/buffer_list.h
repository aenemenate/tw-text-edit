#pragma once
#include "text_buffer.h"
#include <vector>

// BUFFER LIST

struct BufferList {
  int cur;
  std::vector<TextBuffer> textBuffers;
};

void buildBufferList(BufferList *bufferList);

void handleInputBufferList(BufferList *bufferList, int key, Size termSize, bool lineNums);

void drawBufferList(BufferList *bufferList, Size termSize, bool highlight, bool lineNums);
