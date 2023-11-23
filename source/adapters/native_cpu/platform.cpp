//===--------- platform.cpp - Native CPU Adapter --------------------------===//
//
// Copyright (C) 2023 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "platform.hpp"
#include "common.hpp"

#include "ur/ur.hpp"
#include "ur_api.h"

#include <iostream>

UR_APIEXPORT ur_result_t UR_APICALL
urPlatformGet(ur_adapter_handle_t *, uint32_t, uint32_t NumEntries,
              ur_platform_handle_t *phPlatforms, uint32_t *pNumPlatforms) {

  UR_ASSERT(pNumPlatforms || phPlatforms, UR_RESULT_ERROR_INVALID_VALUE);

  if (pNumPlatforms) {
    *pNumPlatforms = 1;
  }

  if (NumEntries == 0) {
    if (phPlatforms != nullptr) {
      if (PrintTrace) {
        std::cerr << "Invalid argument combination for urPlatformsGet\n";
      }
      return UR_RESULT_ERROR_INVALID_VALUE;
    }
    return UR_RESULT_SUCCESS;
  }
  if (phPlatforms && NumEntries > 0) {
    static ur_platform_handle_t_ ThePlatform;
    *phPlatforms = &ThePlatform;
  }
  return UR_RESULT_SUCCESS;
}

UR_APIEXPORT ur_result_t UR_APICALL urPlatformGetApiVersion(
    ur_platform_handle_t hDriver, ur_api_version_t *pVersion) {
  UR_ASSERT(hDriver, UR_RESULT_ERROR_INVALID_NULL_HANDLE);
  UR_ASSERT(pVersion, UR_RESULT_ERROR_INVALID_NULL_POINTER);

  *pVersion = UR_API_VERSION_CURRENT;
  return UR_RESULT_SUCCESS;
}

UR_APIEXPORT ur_result_t UR_APICALL
urPlatformGetInfo(ur_platform_handle_t hPlatform, ur_platform_info_t propName,
                  size_t propSize, void *pParamValue, size_t *pSizeRet) {

  if (hPlatform == nullptr) {
    return UR_RESULT_ERROR_INVALID_PLATFORM;
  }
  UrReturnHelper ReturnValue(propSize, pParamValue, pSizeRet);

  switch (propName) {
  case UR_PLATFORM_INFO_NAME:
    return ReturnValue("SYCL_NATIVE_CPU");

  case UR_PLATFORM_INFO_VENDOR_NAME:
    return ReturnValue("tbd");

  case UR_PLATFORM_INFO_VERSION:
    return ReturnValue("0.1");

  case UR_PLATFORM_INFO_PROFILE:
    return ReturnValue("FULL_PROFILE");

  case UR_PLATFORM_INFO_EXTENSIONS:
    return ReturnValue("");

  case UR_PLATFORM_INFO_BACKEND:
    // TODO(alcpz): PR with this enum value at
    // https://github.com/oneapi-src/unified-runtime
    return ReturnValue(UR_PLATFORM_BACKEND_NATIVE_CPU);
  default:
    return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
  }

  return UR_RESULT_SUCCESS;
}

UR_APIEXPORT ur_result_t UR_APICALL urPlatformGetBackendOption(
    ur_platform_handle_t hPlatform, const char *pFrontendOption,
    const char **ppPlatformOption) {
  std::ignore = hPlatform;
  std::ignore = pFrontendOption;
  std::ignore = ppPlatformOption;

  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

UR_APIEXPORT ur_result_t UR_APICALL urPlatformCreateWithNativeHandle(
    ur_native_handle_t hNativePlatform,
    const ur_platform_native_properties_t *pProperties,
    ur_platform_handle_t *phPlatform) {
  std::ignore = hNativePlatform;
  std::ignore = pProperties;
  std::ignore = phPlatform;

  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

UR_APIEXPORT ur_result_t UR_APICALL urPlatformGetNativeHandle(
    ur_platform_handle_t hPlatform, ur_native_handle_t *phNativePlatform) {
  std::ignore = hPlatform;
  std::ignore = phNativePlatform;

  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}
