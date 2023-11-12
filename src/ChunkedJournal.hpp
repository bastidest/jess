#pragma once

#include "CSeekableStream.hpp"
#include "SdCursor.hpp"
#include "SdJournal.hpp"
#include "SdLine.hpp"

#include <cassert>
#include <list>
#include <span>
#include <vector>

namespace jess
{

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
  std::map<SdSeqnumId, SdSeqnum> lowestIdsInChunk{};
  std::map<SdSeqnumId, SdSeqnum> highestIdsInChunk{};
  std::vector<SdLine> lines;
  Contiguity contiguityBeginning{ Contiguity::NON_CONTIGUOUS };
  Contiguity contiguityEnd{ Contiguity::NON_CONTIGUOUS };
};

template<SeekableStream TJournal>
class ChunkedJournal
{
  size_t m_uChunkSize;
  size_t m_uPreloadLines;
  TJournal m_journal{};
  std::list<Chunk> m_chunks{};
  decltype( m_chunks.begin() ) m_pCurrentChunk{ m_chunks.begin() };
  size_t m_uLineOffsetInChunk{ 0 };

public:
  explicit ChunkedJournal( const size_t uChunkSize, const size_t uPreloadLines )
    : m_uChunkSize( uChunkSize )
    , m_uPreloadLines( uPreloadLines )
  {
  }

  const auto& getChunks() const { return m_chunks; }

private:
  auto createChunkAtCurrentPosition( const InsertPosition insertPos, const Contiguity contiguity ) -> decltype( m_chunks.begin() )
  {
    Chunk newChunk{ {}, {}, {} };

    newChunk.lines.reserve( m_uChunkSize );

    for ( size_t i = 0; i < m_uChunkSize; ++i )
    {
      newChunk.lines.push_back( m_journal.getLine() );
      const SdSeqnumId key = newChunk.lines.back().seqid().seqnumId;
      const SdSeqnum value = newChunk.lines.back().seqid().seqnum;

      // if we have not seen the seqnumid before, add the first seqnum we encountered
      if ( !newChunk.lowestIdsInChunk.contains( key ) )
      {
        newChunk.lowestIdsInChunk[key] = value;
        newChunk.highestIdsInChunk[key] = value;
      }

      // if we know the seqnumid already, update the last known seqnum
      if ( newChunk.highestIdsInChunk.contains( key ) )
      {
        newChunk.highestIdsInChunk[key] = value;
      }

      if ( !m_journal.next() )
      {
        break;
      }
    }

    auto insertIt = m_pCurrentChunk;
    if ( insertPos == InsertPosition::AFTER )
    {
      std::advance( insertIt, 1 );
    }
    auto newChunkIt = m_chunks.insert( insertIt, std::move( newChunk ) );

    if ( insertPos == InsertPosition::BEFORE )
    {
      newChunkIt->contiguityEnd = contiguity;
      if ( m_pCurrentChunk != m_chunks.end() )
      {
        m_pCurrentChunk->contiguityBeginning = contiguity;
      }
    }
    else if ( insertPos == InsertPosition::AFTER )
    {
      if ( m_pCurrentChunk != m_chunks.end() )
      {
        m_pCurrentChunk->contiguityEnd = contiguity;
      }
      newChunkIt->contiguityBeginning = contiguity;
    }

    return newChunkIt;
  }

  [[nodiscard]] std::vector<decltype( m_chunks.begin() )> getChunksBySeqid( SdSeqid seqid )
  {
    std::vector<decltype( m_chunks.begin() )> ret{};
    for ( auto chunkIt = m_chunks.begin(); chunkIt != m_chunks.end(); ++chunkIt )
    {
      if ( auto highestIt = chunkIt->highestIdsInChunk.find( seqid.seqnumId );
           highestIt != chunkIt->highestIdsInChunk.end() && seqid.seqnum <= highestIt->second )
      {
        if ( auto lowestIt = chunkIt->lowestIdsInChunk.find( seqid.seqnumId ); lowestIt == chunkIt->lowestIdsInChunk.end() || seqid.seqnum >= lowestIt->second )
        {
          ret.push_back( chunkIt );
        }
      }
    }
    return ret;
  }

  void loadChunkAtCurrentPosition( const InsertPosition insertPos, const Contiguity contiguity )
  {
    const auto seqid = m_journal.getSeqid();
    if ( auto chunks = getChunksBySeqid( seqid ); !chunks.empty() )
    {
      m_pCurrentChunk = chunks.back();
    }
    else
    {
      m_pCurrentChunk = createChunkAtCurrentPosition( insertPos, contiguity );
    }
  }

public:
  void seekToBof()
  {
    m_journal.seekToBof();
    m_journal.next();
    loadChunkAtCurrentPosition( InsertPosition::BEFORE, Contiguity::CONTIGUOUS );
    m_uLineOffsetInChunk = 0;
  }

  void seekLines( const int64_t uNumLines )
  {
    assert( m_pCurrentChunk != m_chunks.end() );

    if ( uNumLines < 0 && static_cast<size_t>( std::abs( uNumLines ) ) > m_uLineOffsetInChunk )
    {
      m_uLineOffsetInChunk = 0;
      // todo: load previous chunk
      return;
    }
    if ( m_uLineOffsetInChunk + uNumLines >= m_pCurrentChunk->lines.size() )
    {
      const int64_t uNewOffsetRelativeToEndOfCurrentChunk = uNumLines - static_cast<int64_t>( m_pCurrentChunk->lines.size() - m_uLineOffsetInChunk );
      const size_t uNumChunksToSkip = uNewOffsetRelativeToEndOfCurrentChunk / m_uChunkSize;
      m_uLineOffsetInChunk = uNewOffsetRelativeToEndOfCurrentChunk - uNumChunksToSkip * m_uChunkSize;

      if ( const size_t uLinesToSeek = uNumChunksToSkip * m_uChunkSize; uLinesToSeek > 0)
      {
        m_journal.seekLinesForward( uLinesToSeek );
        m_journal.next();
      }

      const Contiguity contiguity = uNumChunksToSkip == 0 ? Contiguity::CONTIGUOUS : Contiguity::NON_CONTIGUOUS;
      loadChunkAtCurrentPosition( InsertPosition::AFTER, contiguity );
      return;
    }

    m_uLineOffsetInChunk += uNumLines;
  }

  std::span<SdLine> getLines( size_t uNumLines )
  {
    if ( m_pCurrentChunk == m_chunks.end() )
    {
      return {};
    }
    const std::span ret{ m_pCurrentChunk->lines };
    return ret.subspan( m_uLineOffsetInChunk, std::min( ret.size() - m_uLineOffsetInChunk, uNumLines ) );
  }
};

}// namespace jess
