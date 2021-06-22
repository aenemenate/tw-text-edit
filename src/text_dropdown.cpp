#include "text_dropdown.h"
#include <BearLibTerminal.h>
#include "editor_data.h"
#include "util/helpers.h"
#include "editor_actions.h"
#include "util/filesystem.h"
#include <fstream>

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

int selected_menu_option = 0;
std::string selected_option_name = "";
int num_folder_options = 0;
int num_file_options = 0;

#if defined(_WIN32) || defined(_WIN64)
  char file_delimiter = '\\';
#elif defined(linux)
  char file_delimiter = '/';
#endif

void buildTextDropdown(TextDropdown *textDropdown, Size termSize) {
  textDropdown->inputBuffer = buildTextBuffer("", "", true, {0, 0}, {max(0, termSize.width - 30), 1}, "");
  textDropdown->showing = false;
}

void resetTextDropdown(TextDropdown *textDropdown) {
  textDropdown->inputBuffer.buffer = "";
  textDropdown->inputBuffer.caretPos = 0;
  textDropdown->inputBuffer.caretSelPos = 0;
  textDropdown->inputBuffer.cachedXPos = 0;
  textDropdown->inputBuffer.offs = {0, 0};
}

void drawTextDropdown(EditorData *editorData, Size termSize) {
  selected_option_name == "";
  TextDropdown *textDropdown = &(editorData->textDropdown);
  std::string *workingDirectory = &(editorData->workingDirectory);
  if (textDropdown->showing) {
    drawTextBuffer(&(textDropdown->inputBuffer), {30, 1}, false);
    if (textDropdown->action == TextAction::Open || textDropdown->action == TextAction::Save) {
      int x_pos = max(0, termSize.width - 30);
      terminal_print(min(x_pos, termSize.width - workingDirectory->length()),0,workingDirectory->c_str());
      int i = 1;
      try {
        for (const auto & entry : fs::directory_iterator(*workingDirectory + file_delimiter)) {
          std::string entry_name = entry.path().filename().string();
          try {
            using fs::directory_iterator;
            directory_iterator(entry.path().string() + file_delimiter);
          }
          catch (fs::filesystem_error ex) {
            continue;
          }
          if (entry.is_directory()) {
	    if (i == selected_menu_option)
	      terminal_bkcolor(color_from_name("dark workspacedefaultbk"));
            terminal_clear_area(x_pos, 1+i, termSize.width - x_pos, 1);
            terminal_print_ext(x_pos, 1+i, termSize.width - x_pos, 1, TK_ALIGN_LEFT,
				std::string{file_delimiter + entry_name}.c_str());
	    if (i == selected_menu_option) {
	      terminal_bkcolor(color_from_name("workspacedefaultbk"));
	      selected_option_name = entry_name;
            }
            ++i;
          }
        }
	num_folder_options = i-1;
        for (const auto & entry : fs::directory_iterator(*workingDirectory + file_delimiter)) {
          std::string entry_name = entry.path().filename().string();
          if (!entry.is_directory()) {
	    if (i == selected_menu_option)
	      terminal_bkcolor(color_from_name("dark workspacedefaultbk"));
            terminal_clear_area(x_pos, 1+i, termSize.width - x_pos, 1);
            terminal_print_ext(x_pos, 1+i, termSize.width - x_pos, 1, TK_ALIGN_LEFT, std::string{entry_name}.c_str());
	    if (i == selected_menu_option) {
	      terminal_bkcolor(color_from_name("workspacedefaultbk"));
	      selected_option_name = entry_name;
            }
            ++i;
          }
        }
	num_file_options = i - num_folder_options - 1;
      }
      catch (fs::filesystem_error _ex) {
        *workingDirectory = fs::path(*workingDirectory).parent_path().string();
      }
    }
    else if (textDropdown->action == TextAction::Find) {
      int x_pos = max(0, termSize.width - 30);
      int occurrences = 0;
      int pos = 0;
      if (editorData->buffers.textBuffers[editorData->buffers.cur].buffer != "" && textDropdown->inputBuffer.buffer != "") {
        while ((pos = editorData->buffers.textBuffers[editorData->buffers.cur].buffer.find(textDropdown->inputBuffer.buffer, pos )) != std::string::npos) {
          ++ occurrences;
          pos += textDropdown->inputBuffer.buffer.length();
        }
      }
      // TODO: print this on the status bar as well when finding text
      terminal_print(x_pos,0,std::string{"Occurrences: " + std::to_string(occurrences)}.c_str());
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
      int pos, occurrences;
      switch (editorData->textDropdown.action) {
        case TextAction::Open:
        case TextAction::Save:
          for (auto it = inputText.begin(); it < inputText.end(); ++it) {
            if(illegalChars.find(*it) != std::string::npos)
              break;
          }
          if (inputText == "" && selected_menu_option > 0)
	    inputText = selected_option_name;
          filename = editorData->workingDirectory + file_delimiter + inputText;
          if (inputText != "" && inputText != "." && isNotAllSpaces(inputText)) {
	    selected_menu_option = 0;
	    selected_option_name = "";
            if (fs::exists(filename)) {
              if (inputText == "..") {
                editorData->workingDirectory = fs::path(editorData->workingDirectory).parent_path().string();
                editorData->textDropdown.showing = true;
                resetTextDropdown(&(editorData->textDropdown));
              }
              else if (fs::is_directory(filename)) {
                editorData->workingDirectory = filename;
                editorData->textDropdown.showing = true;
                resetTextDropdown(&(editorData->textDropdown));
              }
              else {
                if (editorData->textDropdown.action == TextAction::Open)
                  OpenFile(filename, editorData);
                else if (editorData->textDropdown.action == TextAction::Save) {
                  // TODO: move this into save function and pass filename and filepath as parameters
                  std::ofstream out(filename);
                  if (out) {
                    editorData->buffers.textBuffers[editorData->buffers.cur].name = inputText;
                    editorData->buffers.textBuffers[editorData->buffers.cur].filePath = editorData->workingDirectory;
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
                  editorData->buffers.textBuffers[editorData->buffers.cur].filePath = editorData->workingDirectory;
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
          occurrences = 0;
          pos = 0;
          if (editorData->buffers.textBuffers[editorData->buffers.cur].buffer != "" && editorData->textDropdown.inputBuffer.buffer != "") {
            while ((pos = editorData->buffers.textBuffers[editorData->buffers.cur].buffer.find(editorData->textDropdown.inputBuffer.buffer, pos )) != std::string::npos) {
              ++ occurrences;
              pos += editorData->textDropdown.inputBuffer.buffer.length();
            }
          }
          if (occurrences > 0)
            editorData->buffers.textBuffers[editorData->buffers.cur].findText = inputText;
          else
            editorData->buffers.textBuffers[editorData->buffers.cur].findText = "";
          break;
        case TextAction::FindAndReplace:
          // prompt for replace text
          break;
      }
      if (editorData->workingDirectory.back() == file_delimiter)
        editorData->workingDirectory = editorData->workingDirectory.substr(0, editorData->workingDirectory.length() - 1);
      return true;
    }
    else if (editorData->textDropdown.action == TextAction::Save
	 ||  editorData->textDropdown.action == TextAction::Open) {
      if (key == TK_UP) {
	selected_menu_option = max(0, selected_menu_option - 1);
      }
      else if (key == TK_DOWN) {
	selected_menu_option = min(num_folder_options + num_file_options, selected_menu_option + 1);
      }
    }
    // print any extra necessary text
  }
  return editorData->textDropdown.showing;
}