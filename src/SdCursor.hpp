#pragma once

#include "SdSeqid.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string_view>

namespace jess {

template <bool bUppercase> constexpr uint8_t hexCharToNibble(char c) {
  constexpr char startOfAlphabet = bUppercase ? 'A' : 'a';
  const bool bIsLetter = static_cast<bool>( c & ( 0b1 << 6 ) );
  const uint8_t uRelativeOffset = c - '0';
  return uRelativeOffset - bIsLetter * (startOfAlphabet - '0' - 10);
}

template <bool bUppercase, size_t uSize>
constexpr std::array<uint8_t, uSize> hexStringToByteArray(std::string_view sHexString) {
  std::array<uint8_t, uSize> ret{};
  for (size_t i = 0; i < uSize; ++i) {
    ret[i] =
        (hexCharToNibble<bUppercase>(sHexString[i * 2 + 0]) << 4) | hexCharToNibble<bUppercase>(sHexString[i * 2 + 1]);
  }
  return ret;
}

struct SdCursor;
std::ostream &operator<<(std::ostream &os, const std::array<uint8_t, 16> &data);
std::ostream &operator<<(std::ostream &os, const SdCursor &that);

struct SdCursor {
  SdSeqid seqid;
  std::array<uint8_t, 16> bootId;
  size_t timeMonotonic;
  size_t timeRealtime;
  size_t someXor;

  [[nodiscard]] std::string toString() const {
    std::stringstream ss{};
    ss << *this;
    return ss.str();
  }

  bool operator==(const SdCursor &other) const { return seqid == other.seqid; }

  std::partial_ordering operator<=>(const SdCursor &other) const { return seqid <=> other.seqid; }

  static SdCursor fromString(std::string_view sCursor) {
    using namespace std::string_view_literals;
    SdCursor ret{};

    ret.seqid = {
        {parse<std::array<uint8_t, 16>>(sCursor, "s=")},
        {parse<std::size_t>(sCursor, "i=")},
    };
    ret.bootId = parse<std::array<uint8_t, 16>>(sCursor, "b=");
    ret.timeMonotonic = parse<std::size_t>(sCursor, "m=");
    ret.timeRealtime = parse<std::size_t>(sCursor, "t=");
    ret.someXor = parse<std::size_t>(sCursor, "x=");

    return ret;
  }

private:
  template <typename T> static T parse(std::string_view sCursorString, std::string_view sMarkerString) {
    auto markerPos = sCursorString.find(sMarkerString);
    if (markerPos != std::string_view::npos) {
      auto fieldBeginningPos = markerPos + sMarkerString.size();
      auto fieldEndPos = sCursorString.find(';', markerPos);
      auto sUuid = sCursorString.substr(fieldBeginningPos, fieldEndPos - fieldBeginningPos);
      if constexpr (std::is_same_v<T, std::array<uint8_t, 16>>) {
        return uuid128StringToArray(sUuid);
      }
      if constexpr (std::is_same_v<T, size_t>) {
        return stringToU64(sUuid);
      }
    }
    throw std::runtime_error{std::string{sMarkerString} + std::string{" not found"}};
  }

  static std::array<uint8_t, 16> uuid128StringToArray(std::string_view sUuid) {
    if (sUuid.size() != 32) {
      throw std::runtime_error{"expected hex string with 32 characters"};
    }

    return hexStringToByteArray<false, 16>(sUuid);
  }

  static size_t stringToU64(std::string_view sNumber) { return std::strtoull(sNumber.cbegin(), nullptr, 16); }
};

} // namespace jess
