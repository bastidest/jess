#pragma once

#include "MainFrame.hpp"
#include "Modeline.hpp"
#include "NcTerminal.hpp"

#include <systemd/sd-journal.h>

#include <vector>

namespace jess {

class JessMain {
  jess::NcTerminal m_rootTerminal{};
  jess::NcWindow m_rootWindow = m_rootTerminal.rootWindow();
  jess::Modeline m_modeline{m_rootWindow};
  jess::MainFrame m_mainFrame{m_rootWindow};
  size_t m_uCurrentOffset = 0;
  bool m_bModelineActive = false;
  std::vector<std::string> m_someLines = getFakeLines();

  static std::vector<std::string> getFakeLines() {
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

public:
  KeyCombination getNextKey() { return m_modeline.getKeyCombination().value(); }
  void scrollUpLine() {
    if(m_uCurrentOffset == std::numeric_limits< decltype(m_uCurrentOffset)>::min()) {
      return;
    }
    m_uCurrentOffset -= 1;
    redrawTranslation();
  }
  void scrollDownLine() {
    if(m_uCurrentOffset == std::numeric_limits< decltype(m_uCurrentOffset)>::max()) {
      return;
    }
    m_uCurrentOffset += 1;
    redrawTranslation();
  }
  void scrollUpPage() {
    if(m_uCurrentOffset < m_mainFrame.height()) {
      m_uCurrentOffset = 0;
    } else {
      m_uCurrentOffset -= m_mainFrame.height();
    }
    redrawTranslation();
  }
  void scrollDownPage() {
    m_uCurrentOffset += m_mainFrame.height();
    redrawTranslation();
  }
  void activateModeline() {
    m_bModelineActive = true;
    m_modeline.setActive();
    m_bModelineActive = false;
  }

private:
  void redraw() {
    auto it = m_someLines.begin();
    size_t advancement = std::min(m_uCurrentOffset, m_someLines.size());
    std::advance(it, advancement);
    m_mainFrame.drawLines(std::span<std::string>{it, m_someLines.end()});
    m_modeline.focus();
  }
  void redrawTranslation() {
    redraw();
    displayOffset();
  }
  void displayOffset() {
    m_modeline.displayStatusString("line " + std::to_string(m_uCurrentOffset));
  }
};

} // namespace jess