
// Copyright (C) 2024 Intel Corporation
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
// See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <uur/fixtures.h>
#include <uur/raii.h>

struct urMultiDeviceProgramCreateWithBinaryTest
    : uur::urMultiDeviceProgramTest {
    void SetUp() override {
        UUR_RETURN_ON_FATAL_FAILURE(urMultiDeviceProgramTest::SetUp());

        // First obtain binaries for all devices from the compiler SPIRV program.
        devices = uur::DevicesEnvironment::instance->devices;
        if (devices.size() < 2) {
            GTEST_SKIP();
        }
        ASSERT_SUCCESS(urProgramBuild(context, program, nullptr));
        size_t binary_sizes_len = 0;
        ASSERT_SUCCESS(urProgramGetInfo(program, UR_PROGRAM_INFO_BINARY_SIZES,
                                        0, nullptr, &binary_sizes_len));
        // We're expecting number of binaries equal to number of devices.
        ASSERT_EQ(binary_sizes_len / sizeof(size_t), devices.size());
        binary_sizes.resize(devices.size());
        binaries.resize(devices.size());
        ASSERT_SUCCESS(urProgramGetInfo(program, UR_PROGRAM_INFO_BINARY_SIZES,
                                        binary_sizes.size() * sizeof(size_t),
                                        binary_sizes.data(), nullptr));
        for (size_t i = 0; i < devices.size(); i++) {
            size_t binary_size = binary_sizes[i];
            binaries[i].resize(binary_size);
            pointers.push_back(binaries[i].data());
        }
        ASSERT_SUCCESS(urProgramGetInfo(program, UR_PROGRAM_INFO_BINARIES,
                                        sizeof(uint8_t *) * pointers.size(),
                                        pointers.data(), nullptr));

        // Now create a program with multiple device binaries.
        ASSERT_SUCCESS(urProgramCreateWithBinary(
            context, devices.size(), devices.data(), binary_sizes.data(),
            pointers.data(), nullptr, &binary_program));
    }

    void TearDown() override {
        if (binary_program) {
            EXPECT_SUCCESS(urProgramRelease(binary_program));
        }
        UUR_RETURN_ON_FATAL_FAILURE(urMultiDeviceProgramTest::TearDown());
    }

    std::vector<std::vector<uint8_t>> binaries;
    std::vector<ur_device_handle_t> devices;
    std::vector<const uint8_t *> pointers;
    std::vector<size_t> binary_sizes;
    ur_program_handle_t binary_program = nullptr;
};

// Create the kernel using the program created with multiple binaries and run it on all devices.
TEST_F(urMultiDeviceProgramCreateWithBinaryTest,
       CreateAndRunKernelOnAllDevices) {
    constexpr size_t global_offset = 0;
    constexpr size_t n_dimensions = 1;
    constexpr size_t global_size = 100;
    constexpr size_t local_size = 100;

    auto kernelName =
        uur::KernelsEnvironment::instance->GetEntryPointNames("foo")[0];

    for (size_t i = 1; i < devices.size(); i++) {
        uur::raii::Kernel kernel;
        ASSERT_SUCCESS(urProgramBuild(context, binary_program, nullptr));
        ASSERT_SUCCESS(
            urKernelCreate(binary_program, kernelName.data(), kernel.ptr()));

        ASSERT_SUCCESS(urEnqueueKernelLaunch(
            queues[i], kernel.get(), n_dimensions, &global_offset, &local_size,
            &global_size, 0, nullptr, nullptr));

        ASSERT_SUCCESS(urQueueFinish(queues[i]));
    }
}

