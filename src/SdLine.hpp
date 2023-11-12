#pragma once

#include "SdSeqid.hpp"

#include <chrono>
#include <string>
#include <utility>
#include <iosfwd>


namespace jess
{
class SdLine
{
public:
  explicit SdLine( SdSeqid seqid, std::string sMessage, std::chrono::time_point<std::chrono::system_clock> timestampRealtime )
    : m_seqid( seqid )
    , sMessage( std::move( sMessage ) )
    , timestampRealtime( timestampRealtime )
  {
  }

  [[nodiscard]] SdSeqid seqid() const { return m_seqid; }
  [[nodiscard]] std::string message() const { return sMessage; }
  [[nodiscard]] std::chrono::time_point<std::chrono::system_clock> realtime() const { return timestampRealtime; }
  [[nodiscard]] std::string realtimeUtc() const
  {
    auto time_t_ = std::chrono::system_clock::to_time_t( timestampRealtime );
    std::tm tm_ = *std::gmtime( &time_t_ );
    std::stringstream ss{};
    ss << std::put_time( &tm_, "%Y-%m-%d %H:%M:%S" );
    return ss.str();
  }

private:
  SdSeqid m_seqid;
  std::string sMessage;
  std::chrono::time_point<std::chrono::system_clock> timestampRealtime;
};
}// namespace jess
