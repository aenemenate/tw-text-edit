# tw-text-edit
  A (currently) Windows-only barebone terminal-like text editor written in C++. 
I am approaching development in this way: only implementing a feature as 
needed and if I can make a clear statement for its use, or otherwise produce a 
valid use case. Furthermore, every feature will be implemented according to 
standard Windows design philosophy, which means file menus, right click context
menus, and standard key commands. Imo this is what sets my editor apart from
the current offerings, and why I aimed to create it. It is aimed at a newer
generation of programmers who have their own built-in expectations when it comes
to interacting with programs, but would like to experience the same lightning-fast 
and precise editing of yesteryears.

  Niceties: custom & preincluded theme files, integration with windows file
explorer, mouse support.

  Eventually I would like this to function as a complete development environment
for Windows and Linux, simply so I can use it myself. In order to achieve this 
end it should be user customizeable, which means extensible syntax highlighting 
and build pipelines. It will also need to support project files, git operations,
and possibly code/folder generation.

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
- more complete directory viewer (move, create, delete files/directories)
- syntax highlighting for more languages (makefiles, ini, batch files)
- git -> (init, add, commit, push, pull, etc.)
- symbol tracking for C++
- autocomplete
- Linux support
- language highlighting extension files
- ncurses support
