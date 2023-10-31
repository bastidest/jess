#pragma once

#include "NcWindow.hpp"
#include <ncurses.h>

namespace jess {
class NcTerminal {
public:
  NcTerminal() {
    ::initscr();
    ::cbreak();
    ::noecho();
  }

  ~NcTerminal() {
    ::refresh();
    ::endwin();
  }

  static void cbreak() { ::cbreak(); }
  static void nocbreak() { ::nocbreak(); }
  static void echo() { ::echo(); }
  static void noecho() { ::noecho(); }

  NcWindow rootWindow() { return NcWindow{stdscr}; }
};

} // namespace jess
