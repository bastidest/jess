#pragma once

#include "SdLine.hpp"
#include <concepts>
#include <cstddef>

namespace jess
{

template<typename T>
concept SeekableStream = requires( T a ) {
  std::is_default_constructible_v<T>;
  {
    a.seekToBof()
  } -> std::same_as<void>;
  {
    a.seekToEof()
  } -> std::same_as<void>;
  {
    a.seekLinesForward( std::declval<size_t>() )
  } -> std::same_as<void>;
  {
    a.seekLinesBackward( std::declval<size_t>() )
  } -> std::same_as<void>;
  {
    a.next()
  } -> std::same_as<bool>;
  {
    a.getLine()
  } -> std::same_as<SdLine>;
  {
    a.getSeqid()
  } -> std::same_as<SdSeqid>;
};

}// namespace jess