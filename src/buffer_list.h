#pragma once
#include "text_buffer.h"
#include <vector>
#define vector std::vector

class ColorPalette;

// BUFFER LIST

struct BufferList {
  int cur;
  vector<TextBuffer> textBuffers;
};

void buildBufferList(BufferList *bufferList);

void handleInputBufferList(BufferList *bufferList, int key, Size termSize, bool lineNums);

void drawBufferList(BufferList *bufferList, ColorPalette *colorPalette, Size termSize, bool highlight, bool lineNums);