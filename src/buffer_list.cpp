#include "buffer_list.h"
#include "color_palette.h"
#include "../include/BearLibTerminal.h"

void buildBufferList(BufferList *bufferList) {
  bufferList->cur = 0;
}

void handleInputBufferList(BufferList *bufferList, int key, Size termSize) {
  if (bufferList->textBuffers.empty())
    return;
// if buffer clicked, switch to it
  bool mouse_y_on_bar = terminal_state(TK_MOUSE_Y) == 1;
  int mouse_x = terminal_state(TK_MOUSE_X);
  int cur_x = 0;
  if (key == (TK_MOUSE_LEFT|TK_KEY_RELEASED)) {
    for (int i = 0; i < bufferList->textBuffers.size(); ++i) {
      if (mouse_y_on_bar && mouse_x >= cur_x && mouse_x < cur_x + bufferList->textBuffers[i].name.length()) {
        bufferList->cur = i;
        return;
      }
      cur_x += bufferList->textBuffers[i].name.length() + 1;
    }
  }
// if buffer clicked, go to it
  TextBuffer *currentBuffer = &(bufferList->textBuffers[bufferList->cur]);
  handleInputTextBuffer(currentBuffer, key, {termSize.width, termSize.height - currentBuffer->pos.y}, false);
}

void drawBufferList(BufferList *bufferList, ColorPalette *colorPalette, Size termSize) {
  if (bufferList->textBuffers.empty())
    return;
// draw cur buffer from y 2
  TextBuffer *currentBuffer = &(bufferList->textBuffers[bufferList->cur]);
  drawTextBuffer(currentBuffer, colorPalette, {termSize.width, termSize.height - currentBuffer->pos.y});
// draw buffer list at y 1
  terminal_color(colorPalette->bufferListColor.fg.c_str());
  terminal_bkcolor(colorPalette->bufferListColor.bg.c_str());
// bar bg
  for (int i = 0; i < termSize.width; ++i)
    terminal_put(i, 1, ' ');
// options
  int cur_x = 0;
  bool mouse_y_on_bar = terminal_state(TK_MOUSE_Y) == 1;
  int mouse_x = terminal_state(TK_MOUSE_X);
  for (int i = 0; i < bufferList->textBuffers.size(); ++i) {
    if (mouse_y_on_bar && mouse_x >= cur_x && mouse_x < cur_x + bufferList->textBuffers[i].name.length()) {
      terminal_bkcolor(colorPalette->bufferListHighlightedColor.bg.c_str());
      terminal_color(colorPalette->bufferListHighlightedColor.fg.c_str());
    }
    if (bufferList->cur == i) {
      terminal_bkcolor(colorPalette->bufferListActiveColor.bg.c_str());
      terminal_color(colorPalette->bufferListActiveColor.fg.c_str());
    }
    cur_x += terminal_print(cur_x, 1, bufferList->textBuffers[i].name.c_str()).width + 1;
    terminal_color(colorPalette->bufferListColor.fg.c_str());
    terminal_bkcolor(colorPalette->bufferListColor.bg.c_str());
  }
}
