#include <string>
#include <utility>

#pragma once

namespace jess {
class SdLine {
public:
  explicit SdLine(std::string sMessage, std::string sRealtimeTimestamp)
      : sMessage(std::move(sMessage)), sRealtimeTimestamp(std::move(sRealtimeTimestamp)) {}

  [[nodiscard]] std::string message() const { return sMessage; }
  [[nodiscard]] std::string realtime() const { return sRealtimeTimestamp; }

private:
  std::string sMessage;
  std::string sRealtimeTimestamp;
};
} // namespace jess
