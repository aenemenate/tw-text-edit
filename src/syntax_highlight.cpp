#include "syntax_highlight.h"
#include "text_buffer.h"

#include <algorithm>

// "syntax highlighting"
void Syntax::updateSyntax(TextBuffer *buf, std::string filename) {
  bufVec.resize(buf->buffer.length());
  int ix = 0;
  color_t fg_color = color_from_name("workspacedefaultfg");
  for (auto c : buf->buffer) {
    bufVec[ix] = fg_color;
    ++ix;
  }
  if (filename.find(".cpp", 0) != std::string::npos
  ||  filename.find(".h", 0)   != std::string::npos 
  ||  filename.find(".hpp", 0) != std::string::npos
  ||  filename.find(".cxx", 0)	!= std::string::npos) {
  // prepocessor directives
  std::vector<std::string> keywords = { "#include", "#pragma once", "#pragma" };
  for (int i = 0; i < keywords.size(); ++i) {
    size_t c = -1;
    while (buf->buffer.find(keywords[i], c+1) != std::string::npos) {
      c = buf->buffer.find(keywords[i], c+1);
      for (int j = c; j < c + keywords[i].length(); ++j) {
        bufVec[j] = color_from_name("cppdirectives");
      }
    }
  }
// purple (actions)
  keywords = { 
    "#ifndef", "for", "#define", "#endif", "if", "continue", "break", 
    "using",  "while", "static", "do", "goto", "throw", "return", 
    "else", "switch", "case", "try", "catch", "delete", "default", 
    "new" };
  for (int i = 0; i < keywords.size(); ++i) {
    std::vector<char> check_chars_end = {'(', ' ', '\n', '\t', ';', '{', '}', ':'};
    std::vector<char> check_chars_front = {' ', '\n', '\t', ';', '{', '}'};
    size_t c = -1;
    while (buf->buffer.find(keywords[i], c+1) != std::string::npos) {
      c = buf->buffer.find(keywords[i], c+1);
      if ((c+keywords[i].length() >= buf->buffer.length() || std::count(check_chars_end.begin(), check_chars_end.end(), buf->buffer[c+keywords[i].length()]))
      &&  (c-1 < 0 || std::count(check_chars_front.begin(), check_chars_front.end(), buf->buffer[c-1]))) {
        for (int j = c; j < c + keywords[i].length(); ++j)
          bufVec[j] = color_from_name("cppstatements");
      }
    }
  }
// blue (data)
  keywords = { 
    "sizeof", "int", "bool", "enum", "static_cast", "template", 
    "typedef", "typeid", "typename", "mutable", "namespace", "static", 
    "dynamic_cast", "struct", "inline", "volatile", "char", "union", 
    "extern", "class", "auto", "this", "long", "double", "float","const", 
    "unsigned", "protected", "private", "public", "virtual", "void", 
    "NULL", "nullptr", "true", "false", "friend" };
  for (int i = 0; i < keywords.size(); ++i) {
    std::vector<char> check_chars_end = {'>', '<', '=', ')', '(', '[', '-', '{', ' ', ';', '.', '&', ':', ',', '\n', '\t'};
    std::vector<char> check_chars_front = {'<', '=', ' ', '(', ')', ',', ';', '{', '*', '\n', '\t'};
    size_t c = -1;
    while (buf->buffer.find(keywords[i], c+1) != std::string::npos) {
      c = buf->buffer.find(keywords[i], c+1);
      if ((c+keywords[i].length() >= buf->buffer.length() || std::count(check_chars_end.begin(), check_chars_end.end(), buf->buffer[c+keywords[i].length()]))
      &&  (c-1 < 0 || std::count(check_chars_front.begin(), check_chars_front.end(), buf->buffer[c-1]))) {
        for (int j = c; j < c + keywords[i].length(); ++j) {
          bufVec[j] = color_from_name("cppdefinitions");
        }
      }
    }
  }

// mathematical symbols
  keywords = { 
    "{", "}", "(", ")", "*", "&", "/", "+", "-", "=", "!", ">", "<", 
    ":", ",", "." 
  };
  for (int i = 0; i < keywords.size(); ++i) {
    size_t c = -1;
    while (buf->buffer.find(keywords[i], c+1) != std::string::npos) {
      c = buf->buffer.find(keywords[i], c+1);
      for (int j = c; j < c + keywords[i].length(); ++j) {
        bufVec[j] = color_from_name("cppsymbols");
      }
    }
  }

// unclosed brackets
  std::vector<char> opens = {
    '{', '(', '[' 
  };
  std::vector<char> closeds = {
    '}', ')', ']'
  };
  for (int i = 0; i < opens.size(); ++i) {
    std::vector<int> st;

    for (int j = 0; j < buf->buffer.length(); j++) {

        if (buf->buffer[j] == opens[i] && (j < 1 || buf->buffer[j-1] != '\'')) 

            st.push_back(j);

        else if (buf->buffer[j] == closeds[i] && (j < 1 || buf->buffer[j-1] != '\'')) {

            if (st.empty()) 

              bufVec[j] = color_from_name("red");
            else st.pop_back();
        }
    }
  }

// yellow: chars
  ix = 0;
  bool toggleChar = false;
  for (auto c : buf->buffer) {
    fg_color = bufVec[ix];
    if (c == '\'' && !(buf->buffer[ix-1] == '\\' && buf->buffer[ix-2] != '\\')) {
      toggleChar = !toggleChar;
      fg_color = color_from_name("cppchars");
    }
    if (toggleChar)
      fg_color = color_from_name("cppchars");
    bufVec[ix] = {fg_color};
    ++ix;
  }
// green: comments, strings
  ix = 0;
  bool toggleString = false, toggleComment = false, toggleMultiLineComment = false;
  for (auto c : buf->buffer) {
    fg_color = bufVec[ix];
    if (c == '\"' && !(buf->buffer[ix-1] == '\\' && buf->buffer[ix-2] != '\\')) {
      toggleString = !toggleString;
      fg_color = color_from_name("cppstrcomments");
    }
    else if (c == '/' && ix < buf->buffer.length() - 1 && buf->buffer[ix+1] == '/' && toggleMultiLineComment == false)
      toggleComment = true;
    else if (c == '/' && ix < buf->buffer.length() - 1 && buf->buffer[ix+1] == '*')
      toggleMultiLineComment = true;
    if (toggleString)
      fg_color = color_from_name("cppstrcomments");
    if (toggleComment || toggleMultiLineComment) {
      if ((toggleComment && ix < buf->buffer.length() - 1 && buf->buffer[ix+1] == '\n') 
      || (toggleMultiLineComment && c == '/' && ix > 0 && buf->buffer[ix-1] == '*')) {  
        toggleComment = false;
        toggleMultiLineComment = false;
      }
      fg_color = color_from_name("cppstrcomments");
    }
    bufVec[ix] = fg_color;
    ++ix;
  }
  }
}
