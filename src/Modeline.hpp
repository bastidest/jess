#pragma once

#include "KeyCombination.hpp"
#include "NcTerminal.hpp"
#include "NcWindow.hpp"

namespace jess {
class Modeline {
  jess::NcWindow &m_rootWindow;
  jess::NcWindow m_cliWindow{1, m_rootWindow.width(), m_rootWindow.height() - 1, 0};

  static constexpr char KEY_ESC = 27;
  static constexpr char KEY_CTRL = 0x1f;

public:
  explicit Modeline(jess::NcWindow &rootWindow) : m_rootWindow(rootWindow) {
    m_cliWindow.printw(":");
    m_cliWindow.refresh();
    m_cliWindow.setKeypad(true);
  }

  std::optional<KeyCombination> getKeyCombination() {
    KeyCombination ret{};

    int in = m_cliWindow.getChar();

    // meta + <key> sequences are encoded as ESC + <key>
    if (in == KEY_ESC) {
      ret.bMeta = true;
      in = m_cliWindow.getChar();
    }

    if ((in & KEY_CTRL) == KEY_CTRL) {
      ret.bCtrl = true;
      in ^= KEY_CTRL;
    }

    ret.key = static_cast<int>(in & 0xffffU);

    return ret;
  }
  void setActive() {
    m_cliWindow.move(0, 0);
    m_cliWindow.printw(" :");
    m_cliWindow.clearToBot();
    jess::NcTerminal::echo();
    m_cliWindow.getString();
    m_cliWindow.move(0, 0);
    m_cliWindow.printw(":");
    m_cliWindow.clearToBot();
    jess::NcTerminal::noecho();
  }
  void focus() {
    m_cliWindow.move(0, 1);
  }
  void displayStatusString(const std::string &sStatus) {
    m_cliWindow.move(0, 0);
    m_cliWindow.enableAttributes(A_REVERSE);
    m_cliWindow.printw("%s", sStatus.c_str());
    m_cliWindow.disableAttributes(A_REVERSE);
    m_cliWindow.clearToBot();
    m_cliWindow.refresh();
  }
};
} // namespace jess
