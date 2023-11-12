#include "ChunkedJournal.hpp"
#include <doctest/doctest.h>

using namespace std::string_view_literals;

template<size_t uStreamLength>
struct MockStream {
  size_t pos{};
  std::optional<jess::SdLine> currentLine{};

  void seekToBof() { pos = 0; }

  void seekToEof() { pos = uStreamLength - 1; }

  void seekLinesForward( const size_t uNumLines ) { pos = std::min( pos + uNumLines, uStreamLength - 1 ); }

  void seekLinesBackward( const size_t uNumLines )
  {
    if ( uNumLines > pos )
    {
      pos = 0;
    }
    else
    {
      pos = pos - uNumLines;
    }
  }

  /// read the data the the current position and advance the position
  bool next()
  {
    currentLine = jess::SdLine{
      { { std::array<uint8_t, 16>{} }, { pos } },
      std::string{ "line " } + std::to_string( pos ),
      std::chrono::system_clock::time_point{ std::chrono::seconds{ pos } },
    };
    pos += 1;
    return pos <= uStreamLength;
  }

  [[nodiscard]] jess::SdSeqid getSeqid() const { return currentLine->seqid(); }

  jess::SdLine getLine() { return currentLine.value(); }
};

void checkSequence( const jess::Chunk& chunk, const size_t uLength, const size_t uFirstIndex )
{
  REQUIRE( chunk.lines.size() == uLength );
  for ( size_t i = 0; i < uLength; ++i )
  {
    CHECK( chunk.lines.at( i ).seqid().seqnum.value == i + uFirstIndex );
    CHECK( chunk.lines.at( i ).message() == "line " + std::to_string( i + uFirstIndex ) );
  }
}

TEST_CASE( "ChunkedJournal(1) constructor" )
{
  jess::ChunkedJournal<MockStream<10>> sut{ 1, 10 };
  const std::list<jess::Chunk>& chunks = sut.getChunks();
  REQUIRE( chunks.size() == 0 );
}

TEST_CASE( "ChunkedJournal(1) load BOF" )
{
  jess::ChunkedJournal<MockStream<10>> sut{ 1, 0 };
  sut.seekToBof();
  const std::list<jess::Chunk>& chunks = sut.getChunks();
  REQUIRE( chunks.size() == 1 );

  const jess::Chunk& firstChunk = chunks.front();
  CHECK( firstChunk.contiguityBeginning == jess::Contiguity::NON_CONTIGUOUS );
  CHECK( firstChunk.contiguityEnd == jess::Contiguity::NON_CONTIGUOUS );
  checkSequence( firstChunk, 1, 0 );

  SUBCASE( "seeking to BOF shall again shall not change anything" )
  {
    sut.seekToBof();
    const std::list<jess::Chunk>& chunks2 = sut.getChunks();
    CHECK( chunks2.size() == 1 );
  }

  SUBCASE( "advance one line" )
  {
    sut.seekLines( 1 );
    const std::list<jess::Chunk>& chunks2 = sut.getChunks();
    REQUIRE( chunks2.size() == 2 );
    REQUIRE_MESSAGE( &chunks.front() == &firstChunk, "the first chunk shall remain at the first position" );

    checkSequence( firstChunk, 1, 0 );
    CHECK( firstChunk.contiguityBeginning == jess::Contiguity::NON_CONTIGUOUS );
    CHECK( firstChunk.contiguityEnd == jess::Contiguity::CONTIGUOUS );

    const jess::Chunk& secondChunk = chunks2.back();
    checkSequence( secondChunk, 1, 1 );
    CHECK( secondChunk.contiguityBeginning == jess::Contiguity::CONTIGUOUS );
    CHECK( secondChunk.contiguityEnd == jess::Contiguity::NON_CONTIGUOUS );
  }
}

