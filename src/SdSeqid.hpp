#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace jess {

template <class T, class Tag> struct StrongTypedef {
  T value;
  operator T &() { return value; }
  bool operator==(const StrongTypedef<T, Tag> &other) const { return value == other.value; }
  auto operator<=>(const StrongTypedef<T, Tag> &other) const { return value <=> other.value; }
};

using SdSeqnumId = StrongTypedef<std::array<uint8_t, 16>, class SdSeqnumIdTag>;
using SdSeqnum = StrongTypedef<size_t, class SdSeqnumTag>;

struct SdSeqid {
  SdSeqnumId seqnumId;
  SdSeqnum seqnum;

  bool operator==(const SdSeqid &other) const { return seqnumId == other.seqnumId && seqnum == other.seqnum; }

  std::partial_ordering operator<=>(const SdSeqid &other) const {
    if (seqnumId != other.seqnumId) {
      return std::partial_ordering::unordered;
    }
    return seqnum <=> other.seqnum;
  }
};

} // namespace jess
