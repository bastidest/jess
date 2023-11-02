#include <iostream>
#include <limits>

#include "JessMain.hpp"
#include "Modeline.hpp"
#include "NcWindow.hpp"

#include <ncurses.h>


void run() {
  jess::JessMain main{};
  using jess::ctrl;
  using jess::meta;
  using key = jess::KeyCombination;

  bool bContinue = true;

  while (bContinue) {
    auto kc = main.getNextKey();

    if (kc == ctrl('p') || kc == key(KEY_UP)) {
      main.scrollUpLine();
      continue;
    }

    if (kc == ctrl('n') || kc == key(KEY_DOWN)) {
      main.scrollDownLine();
      continue;
    }

    if (kc == key('b') || kc == meta('v') || kc == key(KEY_PPAGE)) {
      main.scrollUpPage();
      continue;
    }

    if (kc == key('f') || kc == ctrl('v') || kc == key(KEY_NPAGE)) {
      main.scrollDownPage();
      continue;
    }

    if (kc == key(':')) {
      main.activateModeline();
      continue;
    }

    if (kc == key('q')) {
      bContinue = false;
      continue;
    }
  }
}

int main() {
  try {
    run();
  } catch (const jess::NcError &ex) {
    std::cerr << "error: " << ex.what() << std::endl;
  }
  return 0;
}