TEST_CASE( "ChunkedJournal(2) load BOF" )
{
  jess::ChunkedJournal<MockStream<10>> sut{ 2, 0 };
  sut.seekToBof();
  const std::list<jess::Chunk>& chunks = sut.getChunks();
  REQUIRE( chunks.size() == 1 );

  const jess::Chunk& firstChunk = chunks.front();
  checkSequence( firstChunk, 2, 0 );
  CHECK( firstChunk.contiguityBeginning == jess::Contiguity::NON_CONTIGUOUS );
  CHECK( firstChunk.contiguityEnd == jess::Contiguity::NON_CONTIGUOUS );

  SUBCASE( "seeking to BOF shall again shall not change anything" )
  {
    sut.seekToBof();
    const std::list<jess::Chunk>& chunks2 = sut.getChunks();
    CHECK( chunks2.size() == 1 );
  }

  SUBCASE( "advance one line" )
  {
    {
      sut.seekLines( 1 );
      const std::list<jess::Chunk>& chunks2 = sut.getChunks();
      REQUIRE( chunks2.size() == 1 );
      REQUIRE_MESSAGE( &chunks.front() == &firstChunk, "the first chunk shall remain at the first position" );
      checkSequence( chunks.front(), 2, 0 );
      CHECK( firstChunk.contiguityBeginning == jess::Contiguity::NON_CONTIGUOUS );
      CHECK( firstChunk.contiguityEnd == jess::Contiguity::NON_CONTIGUOUS );
    }

    SUBCASE( "advance another line" )
    {
      sut.seekLines( 1 );
      const std::list<jess::Chunk>& chunks2 = sut.getChunks();
      REQUIRE( chunks2.size() == 2 );
      REQUIRE_MESSAGE( &chunks.front() == &firstChunk, "the first chunk shall remain at the first position" );

      checkSequence( chunks.front(), 2, 0 );
      checkSequence( chunks.back(), 2, 2 );

      CHECK( chunks.front().contiguityBeginning == jess::Contiguity::NON_CONTIGUOUS );
      CHECK( chunks.front().contiguityEnd == jess::Contiguity::CONTIGUOUS );
      CHECK( chunks.back().contiguityBeginning == jess::Contiguity::CONTIGUOUS );
      CHECK( chunks.back().contiguityEnd == jess::Contiguity::NON_CONTIGUOUS );
    }
  }

  SUBCASE( "advance two lines" )
  {
    sut.seekLines( 2 );
    const std::list<jess::Chunk>& chunks2 = sut.getChunks();
    REQUIRE( chunks2.size() == 2 );
    REQUIRE_MESSAGE( &chunks.front() == &firstChunk, "the first chunk shall remain at the first position" );

    checkSequence( chunks.front(), 2, 0 );
    checkSequence( chunks.back(), 2, 2 );

    CHECK( chunks.front().contiguityBeginning == jess::Contiguity::NON_CONTIGUOUS );
    CHECK( chunks.front().contiguityEnd == jess::Contiguity::CONTIGUOUS );
    CHECK( chunks.back().contiguityBeginning == jess::Contiguity::CONTIGUOUS );
    CHECK( chunks.back().contiguityEnd == jess::Contiguity::NON_CONTIGUOUS );
  }
}

TEST_CASE( "ChunkedJournal(3) load BOF" )
{
  jess::ChunkedJournal<MockStream<10>> sut{ 3, 0 };
  sut.seekToBof();
  const std::list<jess::Chunk>& chunks = sut.getChunks();
  REQUIRE( chunks.size() == 1 );

  checkSequence( chunks.front(), 3, 0 );
  CHECK( chunks.front().contiguityBeginning == jess::Contiguity::NON_CONTIGUOUS );
  CHECK( chunks.front().contiguityEnd == jess::Contiguity::NON_CONTIGUOUS );
}

TEST_CASE( "ChunkedJournal(1) load EOF" )
{
  jess::ChunkedJournal<MockStream<10>> sut{ 1, 0 };
  sut.seekToEof();
  const std::list<jess::Chunk>& chunks = sut.getChunks();
  REQUIRE( chunks.size() == 1 );
  const jess::Chunk& lastChunk = chunks.front();
  checkSequence( lastChunk, 1, 9 );
  CHECK( lastChunk.contiguityBeginning == jess::Contiguity::NON_CONTIGUOUS );
  CHECK( lastChunk.contiguityEnd == jess::Contiguity::NON_CONTIGUOUS );

  SUBCASE( "seek to BOF" )
  {
    sut.seekToBof();
    REQUIRE( chunks.size() == 2 );
    const jess::Chunk& firstChunk = chunks.front();
    checkSequence( firstChunk, 1, 0 );
    CHECK( firstChunk.contiguityBeginning == jess::Contiguity::NON_CONTIGUOUS );
    CHECK( firstChunk.contiguityEnd == jess::Contiguity::NON_CONTIGUOUS );
    CHECK( lastChunk.contiguityBeginning == jess::Contiguity::NON_CONTIGUOUS );
    CHECK( lastChunk.contiguityEnd == jess::Contiguity::NON_CONTIGUOUS );

    SUBCASE( "load next line" )
    {
      sut.seekLines( 1 );
      REQUIRE( chunks.size() == 3 );
      auto it = chunks.begin();
      std::advance( it, 1 );
      const jess::Chunk& secondChunk = *it;
      REQUIRE( &secondChunk != &firstChunk );
      REQUIRE( &secondChunk != &lastChunk );
      checkSequence( secondChunk, 1, 1 );
      CHECK( firstChunk.contiguityBeginning == jess::Contiguity::NON_CONTIGUOUS );
      CHECK( firstChunk.contiguityEnd == jess::Contiguity::CONTIGUOUS );
      CHECK( secondChunk.contiguityBeginning == jess::Contiguity::CONTIGUOUS );
      CHECK( secondChunk.contiguityEnd == jess::Contiguity::NON_CONTIGUOUS );
      CHECK( lastChunk.contiguityBeginning == jess::Contiguity::NON_CONTIGUOUS );
      CHECK( lastChunk.contiguityEnd == jess::Contiguity::NON_CONTIGUOUS );
    }
  }
}

// todo: check skipping a chunk
