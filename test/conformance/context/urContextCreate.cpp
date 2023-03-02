// Copyright (C) 2023 Intel Corporation
// SPDX-License-Identifier: MIT

#include <uur/fixtures.h>

using urContextCreateTest = uur::urDeviceTest;

UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(urContextCreateTest);

TEST_P(urContextCreateTest, Success) {
    ur_context_handle_t context = nullptr;
    ASSERT_SUCCESS(urContextCreate(1, &device, nullptr, &context));
    ASSERT_NE(nullptr, context);
    ASSERT_SUCCESS(urContextRelease(context));
}

TEST_P(urContextCreateTest, SuccessWithProperties) {
    ur_context_properties_t properties{UR_STRUCTURE_TYPE_CONTEXT_PROPERTIES};
    ur_context_handle_t context = nullptr;
    ASSERT_SUCCESS(urContextCreate(1, &device, &properties, &context));
    ASSERT_NE(nullptr, context);
    ASSERT_SUCCESS(urContextRelease(context));
}

TEST_P(urContextCreateTest, InvalidNullPointerDevices) {
    ur_context_handle_t context = nullptr;
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_POINTER,
                     urContextCreate(1, nullptr, nullptr, &context));
}

TEST_P(urContextCreateTest, InvalidNullPointerContext) {
    auto device = GetParam();
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_POINTER,
                     urContextCreate(1, &device, nullptr, nullptr));
}

using urContextCreateMultiDeviceTest = uur::urAllDevicesTest;
TEST_F(urContextCreateMultiDeviceTest, Success) {
    if (devices.size() < 2) {
        GTEST_SKIP();
    }
    ur_context_handle_t context = nullptr;
    ASSERT_SUCCESS(urContextCreate(static_cast<uint32_t>(devices.size()),
                                   devices.data(), nullptr, &context));
    ASSERT_NE(nullptr, context);
    ASSERT_SUCCESS(urContextRelease(context));
}
