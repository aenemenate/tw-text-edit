#include "editor_actions.h"
#include "editor_data.h"
#include "text_buffer.h"
#include "../include/BearLibTerminal.h"

#include <filesystem>
#include <fstream>

void NewFile(EditorData *editorData) {
  editorData->buffers.textBuffers.push_back(buildTextBuffer("New " + std::to_string(editorData->nextFile), "", true, {0, 0}, {0, 2}, ""));
  ++editorData->nextFile;
  editorData->buffers.cur = editorData->buffers.textBuffers.size() - 1;
}

void GetOpenFile(EditorData *editorData) {
  editorData->textDropdown.showing = true;
  editorData->textDropdown.action = TextAction::Open;
  resetTextDropdown(&(editorData->textDropdown));
}

void OpenFile(std::string name, EditorData *editorData) {
  std::filesystem::path filename = std::filesystem::path(name);
  std::ifstream filestream(name);
  std::string file = std::string((std::istreambuf_iterator<char>(filestream)), std::istreambuf_iterator<char>());
  if (filestream) {
    editorData->buffers.textBuffers.push_back(buildTextBuffer(filename.filename().string(), filename.parent_path().string(), false, {0, 0}, {0, 2}, file));
    editorData->buffers.cur = editorData->buffers.textBuffers.size() - 1;
  }
  filestream.close();
}

void SaveFile(EditorData *editorData) {
  if (editorData->buffers.textBuffers.empty())
    return;
  TextBuffer *curFile = &(editorData->buffers.textBuffers[editorData->buffers.cur]);
  if (curFile->name != "" && curFile->filepath != "") {
    std::filesystem::path filename = std::filesystem::path(curFile->filepath + '\\' + curFile->name);
    std::ofstream out(filename);
    out << curFile->buffer;
    out.close();
    curFile->isDirty = false;
  }
  else {
    editorData->textDropdown.showing = true;
    editorData->textDropdown.action = TextAction::Save;
    resetTextDropdown(&(editorData->textDropdown));
  }
}

void SaveFileAs(EditorData *editorData) {
  if (editorData->buffers.textBuffers.empty())
    return;
  editorData->textDropdown.showing = true;
  editorData->textDropdown.action = TextAction::Save;
  resetTextDropdown(&(editorData->textDropdown));
}

void CloseFile(EditorData *editorData) {
  if (editorData->buffers.textBuffers[editorData->buffers.cur].isDirty) {
    // prompt to save
  }
  if (editorData->buffers.textBuffers.empty())
    return;
  editorData->buffers.textBuffers.erase(editorData->buffers.textBuffers.begin() + editorData->buffers.cur, 
                                        editorData->buffers.textBuffers.begin() + editorData->buffers.cur + 1);
  editorData->buffers.cur;
  if (editorData->buffers.cur >= editorData->buffers.textBuffers.size()) {
    editorData->buffers.cur = editorData->buffers.textBuffers.size() - 1;
  }
  if (editorData->buffers.cur < 0) {
    editorData->buffers.cur = 0;
  }
}


void Cut(EditorData *editorData) {
  if (editorData->buffers.textBuffers.empty()) return;
  Size termSize = {terminal_state(TK_WIDTH),terminal_state(TK_HEIGHT)};
  TextBuffer *curFile = &(editorData->buffers.textBuffers[editorData->buffers.cur]);
  curFile->cut(termSize, editorData->lineNums);
}

void Copy(EditorData *editorData) {
  if (editorData->buffers.textBuffers.empty()) return;
  Size termSize = {terminal_state(TK_WIDTH),terminal_state(TK_HEIGHT)};
  TextBuffer *curFile = &(editorData->buffers.textBuffers[editorData->buffers.cur]);
  curFile->copy(termSize, editorData->lineNums);
}

void Paste(EditorData *editorData) {
  if (editorData->buffers.textBuffers.empty()) return;
  Size termSize = {terminal_state(TK_WIDTH),terminal_state(TK_HEIGHT)};
  TextBuffer *curFile = &(editorData->buffers.textBuffers[editorData->buffers.cur]);
  curFile->paste(termSize, editorData->lineNums);
}

void StartFind(EditorData *editorData) {
  if (editorData->buffers.textBuffers.empty()) return;
  editorData->textDropdown.showing = true;
  editorData->textDropdown.action = TextAction::Find;
  resetTextDropdown(&(editorData->textDropdown));
}

void ToggleLineNums(EditorData *editorData) {
  editorData->lineNums = !editorData->lineNums;
}
