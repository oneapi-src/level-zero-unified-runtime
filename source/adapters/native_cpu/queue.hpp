//===----------- queue.hpp - Native CPU Adapter ---------------------------===//
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
#include "device.hpp"

struct ur_queue_handle_t_ : RefCounted {
  ur_device_handle_t_ *const device;

  ur_queue_handle_t_(ur_device_handle_t_ *device) : device(device) {}
};
