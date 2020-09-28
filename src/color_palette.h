#pragma once

#include <string>

struct Color {
  std::string fg, bg;
};

struct ColorPalette {
  Color menuBarColor;
  Color menuBarHighlightedColor;
  Color menuBarDropdownColor;
  Color menuBarDropdownHighlightedColor;
  Color bufferListColor;
  Color bufferListHighlightedColor;
  Color bufferListActiveColor;
  Color workspaceDefaultColor;
};

void buildColorPalette(ColorPalette *colorPalette);
