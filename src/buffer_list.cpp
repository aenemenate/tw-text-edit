#include "buffer_list.h"
#include "../include/BearLibTerminal.h"

void buildBufferList(BufferList *bufferList) {
  bufferList->cur = 0;
}

void handleInputBufferList(BufferList *bufferList, int key, Size termSize, bool lineNums) {
  if (bufferList->textBuffers.empty())
    return;
// if buffer clicked, switch to it
  bool mouse_y_on_bar = terminal_state(TK_MOUSE_Y) == 1;
  int mouse_x = terminal_state(TK_MOUSE_X);
  int cur_x = 0;
  if (key == (TK_MOUSE_LEFT|TK_KEY_RELEASED)) {
    for (int i = 0; i < bufferList->textBuffers.size(); ++i) {
    std::string bufferName = bufferList->textBuffers[i].filePath != "" ?
                             ((bufferList->textBuffers[i].isDirty ? "* " : "") 
                             + bufferList->textBuffers[i].name) :
                             ("%" + bufferList->textBuffers[i].name + "%");      if (mouse_y_on_bar && mouse_x >= cur_x && mouse_x < cur_x + bufferName.length()) {
        bufferList->cur = i;
        return;
      }
      cur_x += bufferName.length() + 1;
    }
  }
  TextBuffer *currentBuffer = &(bufferList->textBuffers[bufferList->cur]);
  handleInputTextBuffer(currentBuffer, key, {termSize.width, termSize.height - currentBuffer->pos.y}, false, lineNums);
}

void drawBufferList(BufferList *bufferList, Size termSize, bool highlight, bool lineNums) {
  if (bufferList->textBuffers.empty())
    return;
// draw cur buffer from y 2
  TextBuffer *currentBuffer = &(bufferList->textBuffers[bufferList->cur]);
  drawTextBuffer(currentBuffer, {termSize.width, termSize.height - currentBuffer->pos.y}, lineNums);
// draw buffer list at y 1
  terminal_color(color_from_name("bufferlistfg"));
  terminal_bkcolor(color_from_name("bufferlistbk"));
// bar bg
  for (int i = 0; i < termSize.width; ++i)
    terminal_put(i, 1, ' ');
// options
  int cur_x = 0;
  bool mouse_y_on_bar = terminal_state(TK_MOUSE_Y) == 1;
  int mouse_x = terminal_state(TK_MOUSE_X);
  for (int i = 0; i < bufferList->textBuffers.size(); ++i) {
    std::string bufferName = bufferList->textBuffers[i].filePath != "" ?
                             ((bufferList->textBuffers[i].isDirty ? "* " : "") 
                             + bufferList->textBuffers[i].name) :
                             ("%" + bufferList->textBuffers[i].name + "%");
    if (highlight && mouse_y_on_bar && mouse_x >= cur_x 
    &&  mouse_x < cur_x + bufferName.length()) {
      terminal_color(color_from_name("bufferlisthlfg"));
      terminal_bkcolor(color_from_name("bufferlisthlbk"));
    }
    if (bufferList->cur == i) {
      terminal_color(color_from_name("bufferlistactfg"));
      terminal_bkcolor(color_from_name("bufferlistactbk"));
    }
    cur_x += terminal_print(cur_x, 1, bufferName.c_str()).width + 1;
    terminal_color(color_from_name("bufferlistfg"));
    terminal_bkcolor(color_from_name("bufferlistbk"));
  }
}
