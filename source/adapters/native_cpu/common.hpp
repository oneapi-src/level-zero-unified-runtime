//===----------- common.hpp - Native CPU Adapter ---------------------===//
//
// Copyright (C) 2023 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#pragma once

#include <atomic>

#include "ur_api.h"
#include "ur_util.hpp"

constexpr size_t MaxMessageSize = 256;

extern thread_local ur_result_t ErrorMessageCode;
extern thread_local char ErrorMessage[MaxMessageSize];

#define DIE_NO_IMPLEMENTATION                                                  \
  if (PrintTrace) {                                                            \
    std::cerr << "Not Implemented : " << __FUNCTION__                          \
              << " - File : " << __FILE__;                                     \
    std::cerr << " / Line : " << __LINE__ << std::endl;                        \
  }                                                                            \
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;

#define CONTINUE_NO_IMPLEMENTATION                                             \
  if (PrintTrace) {                                                            \
    std::cerr << "Warning : Not Implemented : " << __FUNCTION__                \
              << " - File : " << __FILE__;                                     \
    std::cerr << " / Line : " << __LINE__ << std::endl;                        \
  }                                                                            \
  return UR_RESULT_SUCCESS;

#define CASE_UR_UNSUPPORTED(not_supported)                                     \
  case not_supported:                                                          \
    if (PrintTrace) {                                                          \
      std::cerr << std::endl                                                   \
                << "Unsupported UR case : " << #not_supported << " in "        \
                << __FUNCTION__ << ":" << __LINE__ << "(" << __FILE__ << ")"   \
                << std::endl;                                                  \
    }                                                                          \
    return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;

/// ------ Error handling, matching OpenCL plugin semantics.
/// Taken from other adapter
namespace detail {
namespace ur {

// Report error and no return (keeps compiler from printing warnings).
// TODO: Probably change that to throw a catchable exception,
//       but for now it is useful to see every failure.
//
[[noreturn]] void die(const char *pMessage);
} // namespace ur
} // namespace detail

// Base class to store common data
struct _ur_object {
  ur::SharedMutex Mutex;
};

// Todo: replace this with a common helper once it is available
struct RefCounted {
  std::atomic_uint32_t _refCount;
  uint32_t incrementReferenceCount() { return ++_refCount; }
  uint32_t decrementReferenceCount() { return --_refCount; }
  RefCounted() : _refCount{1} {}
  uint32_t getReferenceCount() const { return _refCount; }
};

template <typename T> inline void decrementOrDelete(T *refC) {
  if (refC->decrementReferenceCount() == 0)
    delete refC;
}
