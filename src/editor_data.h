#pragma once

#include "buffer_list.h"
#include "menu_bar.h"
#include "text_dropdown.h"
#include "context_menu.h"

struct EditorData {
  bool running;
  bool lineNums;
  int nextFile;
  std::string workingDirectory;
  BufferList buffers;
  MenuBar menuBar;
  TextDropdown textDropdown;
  ContextMenu  contextMenu;
};

void buildEditorData(EditorData *editorData);
