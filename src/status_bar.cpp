#include "status_bar.h"
#include "editor_data.h"
#include <BearLibTerminal.h>
#include <iostream>
#include <ctime>
#include <string>
#include <algorithm>
#include <cctype>

std::string getTime() {
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];

  time (&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer,sizeof(buffer),"%d-%m-%Y %H:%M",timeinfo);
  std::string str(buffer);
  return str;
}

void drawStatusBar(EditorData* editorData, Size termSize) {
  terminal_color(color_from_name("dark workspacedefaultfg"));
  terminal_bkcolor(color_from_name("dark workspacedefaultbk"));
// clear area
  for (int i = 0; i < termSize.width; ++i) {
    terminal_put(i, termSize.height - 1, ' ');
  }
  if (editorData->buffers.textBuffers.size() > 0) {
    Point cursor_pos = editorData->buffers.textBuffers[editorData->buffers.cur].getCaretPos(editorData->buffers.textBuffers[editorData->buffers.cur].caretPos);
    std::string cursor_pos_str = std::to_string(cursor_pos.x + 1) + "," + std::to_string(cursor_pos.y + 1);
    terminal_print(0, termSize.height - 1, cursor_pos_str.c_str());
    int start_x = cursor_pos_str.length() + 1;
// print file path
    terminal_print(start_x, termSize.height - 1, editorData->buffers.textBuffers[editorData->buffers.cur].filePath.c_str());
// calculate starting position of lang
    start_x += editorData->buffers.textBuffers[editorData->buffers.cur].filePath.length();
    start_x += 8 - start_x % 8;
// get file ext
    std::string filename = editorData->buffers.textBuffers[editorData->buffers.cur].name;
    int dot_ind = filename.find('.', 0)+1;
    std::string ext = filename.substr(dot_ind, filename.length() - dot_ind);
    std::transform(filename.begin(), filename.end(), filename.begin(),
      [](unsigned char c){ return std::tolower(c); });
// set string according to extension
    if (ext == "cpp" || ext == "h" || ext == "hpp" || ext == "cxx")
      filename = "C++";
    else if (ext == "ini")
      filename = "Ini Config File";
    else if (ext == "bat" || ext == "cmd")
      filename = "Batch File";
    else if (filename == "makefile")
      filename = "Makefile";
    else
      filename = "Plaintext";
// print lang
    terminal_print(start_x, termSize.height - 1, filename.c_str());
    start_x += filename.length();
    start_x += 8 - start_x % 8;
    std::string findText = editorData->buffers.textBuffers[editorData->buffers.cur].findText;
    if (findText != "")
      terminal_print(start_x, termSize.height - 1, std::string{"Finding: \'" + findText + "\'"}.c_str());
  }
// print time
  std::string time{getTime()};
  terminal_print(termSize.width - 1 - time.length(), termSize.height - 1, time.c_str());
}
