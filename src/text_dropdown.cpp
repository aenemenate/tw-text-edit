#include "text_dropdown.h"
#include "color_palette.h"
#include "../include/BearLibTerminal.h"
#include "editor_data.h"
#include "helpers.h"
#include "editor_actions.h"
#include <filesystem>

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif


void buildTextDropdown(TextDropdown *textDropdown, Size termSize) {
  textDropdown->inputBuffer = {"", "", {0, 0}, {max(0, termSize.width - 30), 1}, 0, 0, ""};
  textDropdown->showing = false;
}

void resetTextDropdown(TextDropdown *textDropdown) {
  textDropdown->inputBuffer.buffer = "";
  textDropdown->inputBuffer.caret_pos = 0;
  textDropdown->inputBuffer.cached_x_pos = 0;
  textDropdown->inputBuffer.offs = {0, 0};
}

void drawTextDropdown(TextDropdown *textDropdown, std::string workingDirectory, ColorPalette *colorPalette, Size termSize) {
  if (textDropdown->showing) {
    drawTextBuffer(&(textDropdown->inputBuffer), colorPalette, {30, 1});
    if (textDropdown->action == TextAction::Open) {
      int x_pos = max(0, termSize.width - 30);
      terminal_print(min(x_pos, termSize.width - workingDirectory.length()),0,workingDirectory.c_str());
      int i = 1;
      for (const auto & entry : std::filesystem::directory_iterator(workingDirectory)) {
        terminal_print(x_pos,1+i, entry.path().filename().string().c_str());
        ++i;
      }
    }
  }
}

bool handleInputTextDropdown(EditorData *editorData, int key, Size termSize) {
  if (editorData->textDropdown.showing) {
    bool finished = handleInputTextBuffer(&(editorData->textDropdown.inputBuffer), key, {30, 1}, true);
    if (finished) {
      editorData->textDropdown.showing = false;
      std::string filename;
      std::string inputText = editorData->textDropdown.inputBuffer.buffer;
      switch (editorData->textDropdown.action) {
        case TextAction::Open:
          filename = editorData->workingDirectory + "\\" + inputText;
          if (inputText != "" && inputText != "." && isNotAllSpaces(inputText) && std::filesystem::exists(filename)) {
            if (inputText == "../" || inputText == "..") {
              editorData->workingDirectory = std::filesystem::path(editorData->workingDirectory).parent_path().string();
              if (editorData->workingDirectory[editorData->workingDirectory.length()-1] == '\\')
                editorData->workingDirectory = editorData->workingDirectory.substr(0, editorData->workingDirectory.length() - 1);
              editorData->textDropdown.showing = true;
              resetTextDropdown(&(editorData->textDropdown));
            }
            else if (std::filesystem::is_directory(filename)) {
              editorData->workingDirectory = filename;
              editorData->textDropdown.showing = true;
              resetTextDropdown(&(editorData->textDropdown));
            }
            else {
              OpenFile(editorData->workingDirectory + "\\" + editorData->textDropdown.inputBuffer.buffer, editorData);
            }
          }
          break;
        case TextAction::Save:
          // save file as entered by user
          break;
        case TextAction::Find:
          // highlight found text, then enter mode where user can press left and right to jump to instances
          break;
        case TextAction::FindAndReplace:
          // prompt for replace text
          break;
      }
      return true;
    }
    // print any extra necessary text
  }
  return editorData->textDropdown.showing;
}
