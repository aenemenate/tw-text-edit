#pragma once

#include <string>
class EditorData;

void NewFile(EditorData*);
void GetOpenFile(EditorData*);
void OpenFile(std::string name, EditorData*);
void SaveFile(EditorData*);
void SaveFileAs(EditorData*);
void SaveAllFiles(EditorData*);
void CloseFile(EditorData*);

void Cut(EditorData*);
void Copy(EditorData*);
void Paste(EditorData*);
void StartFind(EditorData*);

void Undo(EditorData*);
void Redo(EditorData*);

void RunBatchFile(EditorData*);

void ToggleLineNums(EditorData*);