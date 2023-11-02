#pragma once

#include "NcError.hpp"

#include <ncurses.h>

#include <cstddef>
#include <cstring>
#include <memory>
#include <optional>

namespace jess {

struct WindowDeleter {
  void operator()(WINDOW *ptr) { NC_CHECK_RC(delwin(ptr)); }
};

class NcWindow {
  friend class NcTerminal;

  std::unique_ptr<WINDOW, WindowDeleter> handle{};

  explicit NcWindow(WINDOW *ptr) {
    NC_CHECK_PTR(ptr);
    handle.reset(ptr);
  }

public:
  NcWindow(size_t uHeight, size_t uWidth, size_t uPosY, size_t uPosX) {
    handle.reset(::newwin(static_cast<int>(uHeight), static_cast<int>(uWidth), static_cast<int>(uPosY),
                          static_cast<int>(uPosX)));
    NC_CHECK_PTR(handle.get());
  }

  [[nodiscard]] size_t width() const { return getmaxx(handle.get()); }
  [[nodiscard]] size_t height() const { return getmaxy(handle.get()); }
  [[nodiscard]] size_t cursorPosY() const { return getcury(handle.get()); }
  [[nodiscard]] size_t cursorPosX() const { return getcurx(handle.get()); }
  void setKeypad(bool bEnable) { NC_CHECK_RC(::keypad(handle.get(), bEnable)); }
  void move(size_t uPosY, size_t uPosX) { NC_CHECK_RC(::wmove(handle.get(), uPosY, uPosX)); }
  template <typename... Args> void printw(const char *formatStr, Args... args) {
    NC_CHECK_RC(::wprintw(handle.get(), formatStr, std::forward<Args>(args)...));
  }
  void clear() { NC_CHECK_RC(::wclear(handle.get())); }
  void clearToEol() {
//    if(cursorPosX() + 10 >= width()) {
//      return;
//    }
    ::wclrtoeol(handle.get());
  }
  void clearToBot() { NC_CHECK_RC(::wclrtobot(handle.get())); }
  void refresh() { NC_CHECK_RC(::wrefresh(handle.get())); }
  int getChar() { return ::wgetch(handle.get()); }
  std::optional<std::string> getString() {
    constexpr size_t uMaxSize = 1024;
    std::string ret(uMaxSize, 0);
    NC_CHECK_RC(::wgetnstr(handle.get(), ret.data(), static_cast<int>(ret.size())));
    size_t uActualLength = ::strnlen(ret.data(), uMaxSize);
    ret.resize(uActualLength);
    return ret;
  }
  void setNodelay(bool bNodelay) {
    ::nodelay(handle.get(), bNodelay);
  }
  void enableAttributes(int attributes) {
    NC_CHECK_RC(::wattron(handle.get(), attributes));
  }
  void disableAttributes(int attributes) {
    NC_CHECK_RC(::wattroff(handle.get(), attributes));
  }
};

} // namespace jess
