
#define _AMD64_

#include "../include/BearLibTerminal.h"

#include <windows.h>
#include <string>
#include <iostream>

#include "editor_data.h"
#include "base_types.h"
#include "editor_actions.h"

EditorData editorData;

void init(std::string fileName) {
  terminal_open();
  terminal_set("window.size=60x40");
  terminal_refresh();
  buildEditorData(&editorData);
  if (fileName != "") {
    OpenFile(fileName, &editorData);
  }
}

void handleInput(Size termSize) {
  int key = terminal_read();
  if (key == TK_CLOSE)
    editorData.running = false;
  if (!handleInputMenuBar(&editorData, key, termSize) 
  &&  !handleInputTextDropdown(&editorData, key, termSize))
    handleInputBufferList(&(editorData.buffers), key, termSize);
}

void update(Size termSize) {
}

void draw(Size termSize) {
  terminal_clear();
  drawBufferList(&(editorData.buffers), &(editorData.colorPalette), termSize);
  drawMenuBar(&(editorData.menuBar), &(editorData.colorPalette), termSize);
  drawTextDropdown(&(editorData.textDropdown), editorData.workingDirectory + "\\", &(editorData.colorPalette), termSize);
  terminal_refresh();
}

int main(int argc, char *argv[]) {
  FreeConsole();
// get command line arg arg 1 is 
  std::string filename = "";
  if (argc > 1)
    filename = argv[1];

  init(filename);
  Size termSize = {terminal_state(TK_WIDTH),terminal_state(TK_HEIGHT)};
  draw(termSize);
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