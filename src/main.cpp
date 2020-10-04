
#define _AMD64_

#include "../include/BearLibTerminal.h"

#include "editor_data.h"
#include "base_types.h"
#include "editor_actions.h"
#include "status_bar.h"

#include <windows.h>
#include <string>
#include <iostream>
#include <filesystem>

EditorData editorData;

void init(int argc, char *argv[]) {
  std::filesystem::path start_path = std::filesystem::current_path();
  buildEditorData(&editorData);
  std::filesystem::current_path(std::filesystem::path(_pgmptr).parent_path());
  terminal_open();
  terminal_set("window.size=80x32");
  terminal_refresh();
  std::filesystem::current_path(start_path);
  if (argc > 1)
  for (int i = 1; i < argc; ++i) {
    std::string arg = std::string{argv[i]};
    try {
      int line = std::stoi(arg,NULL);
      if (editorData.buffers.textBuffers.size() > 0) {
        int caret_pos = 0;
        int i;
        while (caret_pos = editorData.buffers.textBuffers.back().buffer.find("\n", i)) {
          ++i;
          if (editorData.buffers.textBuffers.back().getCaretPos(caret_pos).y + 1 == line)
            break;
        }
       editorData.buffers.textBuffers.back().caret_pos = caret_pos;
       editorData.buffers.textBuffers.back().caret_sel_pos = editorData.buffers.textBuffers.back().caret_pos;
       editorData.buffers.textBuffers.back().setOffs({80, 32-3}, false);
      }
    }
    catch(std::exception ex) {
      if (std::string{argv[1]} != "") {
        editorData.workingDirectory = start_path.string();
        OpenFile(arg, &editorData);
        if (std::string{argv[1]} == ".") {
	  editorData.textDropdown.showing = true;
	  editorData.textDropdown.action = TextAction::Open;
        }
      }
    }
  }
}

void handleInput(Size termSize) {
  int key = terminal_read();
  if (key == TK_CLOSE)
    editorData.running = false;
  if (!handleInputTextDropdown(&editorData, key, termSize))
    if (!handleInputMenuBar(&editorData, key, termSize))
      if (!handleInputContextMenu(&editorData, key))
        handleInputBufferList(&(editorData.buffers), key, {termSize.width, termSize.height - 1}, editorData.lineNums);
}

void update(Size termSize) {
}

void draw(Size termSize) {
  terminal_clear();
  drawBufferList(&(editorData.buffers), {termSize.width, termSize.height - 1}, editorData.menuBar.clicked_opt == -1, editorData.lineNums);
  drawMenuBar(&(editorData.menuBar), termSize);
  drawTextDropdown(&editorData, termSize);
  drawContextMenu(&(editorData.contextMenu));
  drawStatusBar(&editorData, termSize);
  terminal_refresh();
}

int main(int argc, char *argv[]) {
  FreeConsole();

  init(argc, argv);
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