#include "SdCursor.hpp"


std::ostream &jess::operator<<(std::ostream &os, const std::array<uint8_t, 16> &data) {
  for (uint8_t num : data) {
    os << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(num);
  }
  os << std::setw(0);
  return os;
}

std::ostream &jess::operator<<(std::ostream &os, const jess::SdCursor &that) {
  os << std::hex;
  os << "s=" << that.seqid.seqnumId.value << ";";
  os << "i=" << that.seqid.seqnum.value << ";";
  os << "b=" << that.bootId << ";";
  os << "m=" << that.timeMonotonic << ";";
  os << "t=" << that.timeRealtime << ";";
  os << "x=" << that.someXor;
  return os;
}
