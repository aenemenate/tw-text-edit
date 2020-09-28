#pragma once

#include "color_palette.h"
#include "buffer_list.h"
#include "menu_bar.h"
#include "text_dropdown.h"

struct EditorData {
  bool running;
  int nextFile;
  std::string workingDirectory;
  ColorPalette colorPalette;
  BufferList buffers;
  MenuBar menuBar;
  TextDropdown textDropdown;
};

void buildEditorData(EditorData *editorData);
