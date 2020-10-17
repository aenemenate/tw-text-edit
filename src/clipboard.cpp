#include "clipboard.h"
#if defined(_WIN32) || defined(_WIN64)
  #include <windows.h>
#endif
std::string GetClipboardContents() {
#if defined(_WIN32) || defined(_WIN64)
  std::string text;
  if (OpenClipboard(nullptr)) {
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData != nullptr) {
      char * pszText = static_cast<char*>( GlobalLock(hData) );
      if (pszText != nullptr)
        text = pszText;
      GlobalUnlock(hData);
      CloseClipboard();
    }
  }
  return text;
#endif
  return std::string("");
}

void PushTextToClipboard(std::string text) {
#if defined(_WIN32) || defined(_WIN64)
  const char* output = text.c_str();
  const size_t len = strlen(output) + 1;
  HGLOBAL hMem =  GlobalAlloc(GMEM_MOVEABLE, len);
  memcpy(GlobalLock(hMem), output, len);
  GlobalUnlock(hMem);
  OpenClipboard(0);
  EmptyClipboard();
  SetClipboardData(CF_TEXT, hMem);
  CloseClipboard();
#endif
}
