#pragma once

#include "CSeekableStream.hpp"
#include "SdCursor.hpp"
#include "SdLine.hpp"
#include <systemd/sd-journal.h>
#include <unordered_map>
#include <map>
#include <memory>

namespace jess
{

struct JournalDeleter {
  void operator()( sd_journal* ptr ) { sd_journal_close( ptr ); }
};

class SdJournal
{
  std::unique_ptr<sd_journal, JournalDeleter> handle{};

public:
  explicit SdJournal()
  {
    sd_journal* pJournal{};
    sd_journal_open( &pJournal, 0 );
    handle.reset( pJournal );
  };

  [[nodiscard]] SdCursor getCursor()
  {
    char* cursor;
    sd_journal_get_cursor( handle.get(), &cursor );
    auto ret = SdCursor::fromString( cursor );
    free( cursor );
    return ret;
  }

  void seekToCursor( const std::string& sCursor ) { sd_journal_seek_cursor( handle.get(), sCursor.c_str() ); }

  void seekToBof() { sd_journal_seek_head( handle.get() ); }

  void seekToEof() { sd_journal_seek_tail( handle.get() ); }

  void seekLinesBackward( size_t uNumLines ) { sd_journal_previous_skip( handle.get(), uNumLines ); }

  void seekLinesForward( size_t uNumLines ) { sd_journal_next_skip( handle.get(), uNumLines ); }

  bool next() { return sd_journal_next( handle.get() ) > 0; }

  bool previous() { return sd_journal_previous( handle.get() ) > 0; }

  std::string_view getFieldString( std::string_view sFieldName )
  {
    const void* ptr = nullptr;
    size_t uMessageLength{};
    sd_journal_get_data( handle.get(), sFieldName.data(), reinterpret_cast<const void**>( &ptr ), &uMessageLength );
    std::string_view ret{ static_cast<const char*>( ptr ), uMessageLength };

    if ( ret.size() >= sFieldName.size() + 1 )
    {
      return ret.substr( sFieldName.size() + 1 );
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "LocalValueEscapesScope"
    return ret;
#pragma clang diagnostic pop
  }

  std::chrono::time_point<std::chrono::system_clock> getTimestampRealtime()
  {
    uint64_t ret;
    sd_journal_get_realtime_usec( handle.get(), &ret );
    return std::chrono::time_point<std::chrono::system_clock>{ std::chrono::microseconds{ ret } };
  }

  [[nodiscard]] SdSeqid getSeqid() const
  {
    SdSeqid seqid{};
    sd_journal_get_seqnum( handle.get(), &seqid.seqnum.value, reinterpret_cast<sd_id128_t*>( &seqid.seqnumId.value ) );
    return seqid;
  }

  SdLine getLine() { return SdLine{ getSeqid(), std::string{ getFieldString( "MESSAGE" ) }, getTimestampRealtime() }; }
};

static_assert( SeekableStream<SdJournal> );

}// namespace jess
