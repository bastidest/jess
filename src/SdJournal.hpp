#pragma once

#include <memory>
#include <systemd/sd-journal.h>

namespace jess {

struct JournalDeleter {
  void operator()(sd_journal *ptr) { sd_journal_close(ptr); }
};


class SdJournal {
  std::unique_ptr<sd_journal, JournalDeleter> handle{};

public:
  explicit SdJournal() {
    sd_journal *pJournal{};
    sd_journal_open(&pJournal, 0);
    handle.reset(pJournal);
  };
};
}