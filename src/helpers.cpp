#include "helpers.h"


bool isNotAllSpaces(std::string str) {
  if (str.size() == 0)
    return false;
  for (int i = 0; i < str.length(); ++i) {
    if (str[i] != ' ')
      return true;
  }
  return false;
}