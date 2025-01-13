//===--------- common.hpp - Level Zero Adapter ---------------------------===//
//
// Copyright (C) 2024 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#pragma once

#include <exception>
#include <ze_api.h>

#include "../common.hpp"
#include "../ur_interface_loader.hpp"
#include "logger/ur_logger.hpp"

namespace v2 {

namespace raii {

template <typename ZeHandleT, ze_result_t (*destroy)(ZeHandleT)>
struct ze_handle_wrapper {
  ze_handle_wrapper(bool ownZeHandle = true)
      : handle(nullptr), ownZeHandle(ownZeHandle) {}

  ze_handle_wrapper(ZeHandleT handle, bool ownZeHandle = true)
      : handle(handle), ownZeHandle(ownZeHandle) {}

  ze_handle_wrapper(const ze_handle_wrapper &) = delete;
  ze_handle_wrapper &operator=(const ze_handle_wrapper &) = delete;

  ze_handle_wrapper(ze_handle_wrapper &&other)
      : handle(other.handle), ownZeHandle(other.ownZeHandle) {
    other.handle = nullptr;
  }

  ze_handle_wrapper &operator=(ze_handle_wrapper &&other) {
    if (this == &other) {
      return *this;
    }

    if (handle) {
      reset();
    }
    handle = other.handle;
    ownZeHandle = other.ownZeHandle;
    other.handle = nullptr;
    return *this;
  }

  ~ze_handle_wrapper() {
    try {
      reset();
    } catch (...) {
      // logging already done in reset
    }
  }

  void reset() {
    if (!handle) {
      return;
    }

    if (ownZeHandle) {
      auto zeResult = ZE_CALL_NOCHECK(destroy, (handle));
      // Gracefully handle the case that L0 was already unloaded.
      if (zeResult && zeResult != ZE_RESULT_ERROR_UNINITIALIZED)
        throw ze2urResult(zeResult);
    }

    handle = nullptr;
  }

  std::pair<ZeHandleT, bool> release() {
    auto handle = this->handle;
    this->handle = nullptr;
    return {handle, ownZeHandle};
  }

  ZeHandleT get() const { return handle; }

  ZeHandleT *ptr() { return &handle; }

private:
  ZeHandleT handle;
  bool ownZeHandle;
};

using ze_kernel_handle_t =
    ze_handle_wrapper<::ze_kernel_handle_t, zeKernelDestroy>;

using ze_event_handle_t =
    ze_handle_wrapper<::ze_event_handle_t, zeEventDestroy>;

using ze_event_pool_handle_t =
    ze_handle_wrapper<::ze_event_pool_handle_t, zeEventPoolDestroy>;

using ze_context_handle_t =
    ze_handle_wrapper<::ze_context_handle_t, zeContextDestroy>;

using ze_command_list_handle_t =
    ze_handle_wrapper<::ze_command_list_handle_t, zeCommandListDestroy>;

template <typename URHandle, ur_result_t (*retain)(URHandle),
          ur_result_t (*release)(URHandle)>
struct ref_counted {
  ref_counted(URHandle handle) : handle(handle) {
    if (handle) {
      retain(handle);
    }
  }

  ~ref_counted() {
    if (handle) {
      release(handle);
    }
  }

  operator URHandle() const { return handle; }
  URHandle operator->() const { return handle; }

  ref_counted(const ref_counted &) = delete;
  ref_counted &operator=(const ref_counted &) = delete;

  ref_counted(ref_counted &&other) {
    handle = other.handle;
    other.handle = nullptr;
  }

  ref_counted &operator=(ref_counted &&other) {
    if (this == &other) {
      return *this;
    }

    if (handle) {
      release(handle);
    }

    handle = other.handle;
    other.handle = nullptr;
    return *this;
  }

  URHandle get() const { return handle; }

private:
  URHandle handle;
};

template <typename URHandle>
ur_result_t validateRetain([[maybe_unused]] URHandle handle) {
  assert(reinterpret_cast<_ur_object *>(handle)->RefCount.load() != 0);
  return UR_RESULT_SUCCESS;
}

template <typename URHandle>
ur_result_t validateRelease([[maybe_unused]] URHandle handle) {
  assert(reinterpret_cast<_ur_object *>(handle)->RefCount.load() != 0);
  return UR_RESULT_SUCCESS;
}

// Devices are owned by the platform, so we don't need to retain/release them
// as long as the platform is alive.
using ur_device_handle_t =
    ref_counted<::ur_device_handle_t, validateRetain<::ur_device_handle_t>,
                validateRelease<::ur_device_handle_t>>;

// Spec requires that the context is not destroyed until all entities
// using the context are destroyed.
using ur_context_handle_t =
    ref_counted<::ur_context_handle_t, validateRetain<::ur_context_handle_t>,
                validateRelease<::ur_context_handle_t>>;

using ur_mem_handle_t =
    ref_counted<::ur_mem_handle_t, ur::level_zero::urMemRetain, urMemRelease>;

using ur_program_handle_t =
    ref_counted<::ur_program_handle_t, ur::level_zero::urProgramRetain,
                urProgramRelease>;

} // namespace raii

} // namespace v2