TEST_F(urMultiDeviceProgramCreateWithBinaryTest, CheckCompileAndLink) {
    // TODO: Current behaviour is that we allow to compile only IL programs for Level Zero and link only programs in Object state.
    // OpenCL allows to compile and link programs created from native binaries, so probably we should align those two.
    ur_platform_backend_t backend;
    ASSERT_SUCCESS(urPlatformGetInfo(platform, UR_PLATFORM_INFO_BACKEND,
                                     sizeof(backend), &backend, nullptr));
    if (backend == UR_PLATFORM_BACKEND_LEVEL_ZERO) {
        ASSERT_EQ(urProgramCompile(context, binary_program, nullptr),
                  UR_RESULT_ERROR_INVALID_OPERATION);
        uur::raii::Program linked_program;
        ASSERT_EQ(urProgramLink(context, 1, &binary_program, nullptr,
                                linked_program.ptr()),
                  UR_RESULT_ERROR_INVALID_OPERATION);
    } else if (backend == UR_PLATFORM_BACKEND_OPENCL) {
        ASSERT_SUCCESS(urProgramCompile(context, binary_program, nullptr));
        uur::raii::Program linked_program;
        ASSERT_SUCCESS(urProgramLink(context, 1, &binary_program, nullptr,
                                     linked_program.ptr()));
    } else {
        GTEST_SKIP();
    }
}

TEST_F(urMultiDeviceProgramCreateWithBinaryTest,
       InvalidProgramBinaryForOneOfTheDevices) {
    std::vector<const uint8_t *> pointers_with_invalid_binary;
    for (size_t i = 1; i < devices.size(); i++) {
        pointers_with_invalid_binary.push_back(nullptr);
    }
    uur::raii::Program invalid_bin_program;
    ASSERT_EQ(urProgramCreateWithBinary(context, devices.size(), devices.data(),
                                        binary_sizes.data(),
                                        pointers_with_invalid_binary.data(),
                                        nullptr, invalid_bin_program.ptr()),
              UR_RESULT_ERROR_INVALID_VALUE);
}

// Test the case when program is built multiple times for different devices from context.
TEST_F(urMultiDeviceProgramCreateWithBinaryTest, MultipleBuildCalls) {
    // Run test only for level zero backend which supports urProgramBuildExp.
    ur_platform_backend_t backend;
    ASSERT_SUCCESS(urPlatformGetInfo(platform, UR_PLATFORM_INFO_BACKEND,
                                     sizeof(backend), &backend, nullptr));
    if (backend != UR_PLATFORM_BACKEND_LEVEL_ZERO) {
        GTEST_SKIP();
    }
    auto first_subset = std::vector<ur_device_handle_t>(
        devices.begin(), devices.begin() + devices.size() / 2);
    auto second_subset = std::vector<ur_device_handle_t>(
        devices.begin() + devices.size() / 2, devices.end());
    ASSERT_SUCCESS(urProgramBuildExp(binary_program, first_subset.size(),
                                     first_subset.data(), nullptr));
    auto kernelName =
        uur::KernelsEnvironment::instance->GetEntryPointNames("foo")[0];
    uur::raii::Kernel kernel;
    ASSERT_SUCCESS(
        urKernelCreate(binary_program, kernelName.data(), kernel.ptr()));
    ASSERT_SUCCESS(urProgramBuildExp(binary_program, second_subset.size(),
                                     second_subset.data(), nullptr));
    ASSERT_SUCCESS(
        urKernelCreate(binary_program, kernelName.data(), kernel.ptr()));

    // Building for the same subset of devices should not fail.
    ASSERT_SUCCESS(urProgramBuildExp(binary_program, first_subset.size(),
                                     first_subset.data(), nullptr));
}

// Test the case we get native binaries from program created with multiple binaries which wasn't built (i.e. in Native state).
TEST_F(urMultiDeviceProgramCreateWithBinaryTest,
       GetBinariesAndSizesFromProgramInNativeState) {
    size_t exp_binary_sizes_len = 0;
    std::vector<size_t> exp_binary_sizes;
    std::vector<std::vector<uint8_t>> exp_binaries;
    std::vector<const uint8_t *> exp_pointer;
    ASSERT_SUCCESS(urProgramGetInfo(binary_program,
                                    UR_PROGRAM_INFO_BINARY_SIZES, 0, nullptr,
                                    &exp_binary_sizes_len));
    auto num = exp_binary_sizes_len / sizeof(size_t);
    exp_binary_sizes.resize(num);
    exp_binaries.resize(num);
    exp_pointer.resize(num);
    ASSERT_SUCCESS(urProgramGetInfo(binary_program,
                                    UR_PROGRAM_INFO_BINARY_SIZES,
                                    exp_binary_sizes.size() * sizeof(size_t),
                                    exp_binary_sizes.data(), nullptr));
    for (size_t i = 0; i < devices.size(); i++) {
        size_t binary_size = exp_binary_sizes[i];
        exp_binaries[i].resize(binary_size);
        exp_pointer[i] = exp_binaries[i].data();
    }
    ASSERT_SUCCESS(urProgramGetInfo(program, UR_PROGRAM_INFO_BINARIES,
                                    sizeof(uint8_t *) * exp_pointer.size(),
                                    exp_pointer.data(), nullptr));

    // Verify that we get exactly what was provided at the creation step.
    ASSERT_EQ(exp_binaries, binaries);
    ASSERT_EQ(exp_binary_sizes, binary_sizes);
}

