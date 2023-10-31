#include <iostream>
#include <limits>
#include <vector>

#include "NcTerminal.hpp"
#include "NcWindow.hpp"
#include <array>
#include <ncurses.h>

#include <systemd/sd-journal.h>

#define KEY_CTRL(x) ((x)&0x1f)

std::vector<std::string> getFakeLines() {
  std::vector<std::string> ret{};

  sd_journal *pJournal{};
  sd_journal_open(&pJournal, 0);

  size_t uNumLines = 1000;
  ret.reserve(uNumLines);
  for (size_t i = 0; i < uNumLines; ++i) {
    sd_journal_next(pJournal);
    const char *sMessage = nullptr;
    size_t uMessageLength{};
    sd_journal_get_data(pJournal, "MESSAGE", reinterpret_cast<const void **>(&sMessage), &uMessageLength);
    ret.push_back(sMessage);
  }

  sd_journal_close(pJournal);

  return ret;
}

void drawLinesAtOffset(const std::vector<std::string> &lines, jess::NcWindow &window, size_t offset) {
  size_t windowHeight = window.height();
  window.move(0, 0);
  for (size_t i = 0; i < windowHeight; ++i) {
    size_t uLineIndex = offset + i;
    if (uLineIndex < lines.size()) {
      window.move(i, 0);
      window.printw("%s", lines.at(offset + i).c_str());
      window.clearToEol();
    } else {
      window.clearToBot();
      break;
    }
  }
}

void run() {
  jess::NcTerminal rootTerminal{};
  auto rootWindow = rootTerminal.rootWindow();

  size_t uCurrentOffset = 0;
  auto someLines = getFakeLines();

  jess::NcWindow mainWindow{rootWindow.height() - 1, rootWindow.width(), 0, 0};
  mainWindow.refresh();
  drawLinesAtOffset(someLines, mainWindow, uCurrentOffset);
  mainWindow.refresh();

  jess::NcWindow cliWindow(1, rootWindow.width(), rootWindow.height() - 1, 0);
  cliWindow.printw(":");
  cliWindow.refresh();
  cliWindow.setKeypad(true);

  bool bContinue = true;

  while (bContinue) {
    size_t uLastOffset = uCurrentOffset;
    int in = cliWindow.getChar();
    bool bMeta = false;

    if (in == 27) {
      bMeta = true;
      in = cliWindow.getChar();
    }

    switch (in) {
    case KEY_UP:
      if (uCurrentOffset != std::numeric_limits<size_t>::min()) {
        uCurrentOffset -= 1;
      }
      break;
    case KEY_DOWN:
      if (uCurrentOffset != std::numeric_limits<size_t>::max()) {
        uCurrentOffset += 1;
      }
      break;
    case 'v':
      if (!bMeta) {
        continue;
      }
      [[fallthrough]];
    case 'b':
    case KEY_PPAGE:
      if (uCurrentOffset < mainWindow.height()) {
        uCurrentOffset = 0;
      } else {
        uCurrentOffset -= mainWindow.height();
      }
      break;
    case ' ':
    case 'f':
    case KEY_CTRL('f'):
    case KEY_CTRL('v'):
    case KEY_NPAGE:
      //                if (uCurrentOffset < mainWindowHeight) {
      //                    uCurrentOffset = 0;
      //                }
      uCurrentOffset += mainWindow.height();
      break;
    case ':': {
      cliWindow.move(0, 0);
      cliWindow.printw(" :");
      jess::NcTerminal::echo();
      cliWindow.getString();
      cliWindow.clear();
      cliWindow.printw(":");
      jess::NcTerminal::noecho();
      break;
    }
    case 'q':
      bContinue = false;
      break;
    default:
      break;
    }

    if (uCurrentOffset != uLastOffset) {
      drawLinesAtOffset(someLines, mainWindow, uCurrentOffset);
      mainWindow.refresh();
      cliWindow.move(0, 1);
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
