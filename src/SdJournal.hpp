#pragma once

#include "SdCursor.hpp"
#include "SdLine.hpp"
#include <map>
#include <memory>
#include <systemd/sd-journal.h>
#include <unordered_map>

namespace jess {

struct JournalDeleter {
  void operator()(sd_journal *ptr) { sd_journal_close(ptr); }
};

class SdJournal {
  std::unique_ptr<sd_journal, JournalDeleter> handle{};
  std::map<SdCursor, SdLine> m_lineCache{};

public:
  explicit SdJournal() {
    sd_journal *pJournal{};
    sd_journal_open(&pJournal, 0);
    handle.reset(pJournal);
  };

  [[nodiscard]] std::string getCursor() {
    char *cursor;
    sd_journal_get_cursor(handle.get(), &cursor);
    std::string ret{cursor};
    free(cursor);
    return ret;
  }

  void seekToCursor(const std::string &sCursor) { sd_journal_seek_cursor(handle.get(), sCursor.c_str()); }

  void seekToBeginning() {
    sd_journal_seek_head(handle.get());
    sd_journal_next(handle.get());
  }

  void seekToEnd() {
    sd_journal_seek_tail(handle.get());
    sd_journal_next(handle.get());
  }

  void seekBack(size_t uNumLines) {
    sd_journal_previous_skip(handle.get(), uNumLines);
    sd_journal_next(handle.get());
  }

  void seekForward(size_t uNumLines) {
    sd_journal_next_skip(handle.get(), uNumLines);
    sd_journal_next(handle.get());
  }

  std::vector<SdLine> getLines(size_t uNumLines) {
    std::vector<SdLine> ret{};
    ret.reserve(uNumLines);

    for (size_t i = 0; i < uNumLines && sd_journal_next(handle.get()) > 0; ++i) {
      const void *ptr = nullptr;
      size_t uMessageLength{};
      //      SD_JOURNAL_FOREACH_DATA(handle.get(), ptr, uMessageLength) {
      //        std::string foobar{static_cast<const char*>(ptr)};
      //        std::cout << foobar << std::endl;
      //      }

      sd_journal_get_data(handle.get(), "MESSAGE", reinterpret_cast<const void **>(&ptr), &uMessageLength);
      std::string sMessage{static_cast<const char *>(ptr)};
      sd_journal_get_data(handle.get(), "MESSAGE_ID", reinterpret_cast<const void **>(&ptr), &uMessageLength);
      std::string sRealtimeTimestamp{static_cast<const char *>(ptr)};
      ret.emplace_back(sMessage, sRealtimeTimestamp);
    }

    return ret;
  }
};
} // namespace jess
