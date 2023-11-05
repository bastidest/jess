#pragma once

#include "NcWindow.hpp"
#include <span>
#include <vector>
namespace jess {

class MainFrame {
  jess::NcWindow &m_rootWindow;
  jess::NcWindow m_mainWindow{m_rootWindow.height() - 1, m_rootWindow.width(), 0, 0};

public:
  explicit MainFrame(jess::NcWindow &rootWindow) : m_rootWindow(rootWindow) {}

  void drawLines(auto lines) {
    auto it = std::begin(lines);
    auto end = std::end(lines);

    size_t windowHeight = m_mainWindow.height();
    m_mainWindow.move(0, 0);
    for (size_t i = 0; i < windowHeight && it != end; ++i, ++it) {
      m_mainWindow.move(i, 0);
      m_mainWindow.printw("%s ", it->realtimeUtc().c_str());
      m_mainWindow.printw("%s", it->message().c_str());
      m_mainWindow.clearToEol();
    }

    m_mainWindow.clearToBot();

    m_mainWindow.refresh();
  }

  [[nodiscard]] size_t height() const { return m_mainWindow.height(); }
};

} // namespace jess
