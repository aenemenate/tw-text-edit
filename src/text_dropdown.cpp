#include "text_dropdown.h"
#include "color_palette.h"
#include "../include/BearLibTerminal.h"
#include "editor_data.h"
#include "helpers.h"
#include "editor_actions.h"
#include <filesystem>
#include <fstream>

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif


void buildTextDropdown(TextDropdown *textDropdown, Size termSize) {
  textDropdown->inputBuffer = {"", "", true, {0, 0}, {max(0, termSize.width - 30), 1}, 0, 0, ""};
  textDropdown->showing = false;
}

void resetTextDropdown(TextDropdown *textDropdown) {
  textDropdown->inputBuffer.buffer = "";
  textDropdown->inputBuffer.caret_pos = 0;
  textDropdown->inputBuffer.cached_x_pos = 0;
  textDropdown->inputBuffer.offs = {0, 0};
}

void drawTextDropdown(TextDropdown *textDropdown, std::string *workingDirectory, ColorPalette *colorPalette, Size termSize) {
  if (textDropdown->showing) {
    drawTextBuffer(&(textDropdown->inputBuffer), colorPalette, {30, 1}, false);
    if (textDropdown->action == TextAction::Open || textDropdown->action == TextAction::Save) {
      int x_pos = max(0, termSize.width - 30);
      terminal_print(min(x_pos, termSize.width - workingDirectory->length()),0,workingDirectory->c_str());
      int i = 1;
      try {
        for (const auto & entry : std::filesystem::directory_iterator(*workingDirectory + '\\')) {
          std::string entry_name = entry.path().filename().string();
          try {
            using std::filesystem::directory_iterator;
            directory_iterator(entry.path().string() + '\\');
          }
          catch (std::filesystem::filesystem_error ex) {
            continue;
          }
          if (entry.is_directory()) {
            terminal_clear_area(x_pos, 1+i, termSize.width - x_pos, 1);
            terminal_print_ext(x_pos, 1+i, termSize.width - x_pos, 1, TK_ALIGN_LEFT, std::string{'\\' + entry_name}.c_str());
            ++i;
          }
        }
        for (const auto & entry : std::filesystem::directory_iterator(*workingDirectory + '\\')) {
          std::string entry_name = entry.path().filename().string();
          if (!entry.is_directory()) {
            terminal_clear_area(x_pos, 1+i, termSize.width - x_pos, 1);
            terminal_print_ext(x_pos, 1+i, termSize.width - x_pos, 1, TK_ALIGN_LEFT, std::string{entry_name}.c_str());
            ++i;
          }
        }
      }
      catch (std::filesystem::filesystem_error _ex) {
        *workingDirectory = std::filesystem::path(*workingDirectory).parent_path().string();
      }
    }
  }
}

bool handleInputTextDropdown(EditorData *editorData, int key, Size termSize) {
  if (editorData->textDropdown.showing) {
    bool finished = handleInputTextBuffer(&(editorData->textDropdown.inputBuffer), key, {30, 1}, true, false);
    if (finished) {
      editorData->textDropdown.showing = false;
      std::string filename;
      std::string inputText = editorData->textDropdown.inputBuffer.buffer;
      std::string illegalChars = "\\/:?\"<>|";
      switch (editorData->textDropdown.action) {
        case TextAction::Open:
        case TextAction::Save:
          for (auto it = inputText.begin(); it < inputText.end(); ++it) {
            if(illegalChars.find(*it) != std::string::npos)
              break;
          }
          filename = editorData->workingDirectory + "\\" + inputText;
          if (inputText != "" && inputText != "." && isNotAllSpaces(inputText)) {
            if (std::filesystem::exists(filename)) {
              if (inputText == "..") {
                editorData->workingDirectory = std::filesystem::path(editorData->workingDirectory).parent_path().string();
                editorData->textDropdown.showing = true;
                resetTextDropdown(&(editorData->textDropdown));
              }
              else if (std::filesystem::is_directory(filename)) {
                editorData->workingDirectory = filename;
                editorData->textDropdown.showing = true;
                resetTextDropdown(&(editorData->textDropdown));
              }
              else {
                if (editorData->textDropdown.action == TextAction::Open)
                  OpenFile(editorData->workingDirectory + "\\" + editorData->textDropdown.inputBuffer.buffer, editorData);
                else if (editorData->textDropdown.action == TextAction::Save) {
                  // TODO: move this into save function and pass filename and filepath as parameters
                  std::ofstream out(filename);
                  if (out) {
                    editorData->buffers.textBuffers[editorData->buffers.cur].name = inputText;
                    editorData->buffers.textBuffers[editorData->buffers.cur].filepath = editorData->workingDirectory;
                    out.close();
                    SaveFile(editorData);
                  }
                  else {
                    editorData->textDropdown.showing = true;
                    resetTextDropdown(&(editorData->textDropdown));
                    out.close();
                  }
                }
              }
            }
            else {
              if (editorData->textDropdown.action == TextAction::Save) {
                std::ofstream out(filename);
                if (out) {
                  editorData->buffers.textBuffers[editorData->buffers.cur].name = inputText;
                  editorData->buffers.textBuffers[editorData->buffers.cur].filepath = editorData->workingDirectory;
                  out.close();
                  SaveFile(editorData);
                }
                else {
                  editorData->textDropdown.showing = true;
                  resetTextDropdown(&(editorData->textDropdown));
                  out.close();
                }
              }
            }
          }
          break;
        case TextAction::Find:
          // highlight found text, then enter mode where user can press left and right to jump to instances
          break;
        case TextAction::FindAndReplace:
          // prompt for replace text
          break;
      }
      if (editorData->workingDirectory.back() == '\\')
        editorData->workingDirectory = editorData->workingDirectory.substr(0, editorData->workingDirectory.length() - 1);
      return true;
    }
    // print any extra necessary text
  }
  return editorData->textDropdown.showing;
}
