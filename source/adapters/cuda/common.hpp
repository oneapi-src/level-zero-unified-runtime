//===--------- common.hpp - CUDA Adapter ----------------------------------===//
//
// Copyright (C) 2023 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#pragma once

#include <cuda.h>
#include <ur/ur.hpp>

/**
 * Call a UR API and, if the result is not UR_RESULT_SUCCESS, automatically
 * return from the current function.
 */
#define UR_RETURN_ON_FAILURE(urCall)                                           \
  if (const ur_result_t ur_result_macro = urCall;                              \
      ur_result_macro != UR_RESULT_SUCCESS) {                                  \
    return ur_result_macro;                                                    \
  }

ur_result_t mapErrorUR(CUresult Result);

/// Converts CUDA error into UR error codes, and outputs error information
/// to stderr.
/// If PI_CUDA_ABORT env variable is defined, it aborts directly instead of
/// throwing the error. This is intended for debugging purposes.
/// \return UR_RESULT_SUCCESS if \param Result was CUDA_SUCCESS.
/// \throw ur_result_t exception (integer) if input was not success.
///
void checkErrorUR(CUresult Result, const char *Function, int Line,
                  const char *File);

void checkErrorUR(ur_result_t Result, const char *Function, int Line,
                  const char *File);

#define UR_CHECK_ERROR(Result)                                                 \
  checkErrorUR(Result, __func__, __LINE__, __FILE__)

std::string getCudaVersionString();

constexpr size_t MaxMessageSize = 256;
extern thread_local ur_result_t ErrorMessageCode;
extern thread_local char ErrorMessage[MaxMessageSize];

// Utility function for setting a message and warning
[[maybe_unused]] void setErrorMessage(const char *pMessage,
                                      ur_result_t ErrorCode);

void setPluginSpecificMessage(CUresult cu_res);

/// ------ Error handling, matching OpenCL plugin semantics.
namespace detail {
namespace ur {

// Reports error messages
void cuPrint(const char *Message);

} // namespace ur
} // namespace detail
