
#include <algorithm>

#include "syntax_highlight.h"
#include "text_buffer.h"

void Syntax::highlightMultiLineComments(char openDelim, char closedDelim, TextBuffer *buf) {
  color_t fg_color;
  int ix = 0;
  bool toggleMultiLineComment = false;
  for (auto c : buf->buffer) {
    fg_color = bufVec[ix];
    if (c == openDelim && ix < buf->buffer.length() - 1 && buf->buffer[ix+1] == '*')
      toggleMultiLineComment = true;
    if (toggleMultiLineComment) {
      if (toggleMultiLineComment && c == closedDelim && ix > 0 && buf->buffer[ix-1] == '*')
        toggleMultiLineComment = false;
      fg_color = color_from_name("cppstrcomments");
    }
    bufVec[ix] = fg_color;
    ++ix;
  }
}

void Syntax::highlightSingleLineComments(TextBuffer *buf) {
  
  color_t fg_color;
  int ix = 0;
  bool toggleComment = false;
  for (auto c : buf->buffer) {
    fg_color = bufVec[ix];
    if (c == '/' && ix < buf->buffer.length() - 1 && buf->buffer[ix+1] == '/')
      toggleComment = true;
    if (toggleComment)
      fg_color = color_from_name("cppstrcomments");
    if (toggleComment && ix < buf->buffer.length() - 1 && buf->buffer[ix+1] == '\n')
      toggleComment = false;
    bufVec[ix] = fg_color;
    ++ix;
  }
}

void Syntax::highlightStrings(char delim, TextBuffer *buf, char delimVariant) {
  
  color_t fg_color;
  int  ix = 0;
  bool toggleString = false, toggleVariantString = false;
  for (auto c : buf->buffer) {
    fg_color = bufVec[ix];
    if (c == delim
    && !((ix == 0 || buf->buffer[ix-1] == '\\') && (ix <= 1 || buf->buffer[ix-2] != '\\'))
    && toggleVariantString == false) {
      toggleString = !toggleString;
      fg_color = color_from_name("cppstrcomments");
    }
    if (delimVariant != ' '
    && c == delimVariant 
    && !((ix == 0 || buf->buffer[ix-1] == '\\') && (ix <= 1 || buf->buffer[ix-2] != '\\'))
    && toggleString == false) {
      toggleVariantString = !toggleVariantString;
      fg_color = color_from_name("cppstrcomments");
    }
    if (toggleString || toggleVariantString)
      fg_color = color_from_name("cppstrcomments");
    bufVec[ix] = fg_color;
    ++ix;
  }
}

void Syntax::highlightCharacters(TextBuffer *buf) {
  color_t fg_color;
  int ix = 0;
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
}

void Syntax::highlightKeywords(std::vector<std::string> keywords, std::vector<char> allowedBefore, 
			 std::vector<char> allowedAfter, std::string color, TextBuffer *buf) {
  
}

void Syntax::highlightUnclosedBrackets(std::vector<char> opens, std::vector<char> closeds, TextBuffer *buf) {
  
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
}

void Syntax::highlightCpp(TextBuffer *buf) {
  int ix = 0;
  color_t fg_color = color_from_name("workspacedefaultfg");
// prepocessor directives
  std::vector<std::string> keywords = { "#ifndef", "#define", "#undef", "#elif", "#endif",
					"#include", "#pragma once", "#pragma" };
  for (int i = 0; i < keywords.size(); ++i) {
    size_t c = -1;
    while (buf->buffer.find(keywords[i], c+1) != std::string::npos) {
      c = buf->buffer.find(keywords[i], c+1);
      for (int j = c; j < c + keywords[i].length(); ++j)
        bufVec[j] = color_from_name("cppdirectives");
    }
  }
// purple (actions)
  keywords = {
      "for", "if", "continue", "break", "new",
      "using",  "while", "static", "do", "goto", "throw", "return",
      "else", "switch", "case", "try", "catch", "delete", "default"
  };
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
        &&  (c-1 < 0 || std::count(check_chars_front.begin(), check_chars_front.end(), buf->buffer[c-1])))
          for (int j = c; j < c + keywords[i].length(); ++j)
            bufVec[j] = color_from_name("cppdefinitions");
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
        for (int j = c; j < c + keywords[i].length(); ++j)
          bufVec[j] = color_from_name("cppsymbols");
      }
    }

