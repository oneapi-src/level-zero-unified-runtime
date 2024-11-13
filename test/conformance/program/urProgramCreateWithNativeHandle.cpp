// Copyright (C) 2023 Intel Corporation
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
// See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <uur/fixtures.h>

struct urProgramCreateWithNativeHandleTest : uur::urProgramTest {
    void SetUp() override {
        UUR_RETURN_ON_FATAL_FAILURE(urProgramTest::SetUp());
        {
            ur_platform_backend_t backend;
            ASSERT_SUCCESS(urPlatformGetInfo(platform, UR_PLATFORM_INFO_BACKEND,
                                             sizeof(backend), &backend,
                                             nullptr));
            // For Level Zero we have to build the program to have the native handle.
            if (backend == UR_PLATFORM_BACKEND_LEVEL_ZERO) {
                ASSERT_SUCCESS(urProgramBuild(context, program, nullptr));
            }
            UUR_ASSERT_SUCCESS_OR_UNSUPPORTED(
                urProgramGetNativeHandle(program, &native_program_handle));
        }
    }

    void TearDown() override {
        if (native_program) {
            EXPECT_SUCCESS(urProgramRelease(native_program));
        }
        UUR_RETURN_ON_FATAL_FAILURE(urProgramTest::TearDown());
    }

    ur_native_handle_t native_program_handle = 0;
    ur_program_handle_t native_program = nullptr;
};
UUR_INSTANTIATE_KERNEL_TEST_SUITE_P(urProgramCreateWithNativeHandleTest);

TEST_P(urProgramCreateWithNativeHandleTest, Success) {
    UUR_ASSERT_SUCCESS_OR_UNSUPPORTED(urProgramCreateWithNativeHandle(
        native_program_handle, context, nullptr, &native_program));

    uint32_t ref_count = 0;
    ASSERT_SUCCESS(urProgramGetInfo(native_program,
                                    UR_PROGRAM_INFO_REFERENCE_COUNT,
                                    sizeof(ref_count), &ref_count, nullptr));

    ASSERT_NE(ref_count, 0);
}

TEST_P(urProgramCreateWithNativeHandleTest, InvalidNullHandleContext) {
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_HANDLE,
                     urProgramCreateWithNativeHandle(native_program_handle,
                                                     nullptr, nullptr,
                                                     &native_program));
}

TEST_P(urProgramCreateWithNativeHandleTest, InvalidNullPointerProgram) {
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_POINTER,
                     urProgramCreateWithNativeHandle(
                         native_program_handle, context, nullptr, nullptr));
}
