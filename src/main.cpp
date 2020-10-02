
#define _AMD64_

#include "../include/BearLibTerminal.h"

#include <windows.h>
#include <string>
#include <iostream>
#include <filesystem>

#include "editor_data.h"
#include "base_types.h"
#include "editor_actions.h"

EditorData editorData;

void init(std::string fileName) {
  buildEditorData(&editorData);
  if (fileName != "") {
    OpenFile(std::filesystem::current_path().string() + fileName, &editorData);
  }
  std::filesystem::current_path(std::filesystem::path(_pgmptr).parent_path());
  terminal_open();
  terminal_set("window.size=60x40");
  terminal_refresh();
}

void handleInput(Size termSize) {
  int key = terminal_read();
  if (key == TK_CLOSE)
    editorData.running = false;
  if (!handleInputMenuBar(&editorData, key, termSize)
  &&  !handleInputTextDropdown(&editorData, key, termSize))
    handleInputBufferList(&(editorData.buffers), key, termSize, editorData.lineNums);
}

void update(Size termSize) {
}

void draw(Size termSize) {
  terminal_clear();
  drawBufferList(&(editorData.buffers), {termSize.width, termSize.height}, editorData.menuBar.clicked_opt == -1, editorData.lineNums);
  drawMenuBar(&(editorData.menuBar), termSize);
  drawTextDropdown(&(editorData.textDropdown), &(editorData.workingDirectory), termSize);
  terminal_refresh();
}

int main(int argc, char *argv[]) {
  FreeConsole();

// get command line arg, arg 1 is filename to load
  std::string filename = "";
  if (argc > 1)
    filename = argv[1];

  init(filename);
  Size termSize = {terminal_state(TK_WIDTH),terminal_state(TK_HEIGHT)};
  draw(termSize);
  filename.clear();
  while ((editorData.running)) {
    if (terminal_state(TK_WIDTH) != termSize.width
    ||  terminal_state(TK_HEIGHT) != termSize.height) {
      termSize = {terminal_state(TK_WIDTH),terminal_state(TK_HEIGHT)};
      bool showing = editorData.textDropdown.showing;
      buildTextDropdown(&(editorData.textDropdown), termSize);
      editorData.textDropdown.showing = showing;
    }
    update(termSize);
    draw(termSize);
    handleInput(termSize);
  }
  terminal_close();
  return 0;
}