
#define _AMD64_

#include <BearLibTerminal.h>

#include "editor_data.h"
#include "base_types.h"
#include "editor_actions.h"
#include "status_bar.h"

#include <string>
#include <iostream>

#include "util/filesystem.h"

#if defined(_WIN32) || defined(_WIN64)
  #include <windows.h>
#elif defined(linux)
  #include <libgen.h>         // dirname
  #include <unistd.h>         // readlink
  #include <linux/limits.h>   // PATH_MAX
#endif

EditorData editorData;

void init(int argc, char *argv[]) {
#if defined(_WIN32) || defined(_WIN64)
  FreeConsole();
#endif
  fs::path startPath = fs::current_path();
  buildEditorData(&editorData);
#if defined(_WIN32) || defined(_WIN64)
  fs::current_path(fs::path(_pgmptr).parent_path());
#elif defined(linux)
  char result[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
  const char *path;
  if (count != -1)
    path = dirname(result);
  fs::current_path(fs::path(std::string(path)));
#endif
  terminal_open();
  terminal_set("window.size=80x32");
  terminal_refresh();
  fs::current_path(startPath);
  if (argc > 1)
  for (int i = 1; i < argc; ++i) {
    std::string arg = std::string{argv[i]};
    try {
      int line = std::stoi(arg,NULL);
      if (editorData.buffers.textBuffers.size() > 0) {
        int caretPos = 0;
        int i;
        while (caretPos = editorData.buffers.textBuffers.back().buffer.find("\n", i)) {
          ++i;
          if (editorData.buffers.textBuffers.back().getCaretPos(caretPos).y + 1 == line)
            break;
        }
 
      editorData.buffers.textBuffers.back().caretPos = caretPos;
 
      editorData.buffers.textBuffers.back().caretSelPos = editorData.buffers.textBuffers.back().caretPos;
 
      editorData.buffers.textBuffers.back().setOffs({80, 32-3}, false);
      }
    }

    catch(std::exception ex) {
      if (std::string{argv[1]} != "") {
        editorData.workingDirectory = startPath.string();
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
