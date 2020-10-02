#include "editor_actions.h"
#include "editor_data.h"
#include "../include/BearLibTerminal.h"

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

void buildContextMenu(ContextMenu *cMenu, Point pos) {
  cMenu->pos = pos;
  cMenu->options.push_back({"Cut  ", Cut});
  cMenu->options.push_back({"Copy ", Copy});
  cMenu->options.push_back({"Paste", Paste});
  cMenu->options.push_back({"Find ", StartFind});
  cMenu->showing = false;
}

void setContextMenuShowing(ContextMenu *cMenu, Point pos) {
  cMenu->showing = true;
  cMenu->pos = pos;
}

bool handleInputContextMenu(EditorData *editorData, int key) {
  if (key == (TK_MOUSE_RIGHT) && terminal_state(TK_MOUSE_Y) >= editorData->buffers.textBuffers[editorData->buffers.cur].pos.y
      && terminal_state(TK_MOUSE_Y) < terminal_state(TK_HEIGHT)
      && editorData->buffers.textBuffers.size() > 0)
    setContextMenuShowing(&(editorData->contextMenu), {
        min(terminal_state(TK_MOUSE_X), (int)(terminal_state(TK_WIDTH) - editorData->contextMenu.options[0].name.length())), 
        min(terminal_state(TK_MOUSE_Y), (int)(terminal_state(TK_HEIGHT) - editorData->contextMenu.options.size()))
      }
    );
  if (editorData->contextMenu.showing && key == TK_MOUSE_LEFT) {
    for (int j = 0; j < editorData->contextMenu.options.size(); ++j)
      if (terminal_state(TK_MOUSE_Y) == editorData->contextMenu.pos.y + j 
      &&  terminal_state(TK_MOUSE_X) >= editorData->contextMenu.pos.x 
      &&  terminal_state(TK_MOUSE_X) < editorData->contextMenu.pos.x + editorData->contextMenu.options[j].name.length()
      &&  editorData->contextMenu.options[j].callback_f != nullptr) {
        editorData->contextMenu.options[j].callback_f(editorData);
        while(terminal_read() != (TK_MOUSE_LEFT|TK_KEY_RELEASED));
        editorData->contextMenu.showing = false;
        return true;
      }
    editorData->contextMenu.showing = false;
  }
  return editorData->contextMenu.showing;
}

void drawContextMenu(ContextMenu *cMenu) {
  if (!cMenu->showing)
    return;
  terminal_color(color_from_name("menudrpdwnfg"));
  terminal_bkcolor(color_from_name("menudrpdwnbk"));
  for (int j = cMenu->pos.y; j < cMenu->pos.y + cMenu->options.size(); ++j) {
    if (terminal_state(TK_MOUSE_Y) == j 
    &&  terminal_state(TK_MOUSE_X) >= cMenu->pos.x
    &&  terminal_state(TK_MOUSE_X) < cMenu->pos.x + cMenu->options[j-cMenu->pos.y].name.length()) {
      terminal_color(color_from_name("menudrpdwnhlfg"));
      terminal_bkcolor(color_from_name("menudrpdwnhlbk"));
    }
    else {
      terminal_color(color_from_name("menudrpdwnfg"));
      terminal_bkcolor(color_from_name("menudrpdwnbk"));
    }
    terminal_print(cMenu->pos.x, j, cMenu->options[j-cMenu->pos.y].name.c_str());
  }
  terminal_color(color_from_name("workspacedefaultfg"));
  terminal_bkcolor(color_from_name("workspacedefaultbk"));
}
