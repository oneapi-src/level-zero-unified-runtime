//===--------- device.hpp - HIP Adapter -----------------------------------===//
//
// Copyright (C) 2023 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#pragma once

#include "common.hpp"

#include <ur_api.h>

/// UR device mapping to a hipDevice_t.
/// Includes an observer pointer to the platform,
/// and implements the reference counting semantics since
/// HIP objects are not refcounted.
struct ur_device_handle_t_ {
private:
  using native_type = hipDevice_t;

  native_type HIPDevice;
  std::atomic_uint32_t RefCount;
  ur_platform_handle_t Platform;
  hipCtx_t HIPContext;
  uint32_t DeviceIndex;

public:
  ur_device_handle_t_(native_type HipDevice, hipCtx_t Context,
                      ur_platform_handle_t Platform, uint32_t DeviceIndex)
      : HIPDevice(HipDevice), RefCount{1}, Platform(Platform),
        HIPContext(Context), DeviceIndex(DeviceIndex) {}

  ~ur_device_handle_t_() noexcept(false) {
    UR_CHECK_ERROR(hipDevicePrimaryCtxRelease(HIPDevice));
  }

  native_type get() const noexcept { return HIPDevice; };

  uint32_t getReferenceCount() const noexcept { return RefCount; }

  ur_platform_handle_t getPlatform() const noexcept { return Platform; };

  hipCtx_t getNativeContext() const noexcept { return HIPContext; };

  // Returns the index of the device relative to the other devices in the same
  // platform
  uint32_t getIndex() const noexcept { return DeviceIndex; };
};

int getAttribute(ur_device_handle_t Device, hipDeviceAttribute_t Attribute);
