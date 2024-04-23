/*
 *
 * Copyright (C) 2024 Intel Corporation
 *
 * Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
 * See LICENSE.TXT
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 * @file ur_sanitizer_utils.hpp
 *
 */

#pragma once

#include "common.hpp"

namespace ur_sanitizer_layer {

struct ManagedQueue {
    ManagedQueue(ur_context_handle_t Context, ur_device_handle_t Device);
    ~ManagedQueue();

    // Disable copy semantics
    ManagedQueue(const ManagedQueue &) = delete;
    ManagedQueue &operator=(const ManagedQueue &) = delete;

    operator ur_queue_handle_t() { return Handle; }

  private:
    ur_queue_handle_t Handle = nullptr;
};

ur_context_handle_t GetContext(ur_queue_handle_t Queue);
ur_context_handle_t GetContext(ur_program_handle_t Program);
ur_context_handle_t GetContext(ur_kernel_handle_t Kernel);
ur_device_handle_t GetDevice(ur_queue_handle_t Queue);
DeviceType GetDeviceType(ur_device_handle_t Device);
std::string GetKernelName(ur_kernel_handle_t Kernel);
size_t GetLocalMemorySize(ur_device_handle_t Device);
ur_program_handle_t GetProgram(ur_kernel_handle_t Kernel);
std::vector<ur_device_handle_t> GetProgramDevices(ur_program_handle_t Program);
ur_device_handle_t GetUSMAllocDevice(ur_context_handle_t Context,
                                     const void *MemPtr);
size_t GetKernelNumArgs(ur_kernel_handle_t Kernel);
size_t GetVirtualMemGranularity(ur_context_handle_t Context,
                                ur_device_handle_t Device);

} // namespace ur_sanitizer_layer
