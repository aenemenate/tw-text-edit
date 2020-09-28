#include "color_palette.h"

void buildColorPalette(ColorPalette *colorPalette) {
  colorPalette->menuBarColor                    = {"darker 0,43,54", "dark 101,123,131"};
  colorPalette->menuBarHighlightedColor         = {"darker 0,43,54", "101,123,131"};
  colorPalette->menuBarDropdownColor            = {"darker 0,43,54", "101,123,131"};
  colorPalette->menuBarDropdownHighlightedColor = {"147,161,161", "101,123,131"};
  colorPalette->bufferListColor                 = {"darker 0,43,54", "dark 101,123,131"};
  colorPalette->bufferListHighlightedColor      = {"darker 0,43,54", "light 101,123,131"};
  colorPalette->bufferListActiveColor           = {"147,161,161", "0,43,54"};
  colorPalette->workspaceDefaultColor           = {"253,246,227", "0,43,54"};
}