// unclosed brackets
  std::vector<char> opens = {
      '{', '(', '['
  };
  std::vector<char> closeds = {
      '}', ')', ']'
  };
  highlightUnclosedBrackets(opens, closeds, buf);
// yellow: chars
  highlightCharacters(buf);
// green: comments, strings
  highlightSingleLineComments(buf);
  highlightMultiLineComments('/', '/', buf);
  highlightStrings('\"', buf);
}

void Syntax::highlightOcaml(TextBuffer *buf) {
    color_t fg_color = color_from_name("workspacedefaultfg");
    int ix = 0;
// prepocessor directives
    std::vector<std::string> keywords = { 
      "module", "open", "include"
    };
    for (int i = 0; i < keywords.size(); ++i) {
      size_t c = -1;
      while (buf->buffer.find(keywords[i], c+1) != std::string::npos) {
        c = buf->buffer.find(keywords[i], c+1);
        for (int j = c; j < c + keywords[i].length(); ++j)
          bufVec[j] = color_from_name("cppdirectives");
      }
    }
// purple (actions)
    keywords = {
      "fun", "let", "try", "raise", "if", "then", "else", "for", "downto", "do", "and", "rec", "begin",
      "while", "match", "with", "in", "type", "ref", "class", "inherit", "new", "end"
    };
    for (int i = 0; i < keywords.size(); ++i) {
      std::vector<char> check_chars_end = {'(', ' ', '\n', '\t', ';', '{', '}', ':'};
      std::vector<char> check_chars_front = {' ', '\n', '\t', ';', '{', '}'};
      size_t c = -1;
      while (buf->buffer.find(keywords[i], c+1) != std::string::npos) {
        c = buf->buffer.find(keywords[i], c+1);
        if ((c+keywords[i].length() >= buf->buffer.length() 
	|| std::count(check_chars_end.begin(), check_chars_end.end(), buf->buffer[c+keywords[i].length()])
        )&&(c-1 < 0 || std::count(check_chars_front.begin(), check_chars_front.end(), buf->buffer[c-1]))) {
          for (int j = c; j < c + keywords[i].length(); ++j)
            bufVec[j] = color_from_name("cppstatements");
        }
      }
    }
// blue (data)
    keywords = { 
      "false", "true", "bool", "string", "list", "int", "float", "char" };
    for (int i = 0; i < keywords.size(); ++i) {
      std::vector<char> check_chars_end = {'>', '<', '=', ')', '(', '[', '-', '{', ' ', ';', '.', '&', ':', ',', '\n', '\t'};
      std::vector<char> check_chars_front = {'<', '=', ' ', '(', ')', ',', ';', '{', '*', '\n', '\t'};
      size_t c = -1;
      while (buf->buffer.find(keywords[i], c+1) != std::string::npos) {
        c = buf->buffer.find(keywords[i], c+1);
        if ((c+keywords[i].length() >= buf->buffer.length() || std::count(check_chars_end.begin(), check_chars_end.end(), buf->buffer[c+keywords[i].length()]))
        &&  (c-1 < 0 || std::count(check_chars_front.begin(), check_chars_front.end(), buf->buffer[c-1])))
          for (int j = c; j < c + keywords[i].length(); ++j)
            bufVec[j] = color_from_name("cppdefinitions");
      }
    }
// mathematical symbols
    keywords = { 
      "{", "}", "(", ")", "*", "&", "/", "+", "-", "=", "!", ">", "<", 
      ":", ",", ".", "^"
    };
    for (int i = 0; i < keywords.size(); ++i) {
      size_t c = -1;
      while (buf->buffer.find(keywords[i], c+1) != std::string::npos) {
        c = buf->buffer.find(keywords[i], c+1);
        for (int j = c; j < c + keywords[i].length(); ++j)
          bufVec[j] = color_from_name("cppsymbols");
      }
    }
  
// unclosed brackets
  std::vector<char> opens = {
      '{', '(', '['
  };
  std::vector<char> closeds = {
      '}', ')', ']'
  };
  highlightUnclosedBrackets(opens, closeds, buf);
// yellow: chars
  highlightCharacters(buf);
// green: comments, strings
  highlightSingleLineComments(buf);
  highlightMultiLineComments('(', ')', buf);
  highlightStrings('\"', buf);
}

