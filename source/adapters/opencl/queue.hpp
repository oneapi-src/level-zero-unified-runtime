//===--------- queue.hpp - OpenCL Adapter ---------------------------===//
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
#include "context.hpp"
#include "device.hpp"

#include <vector>

struct ur_queue_handle_t_ : cl_adapter::ur_handle_t_ {
  using native_type = cl_command_queue;
  native_type CLQueue;
  ur_context_handle_t Context;
  ur_device_handle_t Device;
  // Used to keep a handle to the default queue alive if it is different
  std::optional<ur_queue_handle_t> DeviceDefault = std::nullopt;
  std::atomic<uint32_t> RefCount = 0;
  bool IsNativeHandleOwned = true;

  ur_queue_handle_t_(native_type Queue, ur_context_handle_t Ctx,
                     ur_device_handle_t Dev)
      : cl_adapter::ur_handle_t_(), CLQueue(Queue), Context(Ctx), Device(Dev) {
    RefCount = 1;
    if (Device) {
      urDeviceRetain(Device);
    }
    urContextRetain(Context);
  }

  static ur_result_t makeWithNative(native_type NativeQueue,
                                    ur_context_handle_t Context,
                                    ur_device_handle_t Device,
                                    ur_queue_handle_t &Queue);

  ~ur_queue_handle_t_() {
    if (Device) {
      urDeviceRelease(Device);
    }
    urContextRelease(Context);
    if (IsNativeHandleOwned) {
      clReleaseCommandQueue(CLQueue);
    }
    if (DeviceDefault.has_value()) {
      urQueueRelease(*DeviceDefault);
    }
  }

  uint32_t incrementReferenceCount() noexcept { return ++RefCount; }

  uint32_t decrementReferenceCount() noexcept { return --RefCount; }

  uint32_t getReferenceCount() const noexcept { return RefCount; }
};
