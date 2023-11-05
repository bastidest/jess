#pragma once

#include "SdCursor.hpp"
#include "SdJournal.hpp"
#include "SdLine.hpp"

#include <list>
#include <span>
#include <vector>

namespace jess {

static constexpr auto uChunkSize = 1024;

class ChunkedJournal {
  struct Chunk {
    // we might need multiple cursors depending on the sequence id
    SdCursor beginning;
    SdCursor end;
    std::vector<SdLine> lines;

    std::partial_ordering operator<=>(const Chunk &other) const { return beginning <=> other.beginning; }
  };

  SdJournal m_journal{};
  std::list<Chunk> m_chunks{};
  Chunk *m_pCurrentChunk{nullptr};
  size_t m_uLineOffsetInChunk{0};

  auto addChunk(Chunk chunk) -> decltype(m_chunks.begin()) {
    auto it = std::upper_bound(m_chunks.begin(), m_chunks.end(), chunk);
    return m_chunks.insert(it, std::move(chunk));
  }

  auto loadChunk(const SdCursor &cursor) -> decltype(m_chunks.begin()) {
    Chunk newChunk{cursor, cursor, {}};

    newChunk.lines.reserve(uChunkSize);

    for (size_t i = 0; i < uChunkSize; ++i) {
      newChunk.lines.push_back(m_journal.getLine());
      if (!m_journal.next()) {
        break;
      }
    }

    newChunk.end = m_journal.getCursor();

    return addChunk(std::move(newChunk));
  }

public:

  void seekToBof() {
    m_journal.seekToBof();
    m_journal.next();
    auto cursorBof = m_journal.getCursor();

    if (m_chunks.empty() || m_chunks.front().beginning == cursorBof) {
      m_pCurrentChunk = loadChunk(cursorBof).operator->();
    }

    m_pCurrentChunk = &m_chunks.front();
    m_uLineOffsetInChunk = 0;
  }

  void seekLines(int64_t uNumLines) {
    if(uNumLines < 0 && static_cast<size_t>(std::abs(uNumLines)) > m_uLineOffsetInChunk) {
      m_uLineOffsetInChunk = 0;
      // todo: modulo logic?
      // todo: load previous chunk
      return;
    }
    if(m_uLineOffsetInChunk + uNumLines > uChunkSize) {
      m_uLineOffsetInChunk = 0;
      // todo: modulo logic?
      // todo: load next chunk
      return;
    }

    m_uLineOffsetInChunk += uNumLines;
  }

  std::span<SdLine> getLines(size_t uNumLines) {
    if (m_pCurrentChunk == nullptr) {
      return {};
    }
    std::span ret{m_pCurrentChunk->lines};
    return ret.subspan(m_uLineOffsetInChunk, std::min(ret.size() - m_uLineOffsetInChunk, uNumLines));
  }
};

} // namespace jess
