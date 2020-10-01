#pragma once

#include "buffer_list.h"
#include "menu_bar.h"
#include "text_dropdown.h"

struct EditorData {
  bool running;
  bool lineNums;
  int nextFile;
  std::string workingDirectory;
  BufferList buffers;
  MenuBar menuBar;
  TextDropdown textDropdown;
};

void buildEditorData(EditorData *editorData);