TEST_F(urMultiDeviceProgramCreateWithBinaryTest, GetIL) {
    size_t il_length = 0;
    ASSERT_SUCCESS(urProgramGetInfo(binary_program, UR_PROGRAM_INFO_IL, 0,
                                    nullptr, &il_length));
    ASSERT_EQ(il_length, 0);
    std::vector<uint8_t> il(il_length);
    ASSERT_EQ(urProgramGetInfo(binary_program, UR_PROGRAM_INFO_IL, il.size(),
                               il.data(), nullptr),
              UR_RESULT_ERROR_INVALID_NULL_POINTER);
}

TEST_F(urMultiDeviceProgramCreateWithBinaryTest, CheckProgramGetInfo) {
    std::vector<char> property_value;
    size_t property_size = 0;

    // Program is not in exe state, so error is expected.
    for (auto prop :
         {UR_PROGRAM_INFO_NUM_KERNELS, UR_PROGRAM_INFO_KERNEL_NAMES}) {
        auto result =
            urProgramGetInfo(binary_program, prop, 0, nullptr, &property_size);
        // TODO: OpenCL and Level Zero return diffent error code, it needs to be fixed.
        ASSERT_TRUE(result == UR_RESULT_ERROR_INVALID_PROGRAM_EXECUTABLE ||
                    result == UR_RESULT_ERROR_INVALID_PROGRAM);
    }

    // Now build the program and check that we can get the info.
    ASSERT_SUCCESS(urProgramBuild(context, binary_program, nullptr));

    size_t logSize;
    std::string log;

    for (auto dev : devices) {
        ASSERT_SUCCESS(urProgramGetBuildInfo(
            program, dev, UR_PROGRAM_BUILD_INFO_LOG, 0, nullptr, &logSize));
        // The size should always include the null terminator.
        ASSERT_GT(logSize, 0);
        log.resize(logSize);
        ASSERT_SUCCESS(urProgramGetBuildInfo(program, dev,
                                             UR_PROGRAM_BUILD_INFO_LOG, logSize,
                                             log.data(), nullptr));
        ASSERT_EQ(log[logSize - 1], '\0');
    }

    ASSERT_SUCCESS(urProgramGetInfo(binary_program, UR_PROGRAM_INFO_NUM_KERNELS,
                                    0, nullptr, &property_size));
    property_value.resize(property_size);
    ASSERT_SUCCESS(urProgramGetInfo(binary_program, UR_PROGRAM_INFO_NUM_KERNELS,
                                    property_size, property_value.data(),
                                    nullptr));

    auto returned_num_of_kernels =
        reinterpret_cast<uint32_t *>(property_value.data());
    ASSERT_GT(*returned_num_of_kernels, 0U);
    ASSERT_SUCCESS(urProgramGetInfo(binary_program,
                                    UR_PROGRAM_INFO_KERNEL_NAMES, 0, nullptr,
                                    &property_size));
    property_value.resize(property_size);
    ASSERT_SUCCESS(urProgramGetInfo(binary_program,
                                    UR_PROGRAM_INFO_KERNEL_NAMES, property_size,
                                    property_value.data(), nullptr));
    auto returned_kernel_names =
        reinterpret_cast<char *>(property_value.data());
    ASSERT_STRNE(returned_kernel_names, "");
}
