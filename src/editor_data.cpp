#include "editor_data.h"
#include "base_types.h"
#include "../include/BearLibTerminal.h"

void buildEditorData(EditorData *editorData) {
  Size termSize = {terminal_state(TK_WIDTH),terminal_state(TK_HEIGHT)};
  if (termSize.width == 0 && termSize.height == 0) {
    termSize = {60,40};
  }
  editorData->running = true;
  editorData->lineNums = false;
  editorData->nextFile = 2;
  editorData->workingDirectory = "C:";
  buildMenuBar(&(editorData->menuBar));
  buildBufferList(&(editorData->buffers));
  buildColorPalette(&(editorData->colorPalette));
  buildTextDropdown(&(editorData->textDropdown), termSize);
}