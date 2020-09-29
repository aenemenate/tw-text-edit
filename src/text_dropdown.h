#pragma once

#include "text_buffer.h"

class ColorPalette;
class EditorData;

enum TextAction {
  Open,
  Save,
  Find,
  FindAndReplace
};

struct TextDropdown {
  TextBuffer inputBuffer;
  TextAction action;
  bool showing;
};

void buildTextDropdown(TextDropdown *textDropdown, Size termSize);

void resetTextDropdown(TextDropdown *textDropdown);

void drawTextDropdown(TextDropdown *textDropdown, std::string *workingDirectory, ColorPalette *colorPalette, Size termSize);

bool handleInputTextDropdown(EditorData *editorData, int key, Size termSize);