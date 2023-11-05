#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "SdCursor.hpp"
#include <doctest/doctest.h>

using namespace std::string_view_literals;

TEST_CASE( "SdCursor constructor" ) {
  jess::SdCursor::fromString("s=72f9ece3caa84aaab5fd4eac74f04a32;i=2d39c1ee;b=2f9fe978f1b14fff91f8cc319b400956;m=21733b7;t=6075f19daf269;x=87e478517491f1d0");
}

TEST_CASE( "SdCursor parse and serialize" ) {
  auto input = "s=72f9ece3caa84aaab5fd4eac74f04a32;i=2d39c1ee;b=2f9fe978f1b14fff91f8cc319b400956;m=21733b7;t=6075f19daf269;x=87e478517491f1d0"sv;
  auto sut = jess::SdCursor::fromString(input);
  REQUIRE(sut.toString() == input);
}

TEST_CASE( "hexCharToNibble uppercase" ) {
  REQUIRE(jess::hexCharToNibble<true>('0') == 0x0);
  REQUIRE(jess::hexCharToNibble<true>('1') == 0x1);
  REQUIRE(jess::hexCharToNibble<true>('2') == 0x2);
  REQUIRE(jess::hexCharToNibble<true>('3') == 0x3);
  REQUIRE(jess::hexCharToNibble<true>('4') == 0x4);
  REQUIRE(jess::hexCharToNibble<true>('5') == 0x5);
  REQUIRE(jess::hexCharToNibble<true>('6') == 0x6);
  REQUIRE(jess::hexCharToNibble<true>('7') == 0x7);
  REQUIRE(jess::hexCharToNibble<true>('8') == 0x8);
  REQUIRE(jess::hexCharToNibble<true>('9') == 0x9);
  REQUIRE(jess::hexCharToNibble<true>('A') == 0xa);
  REQUIRE(jess::hexCharToNibble<true>('B') == 0xb);
  REQUIRE(jess::hexCharToNibble<true>('C') == 0xc);
  REQUIRE(jess::hexCharToNibble<true>('D') == 0xd);
  REQUIRE(jess::hexCharToNibble<true>('E') == 0xe);
  REQUIRE(jess::hexCharToNibble<true>('F') == 0xf);
}

TEST_CASE( "hexCharToNibble lowercase" ) {
  REQUIRE(jess::hexCharToNibble<false>('0') == 0x0);
  REQUIRE(jess::hexCharToNibble<false>('1') == 0x1);
  REQUIRE(jess::hexCharToNibble<false>('2') == 0x2);
  REQUIRE(jess::hexCharToNibble<false>('3') == 0x3);
  REQUIRE(jess::hexCharToNibble<false>('4') == 0x4);
  REQUIRE(jess::hexCharToNibble<false>('5') == 0x5);
  REQUIRE(jess::hexCharToNibble<false>('6') == 0x6);
  REQUIRE(jess::hexCharToNibble<false>('7') == 0x7);
  REQUIRE(jess::hexCharToNibble<false>('8') == 0x8);
  REQUIRE(jess::hexCharToNibble<false>('9') == 0x9);
  REQUIRE(jess::hexCharToNibble<false>('a') == 0xa);
  REQUIRE(jess::hexCharToNibble<false>('b') == 0xb);
  REQUIRE(jess::hexCharToNibble<false>('c') == 0xc);
  REQUIRE(jess::hexCharToNibble<false>('d') == 0xd);
  REQUIRE(jess::hexCharToNibble<false>('e') == 0xe);
  REQUIRE(jess::hexCharToNibble<false>('f') == 0xf);
}
