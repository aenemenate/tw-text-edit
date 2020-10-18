# tw-text-edit
  A simple cross-platform barebone faux-terminal text editor written in C++.
Every feature uses a standard key combination, and there is no needless complexity.
Mouse support is included, but not necessary. All features are accessible
through the keyboard.
 
  Niceties: custom & preincluded theme files, integration with windows file
explorer, runs batch files, mouse support.

  Eventually I would like this to function as a complete development environment
for Windows and Linux, simply so I can use it myself. In order to achieve this 
end it should be user customizeable, which means extensible syntax highlighting 
and build pipelines. I plan to implement syntax highlighting using [LSP](https://en.wikipedia.org/wiki/Language_Server_Protocol). 
It will also need to support project files, git operations, and possibly 
code/folder generation.

Why did I make this? Because a programmer must program, and reimplementing existing
technologies in a most likely deficient and honky way is a great way to
learn!

Dependencies:

All Systems: Boost Serialization static version

Linux only: xsel


# Portfolio

![Blank screen](https://i.imgur.com/q6KaHSP.png)

![Blank screen](https://i.imgur.com/FhlUIUo.png)

![Blank screen](https://i.imgur.com/ftp77zq.png)

## Features
- Open as many files as you want from the command line or windows explorer
- Edit multiple files at once
- Line numbers
- Convenient text interaction, mouse and keyboard navigation
- Right click context menu
- copy / paste
- highlight c++ keywords, prepocessor statements, strings, chars, and comments
- customizeable color themes
- undo / redo
- call batch files and view a log of the output

### Things I would like to implement:
- editor settings file
- more input options, like auto indent, block indent, toggle comment, etc.
- multiple on-screen buffers
- scroll bars
- more complete directory viewer (move, create, delete files/directories, scroll options)
- syntax highlighting for more languages (makefiles, ini, batch files)
- git -> (init, add, commit, push, pull, etc.)
- symbol tracking for C++
- autocomplete
- language highlighting extension files
- ncurses support