void Syntax::highlightJavascript(TextBuffer* buf) {
    color_t fg_color = color_from_name("workspacedefaultfg");
    int ix = 0;
// prepocessor directives
    std::vector<std::string> keywords = { 
      "constructor", "Error", "Number", "String", "Boolean", "Date", "Function", "instanceof", "typeof"
    };
    for (int i = 0; i < keywords.size(); ++i) {
      size_t c = -1;
      while (buf->buffer.find(keywords[i], c+1) != std::string::npos) {
        c = buf->buffer.find(keywords[i], c+1);
        for (int j = c; j < c + keywords[i].length(); ++j)
          bufVec[j] = color_from_name("cppdirectives");
      }
    }
// purple (actions)
    keywords = {
      "while", "with", "goto", "do", "for", "if", "else", "switch", "break", "delete", "return", "finally", "try", "catch"
    };
    for (int i = 0; i < keywords.size(); ++i) {
      std::vector<char> check_chars_end = {'(', ' ', '\n', '\t', ';', '{', '}', ':'};
      std::vector<char> check_chars_front = {' ', '\n', '\t', ';', '{', '}'};
      size_t c = -1;
      while (buf->buffer.find(keywords[i], c+1) != std::string::npos) {
        c = buf->buffer.find(keywords[i], c+1);
        if ((c+keywords[i].length() >= buf->buffer.length() 
	|| std::count(check_chars_end.begin(), check_chars_end.end(), buf->buffer[c+keywords[i].length()])
        )&&(c-1 < 0 || std::count(check_chars_front.begin(), check_chars_front.end(), buf->buffer[c-1]))) {
          for (int j = c; j < c + keywords[i].length(); ++j)
            bufVec[j] = color_from_name("cppstatements");
        }
      }
    }
// blue (data)
    keywords = { 
      "new", "var", "let", "function", "const", "case", "Infinity", "NaN", "null", "undefined", "true", "false", "this", "class"  };
    for (int i = 0; i < keywords.size(); ++i) {
      std::vector<char> check_chars_end = {'>', '<', '=', ')', '(', '[', '-', '{', ' ', ';', '.', '&', ':', ',', '\n', '\t'};
      std::vector<char> check_chars_front = {'<', '=', ' ', '(', ')', ',', ';', '{', '*', '\n', '\t'};
      size_t c = -1;
      while (buf->buffer.find(keywords[i], c+1) != std::string::npos) {
        c = buf->buffer.find(keywords[i], c+1);
        if ((c+keywords[i].length() >= buf->buffer.length() || std::count(check_chars_end.begin(), check_chars_end.end(), buf->buffer[c+keywords[i].length()]))
        &&  (c-1 < 0 || std::count(check_chars_front.begin(), check_chars_front.end(), buf->buffer[c-1])))
          for (int j = c; j < c + keywords[i].length(); ++j)
            bufVec[j] = color_from_name("cppdefinitions");
      }
    }
// mathematical symbols
    keywords = { 
      "{", "}", "(", ")", "*", "&", "/", "+", "-", "=", "!", ">", "<", 
      ":", ",", ".", "^"
    };
    for (int i = 0; i < keywords.size(); ++i) {
      size_t c = -1;
      while (buf->buffer.find(keywords[i], c+1) != std::string::npos) {
        c = buf->buffer.find(keywords[i], c+1);
        for (int j = c; j < c + keywords[i].length(); ++j)
          bufVec[j] = color_from_name("cppsymbols");
      }
    }

// unclosed brackets
  std::vector<char> opens = {
      '{', '(', '['
  };
  std::vector<char> closeds = {
      '}', ')', ']'
  };
  highlightUnclosedBrackets(opens, closeds, buf);
// green: comments, strings
  highlightSingleLineComments(buf);
  highlightMultiLineComments('/', '/', buf);
  highlightStrings('\"', buf, '\'');
}

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
  ||  (filename.find(".h", 0)   != std::string::npos 
   && filename.find(".html",0) == std::string::npos)
  ||  filename.find(".hpp", 0) != std::string::npos
  ||  filename.find(".cxx", 0)	!= std::string::npos
  ||  filename.find(".c", 0) != std::string::npos)
    highlightCpp(buf);
  else if (filename.find(".ml", 0) != std::string::npos
       ||  filename.find(".mli", 0) != std::string::npos)
    highlightOcaml(buf);
  else if (filename.find(".js", 0) != std::string::npos)
    highlightJavascript(buf);
}
