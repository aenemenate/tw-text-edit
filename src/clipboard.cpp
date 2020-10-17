#include "clipboard.h"

#if defined(_WIN32) || defined(_WIN64)
  #include <windows.h>
#elif defined(linux)
  #include <memory>
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
#elif defined(linux)
    const unsigned buffsize = 1024;
    char buffer[buffsize];
    std::string result;
    std::shared_ptr<FILE> pipe(popen("xclip -selection clipboard -o", "r"), pclose);

    if (!pipe)
        return "";

    while (!feof(pipe.get()))
    {
        if (fgets(buffer, buffsize, pipe.get()) != NULL)
            result += buffer;
    }
    return result;
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
#elif defined(linux)
    std::shared_ptr<FILE> pipe(popen("xclip -selection clipboard -i", "w"), pclose);

    if (!pipe)
        return;

    fprintf(pipe.get(), "%s", text.c_str());

    if (ferror (pipe.get()))
        return;
#endif
}
