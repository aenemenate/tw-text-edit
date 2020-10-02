#include "base_types.h"
#include "menu_bar.h"

class EditorData;

struct ContextMenu {
  Point pos;
  vector<MenuFunction> options;
  bool showing;
};

void buildContextMenu(ContextMenu *contextMenu, Point pos);

void setContextMenuShowing(ContextMenu *contextMenu, Point pos);

bool handleInputContextMenu(EditorData *editorData, int key);

void drawContextMenu(ContextMenu *contextMenu);