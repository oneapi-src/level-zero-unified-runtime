// Copyright (C) 2023 Intel Corporation
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
//
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <uur/fixtures.h>
#include <uur/known_failure.h>

using urProgramLinkWithParam = uur::urProgramLinkWithParamBaseTest<std::string>;

UUR_DEVICE_TEST_SUITE_WITH_PARAM(
    urProgramLinkWithParam,
    ::testing::Values("-cl-denorms-are-zero", "-cl-no-signed-zeros",
                      "-cl-unsafe-math-optimizations", "-cl-finite-math-only",
                      "cl-fast-relaxed-math", "-cl-no-subgroup-ifp"),
    uur::deviceTestWithParamPrinter<std::string>);

TEST_P(urProgramLinkWithParam, Success) {
  UUR_KNOWN_FAILURE_ON(uur::OpenCL{"Intel(R) Core(TM) i9-12900K"});

  size_t property_size = 0;
  const ur_platform_info_t property_name = UR_PLATFORM_INFO_BACKEND;

  ASSERT_SUCCESS_OR_OPTIONAL_QUERY(
      urPlatformGetInfo(platform, property_name, 0, nullptr, &property_size),
      property_name);
  ASSERT_EQ(property_size, sizeof(ur_platform_backend_t));

  ur_platform_backend_t backend = UR_PLATFORM_BACKEND_UNKNOWN;
  ASSERT_SUCCESS(urPlatformGetInfo(platform, property_name, property_size,
                                   &backend, nullptr));

  ASSERT_SUCCESS(
      urProgramLink(context, 1, &program, getParam().c_str(), &linked_program));
}

using urProgramLinkTest = uur::urProgramLinkBaseTest<uur::urProgramTest>;
UUR_INSTANTIATE_DEVICE_TEST_SUITE(urProgramLinkTest);

TEST_P(urProgramLinkTest, Success) {
  // This entry point isn't implemented for HIP.
  UUR_KNOWN_FAILURE_ON(uur::HIP{});

  ASSERT_SUCCESS(urProgramLink(context, 1, &program, nullptr, &linked_program));
  ur_program_binary_type_t binary_type = UR_PROGRAM_BINARY_TYPE_NONE;
  ASSERT_SUCCESS(urProgramGetBuildInfo(
      linked_program, device, UR_PROGRAM_BUILD_INFO_BINARY_TYPE,
      sizeof(binary_type), &binary_type, nullptr));
  ASSERT_EQ(binary_type, UR_PROGRAM_BINARY_TYPE_EXECUTABLE);
}

TEST_P(urProgramLinkTest, InvalidNullHandleContext) {
  ASSERT_EQ_RESULT(
      UR_RESULT_ERROR_INVALID_NULL_HANDLE,
      urProgramLink(nullptr, 1, &program, nullptr, &linked_program));
}

TEST_P(urProgramLinkTest, InvalidNullPointerProgram) {
  ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_POINTER,
                   urProgramLink(context, 1, &program, nullptr, nullptr));
}

TEST_P(urProgramLinkTest, InvalidNullPointerInputPrograms) {
  ASSERT_EQ_RESULT(
      UR_RESULT_ERROR_INVALID_NULL_POINTER,
      urProgramLink(context, 1, nullptr, nullptr, &linked_program));
}

TEST_P(urProgramLinkTest, InvalidSizeCount) {
  ASSERT_EQ_RESULT(
      UR_RESULT_ERROR_INVALID_SIZE,
      urProgramLink(context, 0, &program, nullptr, &linked_program));
}

TEST_P(urProgramLinkTest, SetOutputOnZeroCount) {
  uintptr_t invalid_pointer;
  linked_program = reinterpret_cast<ur_program_handle_t>(&invalid_pointer);
  ASSERT_EQ_RESULT(
      UR_RESULT_ERROR_INVALID_SIZE,
      urProgramLink(context, 0, &program, nullptr, &linked_program));
  ASSERT_NE(linked_program,
            reinterpret_cast<ur_program_handle_t>(&invalid_pointer));
}

struct urProgramLinkErrorTest : uur::urQueueTest {
  const std::string linker_error_program_name = "linker_error";

  void SetUp() override {
    // We haven't got device code tests working on native cpu yet.
    UUR_KNOWN_FAILURE_ON(uur::NativeCPU{});

    UUR_RETURN_ON_FATAL_FAILURE(urQueueTest::SetUp());
    // TODO: This should use a query for urProgramCreateWithIL support or
    // rely on UR_RESULT_ERROR_UNSUPPORTED_FEATURE being returned.
    ur_platform_backend_t backend;
    ASSERT_SUCCESS(urPlatformGetInfo(platform, UR_PLATFORM_INFO_BACKEND,
                                     sizeof(ur_platform_backend_t), &backend,
                                     nullptr));
    if (backend == UR_PLATFORM_BACKEND_HIP) {
      GTEST_SKIP();
    }
    // Don't know how to produce alinker error on CUDA
    if (backend == UR_PLATFORM_BACKEND_CUDA) {
      GTEST_SKIP();
    }

    std::shared_ptr<std::vector<char>> il_binary{};
    UUR_RETURN_ON_FATAL_FAILURE(uur::KernelsEnvironment::instance->LoadSource(
        linker_error_program_name, platform, il_binary));
    ASSERT_SUCCESS(uur::KernelsEnvironment::instance->CreateProgram(
        platform, context, device, *il_binary, nullptr, &program));
    ASSERT_SUCCESS(urProgramCompile(context, program, nullptr));
  }

  void TearDown() override {
    if (linked_program) {
      EXPECT_SUCCESS(urProgramRelease(linked_program));
    }
    UUR_RETURN_ON_FATAL_FAILURE(urQueueTest::TearDown());
  }

  ur_program_handle_t program = nullptr;
  ur_program_handle_t linked_program = nullptr;
};
UUR_INSTANTIATE_DEVICE_TEST_SUITE(urProgramLinkErrorTest);

TEST_P(urProgramLinkErrorTest, LinkFailure) {
  ASSERT_EQ_RESULT(
      UR_RESULT_ERROR_PROGRAM_LINK_FAILURE,
      urProgramLink(context, 1, &program, nullptr, &linked_program));
}

TEST_P(urProgramLinkErrorTest, SetOutputOnLinkError) {
  uintptr_t invalid_pointer;
  linked_program = reinterpret_cast<ur_program_handle_t>(&invalid_pointer);
  ASSERT_EQ_RESULT(
      UR_RESULT_ERROR_PROGRAM_LINK_FAILURE,
      urProgramLink(context, 1, &program, nullptr, &linked_program));
  ASSERT_NE(linked_program,
            reinterpret_cast<ur_program_handle_t>(&invalid_pointer));
}
