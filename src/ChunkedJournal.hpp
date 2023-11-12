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

enum class Adjacency {
  NON_ADJACENT,
  AFTER_CURRENT,
  BEFORE_CURRENT,
};

enum class Contiguity {
  CONTIGUOUS,
  NON_CONTIGUOUS,
  OVERLAPPING,
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
  decltype( m_pCurrentChunk ) findChunkInsertionPosition( const Chunk& chunk )
  {
    const auto firstId = chunk.lines.front().seqid();
    auto lastValidId = m_chunks.begin();
    for ( auto it = m_chunks.begin(); it != m_chunks.end(); ++it )
    {
      if ( auto seq = it->highestIdsInChunk.find( firstId.seqnumId ); seq != it->highestIdsInChunk.end() )
      {
        if ( firstId.seqnum > seq->second )
        {
          lastValidId = it;
        }
        else
        {
          return lastValidId;
        }
      }
    }
    return m_chunks.end();
  }

  auto createChunkAtCurrentPosition( const Adjacency adjacency ) -> decltype( m_chunks.begin() )
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

    decltype( m_pCurrentChunk ) insertIt;

    switch ( adjacency )
    {
      case Adjacency::NON_ADJACENT: {
        insertIt = findChunkInsertionPosition( newChunk );
        break;
      }
      case Adjacency::BEFORE_CURRENT: {
        insertIt = m_pCurrentChunk;
        break;
      }
      case Adjacency::AFTER_CURRENT: {
        insertIt = m_pCurrentChunk;
        std::advance( insertIt, 1 );
        break;
      }
      default: {
        throw std::logic_error{ "unexpected adjacency state" };
      }
    }

    auto newChunkIt = m_chunks.insert( insertIt, std::move( newChunk ) );

    if ( adjacency == Adjacency::BEFORE_CURRENT )
    {
      assert( m_pCurrentChunk != m_chunks.end() );
      newChunkIt->contiguityEnd = Contiguity::CONTIGUOUS;
      m_pCurrentChunk->contiguityBeginning = Contiguity::CONTIGUOUS;
    }
    else if ( adjacency == Adjacency::AFTER_CURRENT )
    {
      assert( m_pCurrentChunk != m_chunks.end() );
      m_pCurrentChunk->contiguityEnd = Contiguity::CONTIGUOUS;
      newChunkIt->contiguityBeginning = Contiguity::CONTIGUOUS;
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

  void loadChunkAtCurrentPosition( const Adjacency adjacency )
  {
    const auto seqid = m_journal.getSeqid();
    if ( auto chunks = getChunksBySeqid( seqid ); !chunks.empty() )
    {
      m_pCurrentChunk = chunks.back();
    }
    else
    {
      m_pCurrentChunk = createChunkAtCurrentPosition( adjacency );
    }
  }

public:
  void seekToBof()
  {
    m_journal.seekToBof();
    m_journal.next();
    loadChunkAtCurrentPosition( Adjacency::NON_ADJACENT );
    m_uLineOffsetInChunk = 0;
  }

  void seekToEof()
  {
    m_journal.seekToEof();
    m_journal.seekLinesBackward( m_uChunkSize );
    loadChunkAtCurrentPosition( Adjacency::NON_ADJACENT );
    // bug: this will explode if there are fewer than m_uChunkSize lines in the chunk
    m_uLineOffsetInChunk = m_uChunkSize - 1;
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

      if ( const size_t uLinesToSeek = uNumChunksToSkip * m_uChunkSize; uLinesToSeek > 0 )
      {
        m_journal.seekLinesForward( uLinesToSeek );
      }

      const Adjacency adjacency = uNumChunksToSkip == 0 ? Adjacency::AFTER_CURRENT : Adjacency::NON_ADJACENT;
      loadChunkAtCurrentPosition( adjacency );
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

  std::string getChunkPositionString()
  {
    const size_t uNthChunk = std::distance( m_chunks.begin(), m_pCurrentChunk );
    const size_t uNumChunks = m_chunks.size();

    return "chunk " + std::to_string( uNthChunk + 1 ) + "/" + std::to_string( uNumChunks ) + "; line " + std::to_string( m_uLineOffsetInChunk + 1 ) + "/" +
      std::to_string( m_pCurrentChunk->lines.size() );
  }
};

}// namespace jess
