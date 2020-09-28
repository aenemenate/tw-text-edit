#include "editor_actions.h"
#include "editor_data.h"

#include <filesystem>
#include <fstream>

void NewFile(EditorData *editorData) {
  editorData->buffers.textBuffers.push_back({"New " + std::to_string(editorData->nextFile), {0, 0}, {0, 2}, 0, 0, ""});
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
  if (filename.has_filename()) {
    editorData->buffers.textBuffers.push_back({filename.filename().string(), {0, 0}, {0, 2}, 0, 0, file});
    editorData->buffers.cur = editorData->buffers.textBuffers.size() - 1;
  }
}

void CloseFile(EditorData *editorData) {
  editorData->buffers.textBuffers.erase(editorData->buffers.textBuffers.begin() + editorData->buffers.cur, 
                                        editorData->buffers.textBuffers.begin() + editorData->buffers.cur + 1);
  ++editorData->buffers.cur;
  if (editorData->buffers.cur >= editorData->buffers.textBuffers.size()) {
    editorData->buffers.cur = editorData->buffers.textBuffers.size() - 1;
  }
  if (editorData->buffers.cur < 0) {
    editorData->buffers.cur = 0;
  }
}
