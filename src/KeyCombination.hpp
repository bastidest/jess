#pragma once

namespace jess {
struct KeyCombination {
  int key = 0;
  bool bCtrl = false;
  bool bMeta = false;

  constexpr KeyCombination() = default;
  explicit constexpr KeyCombination(int key) : key(key){};
  explicit constexpr KeyCombination(int key, bool bCtrl, bool bMeta) : key(key), bCtrl(bCtrl), bMeta(bMeta){};

  bool operator==(const KeyCombination &rhs) const {
    return key == rhs.key && bCtrl == rhs.bCtrl && bMeta == rhs.bMeta;
  }
  bool operator!=(const KeyCombination &rhs) const { return !(rhs == *this); }

  [[nodiscard]] KeyCombination operator()(int key_) const { return KeyCombination{key_, bCtrl, bMeta}; }

  [[nodiscard]] KeyCombination operator+(KeyCombination other) const {
    return KeyCombination{key | other.key, static_cast<bool>(bCtrl | other.bCtrl),
                          static_cast<bool>(bMeta | other.bMeta)};
  }
};

constexpr KeyCombination ctrl{0, true, false};
constexpr KeyCombination meta{0, false, true};

} // namespace jess
