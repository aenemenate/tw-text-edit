#include "clipboard.h"
#include <clip.h>

std::string GetClipboardContents() {
  std::string clip_content;
  clip::get_text(clip_content);
  return clip_content;
}

void PushTextToClipboard(std::string text) {
  clip::set_text(text);
}
