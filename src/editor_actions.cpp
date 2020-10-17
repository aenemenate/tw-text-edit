#include "editor_actions.h"
#include "editor_data.h"
#include "text_buffer.h"
#include <BearLibTerminal.h>

#include "util/filesystem.h"
#include <fstream>

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <algorithm>

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#if defined(_WIN32) || defined(_WIN64)
#define PCLOSE _pclose
#define POPEN _popen
#elif defined(linux)
#define PCLOSE pclose
#define POPEN popen
#endif

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&PCLOSE)> pipe(POPEN(cmd, "r"), PCLOSE);
    if (!pipe)
        throw std::runtime_error("_popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        result += buffer.data();
    return result;
}

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
  fs::path filename = fs::absolute(fs::path(name));
  std::ifstream filestream(name);
  std::string file = std::string((std::istreambuf_iterator<char>(filestream)), std::istreambuf_iterator<char>());
  if (filestream) {
    editorData->buffers.textBuffers.push_back(buildTextBuffer(filename.filename().string(), filename.parent_path().string(), false, {0, 0}, {0, 2}, file));
    editorData->buffers.cur = editorData->buffers.textBuffers.size() - 1;
  }
  filestream.close();
}

void SaveFile(EditorData *editorData) {
  if (editorData->buffers.textBuffers.empty() ||
      !editorData->buffers.textBuffers[editorData->buffers.cur].isDirty)
    return;
  TextBuffer *curFile = &(editorData->buffers.textBuffers[editorData->buffers.cur]);
  if (curFile->name != "" && curFile->filePath != "") {
    fs::path fileName = fs::path(curFile->filePath + '/' + curFile->name);
    std::ofstream out(fileName);
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

void SaveAllFiles(EditorData *editorData) {
  if (editorData->buffers.textBuffers.empty())
    return;
  int starting_buf = editorData->buffers.cur;
  for (int i = 0; i < editorData->buffers.textBuffers.size(); ++i) {
    editorData->buffers.cur = i;
    SaveFile(editorData);
  }
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
  Size termSize = {terminal_state(TK_WIDTH),terminal_state(TK_HEIGHT)-1};
  TextBuffer *curFile = &(editorData->buffers.textBuffers[editorData->buffers.cur]);
  curFile->cut(termSize, editorData->lineNums);
}

void Copy(EditorData *editorData) {
  if (editorData->buffers.textBuffers.empty()) return;
  Size termSize = {terminal_state(TK_WIDTH),terminal_state(TK_HEIGHT)-1};
  TextBuffer *curFile = &(editorData->buffers.textBuffers[editorData->buffers.cur]);
  curFile->copy(termSize, editorData->lineNums);
}

void Paste(EditorData *editorData) {
  if (editorData->buffers.textBuffers.empty()) return;
  Size termSize = {terminal_state(TK_WIDTH),terminal_state(TK_HEIGHT)-1};
  TextBuffer *curFile = &(editorData->buffers.textBuffers[editorData->buffers.cur]);
  curFile->paste(termSize, editorData->lineNums);
}

void StartFind(EditorData *editorData) {
  if (editorData->buffers.textBuffers.empty()) return;
  editorData->textDropdown.showing = true;
  editorData->textDropdown.action = TextAction::Find;
  resetTextDropdown(&(editorData->textDropdown));
}

void Undo(EditorData *editorData) {
  if (editorData->buffers.textBuffers.empty()) return;
  Size termSize = {terminal_state(TK_WIDTH),terminal_state(TK_HEIGHT)-1};
  editorData->buffers.textBuffers[editorData->buffers.cur].undo(termSize, editorData->lineNums);
}

void Redo(EditorData *editorData) {
  if (editorData->buffers.textBuffers.empty()) return;
  Size termSize = {terminal_state(TK_WIDTH),terminal_state(TK_HEIGHT)-1};
  editorData->buffers.textBuffers[editorData->buffers.cur].redo(termSize, editorData->lineNums);
}

void RunBatchFile(EditorData *editorData) {
  if (editorData->buffers.textBuffers.empty()) 
    return;
  std::string fileName = editorData->buffers.textBuffers[editorData->buffers.cur].name;
  int dot_ind = fileName.find('.', 0)+1;
  std::string ext = fileName.substr(dot_ind, fileName.length() - dot_ind);
  if (ext != "bat" && ext != "cmd") 
    return;
  std::string filePath = editorData->buffers.textBuffers[editorData->buffers.cur].filePath;
// run the batch file
  std::string system_call = '\"' + filePath + "/" + fileName + '\"';
  std::string output = exec(system_call.c_str());
  editorData->buffers.textBuffers.push_back(buildTextBuffer(fileName.substr(0, dot_ind-1)+"_log", "", true, {0, 0}, {0, 2}, output));
  editorData->buffers.cur = editorData->buffers.textBuffers.size() - 1;
}

void ToggleLineNums(EditorData *editorData) {
  editorData->lineNums = !editorData->lineNums;
}
void SwitchBufferLeft(EditorData *editorData) {
  if (editorData->buffers.textBuffers.empty()) 
    return;
  if (editorData->buffers.cur > 0)
    --editorData->buffers.cur;
}

void SwitchBufferRight(EditorData *editorData) {
  if (editorData->buffers.textBuffers.empty()) 
    return;
  if (editorData->buffers.cur < editorData->buffers.textBuffers.size()-1)
    ++editorData->buffers.cur;
}

void MoveBufferLeft(EditorData *editorData) {
  if (editorData->buffers.textBuffers.empty()) 
    return;
  std::vector<TextBuffer> *vec =  &(editorData->buffers.textBuffers);
  if (editorData->buffers.cur < 1)
    return;
  std::vector<TextBuffer>::iterator from = vec->begin() + editorData->buffers.cur;
  std::vector<TextBuffer>::iterator to = vec->begin() + max(0,editorData->buffers.cur - 1);
  if (from < to)
    rotate(from, from+1, to+1);
  else if (from > to)
    rotate(to, from, from+1);
  --editorData->buffers.cur;
}

void MoveBufferRight(EditorData *editorData) {
  if (editorData->buffers.textBuffers.empty()) 
    return;
  std::vector<TextBuffer> *vec =  &(editorData->buffers.textBuffers);
  if (editorData->buffers.cur >= vec->size()-1)
    return;
  std::vector<TextBuffer>::iterator from = vec->begin() + editorData->buffers.cur + 1;
  std::vector<TextBuffer>::iterator to = vec->begin() + min(vec->size()-1,editorData->buffers.cur);
  if (from < to)
    rotate(from, from+1, to+1);
  else if (from > to)
    rotate(to, from, from+1);
  ++editorData->buffers.cur;
}
