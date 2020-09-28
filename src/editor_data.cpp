#include "editor_data.h"
#include "base_types.h"
#include "../include/BearLibTerminal.h"

void buildEditorData(EditorData *editorData) {
  Size termSize = {terminal_state(TK_WIDTH),terminal_state(TK_HEIGHT)};
  editorData->running = true;
  editorData->nextFile = 2;
  editorData->workingDirectory = "C:";
  buildMenuBar(&(editorData->menuBar));
  buildBufferList(&(editorData->buffers));
  buildColorPalette(&(editorData->colorPalette));
  buildTextDropdown(&(editorData->textDropdown), termSize);
}