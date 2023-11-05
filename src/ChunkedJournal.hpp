#pragma once

#include "SdCursor.hpp"
#include "SdJournal.hpp"
#include "SdLine.hpp"

#include <cassert>
#include <list>
#include <span>
#include <vector>

namespace jess {

static constexpr auto uChunkSize = 1024;

class ChunkedJournal {
  enum class Contiguity {
    CONTIGUOUS,
    NON_CONTIGUOUS,
    OVERLAPPING,
  };

  enum class InsertPosition {
    BEFORE,
    AFTER,
  };

  struct Chunk {
    // we might need multiple cursors depending on the sequence id
    SdCursor beginning;
    SdCursor end;
    std::vector<SdLine> lines;
    Contiguity contiguityBeginning{Contiguity::NON_CONTIGUOUS};
    Contiguity contiguityEnd{Contiguity::NON_CONTIGUOUS};

    std::partial_ordering operator<=>(const Chunk &other) const { return beginning <=> other.beginning; }
  };

  SdJournal m_journal{};
  std::list<Chunk> m_chunks{};
  decltype(m_chunks.begin()) m_pCurrentChunk{m_chunks.begin()};
  size_t m_uLineOffsetInChunk{0};

  auto createChunkFromCurrentPosition(const SdCursor &cursor, InsertPosition insertPos, Contiguity contiguity)
      -> decltype(m_chunks.begin()) {
    Chunk newChunk{cursor, cursor, {}};

    newChunk.lines.reserve(uChunkSize);

    for (size_t i = 0; i < uChunkSize; ++i) {
      newChunk.lines.push_back(m_journal.getLine());
      if (!m_journal.next()) {
        break;
      }
    }

    newChunk.end = m_journal.getCursor();

    auto insertIt = m_pCurrentChunk;
    if (insertPos == InsertPosition::AFTER) {
      std::advance(insertIt, 1);
    }
    auto newChunkIt = m_chunks.insert(insertIt, std::move(newChunk));

    if (insertPos == InsertPosition::BEFORE) {
      newChunkIt->contiguityEnd = contiguity;
      if(m_pCurrentChunk != m_chunks.end()) {
        m_pCurrentChunk->contiguityBeginning = contiguity;
      }
    } else if (insertPos == InsertPosition::AFTER) {
      if(m_pCurrentChunk != m_chunks.end()) {
        m_pCurrentChunk->contiguityEnd = contiguity;
      }
      newChunkIt->contiguityBeginning = contiguity;
    }

    return newChunkIt;
  }

public:
  void seekToBof() {
    m_journal.seekToBof();
    m_journal.next();
    auto cursorBof = m_journal.getCursor();

    if (m_chunks.empty() || m_chunks.front().beginning == cursorBof) {
      m_pCurrentChunk = createChunkFromCurrentPosition(cursorBof, InsertPosition::BEFORE, Contiguity::NON_CONTIGUOUS);
    }

    m_uLineOffsetInChunk = 0;
  }

  void seekLines(int64_t uNumLines) {
    assert(m_pCurrentChunk != m_chunks.end());

    if (uNumLines < 0 && static_cast<size_t>(std::abs(uNumLines)) > m_uLineOffsetInChunk) {
      m_uLineOffsetInChunk = 0;
      // todo: load previous chunk
      return;
    }
    if (m_uLineOffsetInChunk + uNumLines > m_pCurrentChunk->lines.size()) {
      int64_t uLinesAfterEndOfChunk =
          uNumLines - static_cast<int64_t>(m_pCurrentChunk->lines.size() - m_uLineOffsetInChunk);
      size_t uNumChunksToSkip = uLinesAfterEndOfChunk / uChunkSize;
      m_uLineOffsetInChunk = uLinesAfterEndOfChunk - uNumChunksToSkip * uChunkSize;
      m_journal.seekForward(uNumChunksToSkip * uChunkSize);
      m_journal.next();
      auto cursorBeginningOfChunk = m_journal.getCursor();
      Contiguity contiguity = uNumChunksToSkip == 0 ? Contiguity::CONTIGUOUS : Contiguity::NON_CONTIGUOUS;
      m_pCurrentChunk = createChunkFromCurrentPosition(cursorBeginningOfChunk, InsertPosition::AFTER, contiguity);
      return;
    }

    m_uLineOffsetInChunk += uNumLines;
  }

  std::span<SdLine> getLines(size_t uNumLines) {
    if (m_pCurrentChunk == m_chunks.end()) {
      return {};
    }
    std::span ret{m_pCurrentChunk->lines};
    return ret.subspan(m_uLineOffsetInChunk, std::min(ret.size() - m_uLineOffsetInChunk, uNumLines));
  }
};

} // namespace jess
