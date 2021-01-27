#include "editor_data.h"
#include "base_types.h"
#include <BearLibTerminal.h>

void buildEditorData(EditorData *editorData) {
  Size termSize = {terminal_state(TK_WIDTH),terminal_state(TK_HEIGHT)};
  if (termSize.width == 0 && termSize.height == 0) {
    termSize = {80, 32-3};
  }
  editorData->running = true;
  editorData->lineNums = false;
  editorData->nextFile = 1;
#if defined(_WIN32) || defined(_WIN64)
  editorData->workingDirectory = "C:";
#elif defined(linux)
  editorData->workingDirectory = "";
#endif
  buildMenuBar(&(editorData->menuBar));
  buildBufferList(&(editorData->buffers));
  buildTextDropdown(&(editorData->textDropdown), termSize);
  buildContextMenu(&(editorData->contextMenu), {0, 0});
}