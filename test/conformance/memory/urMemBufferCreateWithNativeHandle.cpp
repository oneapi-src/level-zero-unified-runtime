// Copyright (C) 2023 Intel Corporation
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
// See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <uur/fixtures.h>
#include <uur/raii.h>

using urMemBufferCreateWithNativeHandleTest = uur::urMemBufferTest;
UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(urMemBufferCreateWithNativeHandleTest);

TEST_P(urMemBufferCreateWithNativeHandleTest, Success) {
    ur_native_handle_t hNativeMem = 0;
    {
        UUR_ASSERT_SUCCESS_OR_UNSUPPORTED(
            urMemGetNativeHandle(buffer, device, &hNativeMem));
    }

    // We cannot assume anything about a native_handle, not even if it's
    // `nullptr` since this could be a valid representation within a backend.
    // We can however convert the native_handle back into a unified-runtime handle
    // and perform some query on it to verify that it works.
    ur_mem_handle_t mem = nullptr;
    ur_mem_native_properties_t props = {
        /*.stype =*/UR_STRUCTURE_TYPE_MEM_NATIVE_PROPERTIES,
        /*.pNext =*/nullptr,
        /*.isNativeHandleOwned =*/false,
    };
    UUR_ASSERT_SUCCESS_OR_UNSUPPORTED(
        urMemBufferCreateWithNativeHandle(hNativeMem, context, &props, &mem));
    ASSERT_NE(mem, nullptr);

    size_t alloc_size = 0;
    ASSERT_SUCCESS(urMemGetInfo(mem, UR_MEM_INFO_SIZE, sizeof(size_t),
                                &alloc_size, nullptr));

    ASSERT_SUCCESS(urMemRelease(mem));
}

using urMemBufferMultiQueueMemBufferTest = uur::urMultiDeviceMemBufferQueueTest;

TEST_F(urMemBufferMultiQueueMemBufferTest, WriteBack) {
    void *ptr;
    ASSERT_SUCCESS(urUSMHostAlloc(context, nullptr, nullptr, size, &ptr));

    ur_mem_handle_t mem = nullptr;
    ur_mem_native_properties_t props = {
        /*.stype =*/UR_STRUCTURE_TYPE_MEM_NATIVE_PROPERTIES,
        /*.pNext =*/nullptr,
        /*.isNativeHandleOwned =*/false,
    };
    {
        UUR_ASSERT_SUCCESS_OR_UNSUPPORTED(urMemBufferCreateWithNativeHandle(
            reinterpret_cast<ur_native_handle_t>(ptr), context, &props, &mem));
    }
    ASSERT_NE(mem, nullptr);

    const uint8_t pattern = 0x11;
    std::vector<uint8_t> src(size, pattern);

    // write data to the buffer and destroy the buffer
    ASSERT_SUCCESS(urEnqueueMemBufferWrite(queues[1], mem, true, 0, size,
                                           src.data(), 0, nullptr, nullptr));
    ASSERT_SUCCESS(urMemRelease(mem));

    // Create the buffer again and read back the data, data should have been written to the
    // memory behind the native handle. Use different queue to test data migration logic.
    ASSERT_SUCCESS(urMemBufferCreateWithNativeHandle(
        reinterpret_cast<ur_native_handle_t>(ptr), context, &props, &mem));
    ASSERT_NE(mem, nullptr);

    std::vector<uint8_t> dst(size, 0);
    ASSERT_SUCCESS(urEnqueueMemBufferRead(queues[0], mem, true, 0, size,
                                          dst.data(), 0, nullptr, nullptr));

    ASSERT_EQ(src, dst);

    ASSERT_SUCCESS(urMemRelease(mem));
}
