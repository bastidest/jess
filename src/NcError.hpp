#pragma once

#include <stdexcept>

#define NC_CHECK_RC(STATEMENT)                                                                                         \
  do {                                                                                                                 \
    int rc = (STATEMENT);                                                                                              \
    if (rc != 0) {                                                                                                     \
      throw jess::NcError{#STATEMENT " returned " + std::to_string(rc)};                                               \
    }                                                                                                                  \
  } while (0)

#define NC_CHECK_PTR(STATEMENT)                                                                                        \
  do {                                                                                                                 \
    if ((STATEMENT) == nullptr) {                                                                                      \
      throw jess::NcError{#STATEMENT " returned nullptr"};                                                             \
    }                                                                                                                  \
  } while (0)

namespace jess {

struct NcError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

} // namespace jess
