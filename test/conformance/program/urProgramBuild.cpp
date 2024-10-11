// Copyright (C) 2023 Intel Corporation
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
// See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <uur/fixtures.h>

using urProgramBuildTest = uur::urProgramTest;
UUR_INSTANTIATE_KERNEL_TEST_SUITE_P(urProgramBuildTest);

TEST_P(urProgramBuildTest, Success) {
    ASSERT_SUCCESS(urProgramBuild(program, 1, &device, nullptr));
}

TEST_P(urProgramBuildTest, SuccessWithOptions) {
    const char *pOptions = "";
    ASSERT_SUCCESS(urProgramBuild(program, 1, &device, pOptions));
}

TEST_P(urProgramBuildTest, InvalidNullHandleProgram) {
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_HANDLE,
                     urProgramBuild(nullptr, 1, &device, nullptr));
}

TEST_P(urProgramBuildTest, InvalidNullPointerDevices) {
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_POINTER,
                     urProgramBuild(program, 1, nullptr, nullptr));
}

TEST_P(urProgramBuildTest, InvalidSizeNumDevices) {
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_SIZE,
                     urProgramBuild(program, 0, &device, nullptr));
}

TEST_P(urProgramBuildTest, BuildFailure) {
    ur_program_handle_t program = nullptr;
    std::shared_ptr<std::vector<char>> il_binary;
    uur::KernelsEnvironment::instance->LoadSource("build_failure", il_binary);
    if (!il_binary) {
        // The build failure we are testing for happens at SYCL compile time on
        // AMD and Nvidia, so no binary exists to check for a build failure
        GTEST_SKIP() << "Build failure test not supported on AMD/Nvidia yet";
        return;
    }

    // TODO: This seems to fail on opencl/device combination used in the Github
    // runners (`2023.16.12.0.12_195853.xmain-hotfix`). It segfaults, so we just
    // skip the test so other tests can run
    ur_platform_backend_t backend;
    ASSERT_SUCCESS(urPlatformGetInfo(platform, UR_PLATFORM_INFO_BACKEND,
                                     sizeof(ur_platform_backend_t), &backend,
                                     nullptr));
    if (backend == UR_PLATFORM_BACKEND_OPENCL) {
        GTEST_SKIP() << "Skipping opencl build failure test - segfaults on CI";
    }

    ASSERT_EQ_RESULT(UR_RESULT_SUCCESS,
                     urProgramCreateWithIL(context, il_binary->data(),
                                           il_binary->size(), nullptr,
                                           &program));
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_PROGRAM_BUILD_FAILURE,
                     urProgramBuild(program, 1, &device, nullptr));
}
