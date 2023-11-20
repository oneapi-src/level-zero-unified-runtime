// Copyright (C) 2023 Intel Corporation
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
// See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include <uur/fixtures.h>

using urEnqueueMemImageReadTest = uur::urMemImageQueueTest;
UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(urEnqueueMemImageReadTest);

// Note that for each test, we multiply the size in pixels by 4 to account for
// each channel in the RGBA image

TEST_P(urEnqueueMemImageReadTest, Success1D) {
    std::vector<uint32_t> output(width * 4, 42);
    ASSERT_SUCCESS(urEnqueueMemImageRead(queue, image1D, true, origin, region1D,
                                         0, 0, output.data(), 0, nullptr,
                                         nullptr));
}

TEST_P(urEnqueueMemImageReadTest, Success2D) {
    std::vector<uint32_t> output(width * height * 4, 42);
    ASSERT_SUCCESS(urEnqueueMemImageRead(queue, image2D, true, origin, region2D,
                                         0, 0, output.data(), 0, nullptr,
                                         nullptr));
}

TEST_P(urEnqueueMemImageReadTest, Success3D) {
    std::vector<uint32_t> output(width * height * depth * 4, 42);
    ASSERT_SUCCESS(urEnqueueMemImageRead(queue, image3D, true, origin, region3D,
                                         0, 0, output.data(), 0, nullptr,
                                         nullptr));
}

TEST_P(urEnqueueMemImageReadTest, InvalidNullHandleQueue) {
    std::vector<uint32_t> output(width * 4, 42);
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_HANDLE,
                     urEnqueueMemImageRead(nullptr, image1D, true, origin,
                                           region1D, 0, 0, output.data(), 0,
                                           nullptr, nullptr));
}

TEST_P(urEnqueueMemImageReadTest, InvalidNullHandleImage) {
    std::vector<uint32_t> output(width * 4, 42);
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_HANDLE,
                     urEnqueueMemImageRead(queue, nullptr, true, origin,
                                           region1D, 0, 0, output.data(), 0,
                                           nullptr, nullptr));
}

TEST_P(urEnqueueMemImageReadTest, InvalidNullPointerDst) {
    std::vector<uint32_t> output(width * 4, 42);
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_POINTER,
                     urEnqueueMemImageRead(queue, image1D, true, origin,
                                           region1D, 0, 0, nullptr, 0, nullptr,
                                           nullptr));
}

TEST_P(urEnqueueMemImageReadTest, InvalidNullPtrEventWaitList) {
    std::vector<uint32_t> output(width * 4, 42);
    ASSERT_EQ_RESULT(urEnqueueMemImageRead(queue, image1D, true, origin,
                                           region1D, 0, 0, output.data(), 1,
                                           nullptr, nullptr),
                     UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST);

    ur_event_handle_t validEvent;
    ASSERT_SUCCESS(urEnqueueEventsWait(queue, 0, nullptr, &validEvent));

    ASSERT_EQ_RESULT(urEnqueueMemImageRead(queue, image1D, true, origin,
                                           region1D, 0, 0, output.data(), 0,
                                           &validEvent, nullptr),
                     UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST);

    ur_event_handle_t inv_evt = nullptr;
    ASSERT_EQ_RESULT(urEnqueueMemImageRead(queue, image1D, true, origin,
                                           region1D, 0, 0, output.data(), 1,
                                           &inv_evt, nullptr),
                     UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST);
}

TEST_P(urEnqueueMemImageReadTest, InvalidOrigin1D) {
    std::vector<uint32_t> output(width * 4, 42);
    ur_rect_offset_t bad_origin{1, 0, 0};
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_SIZE,
                     urEnqueueMemImageRead(queue, image1D, true, bad_origin,
                                           region1D, 0, 0, output.data(), 0,
                                           nullptr, nullptr));
}

TEST_P(urEnqueueMemImageReadTest, InvalidOrigin2D) {
    std::vector<uint32_t> output(width * height * 4, 42);
    ur_rect_offset_t bad_origin{0, 1, 0};
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_SIZE,
                     urEnqueueMemImageRead(queue, image2D, true, bad_origin,
                                           region2D, 0, 0, output.data(), 0,
                                           nullptr, nullptr));
}

TEST_P(urEnqueueMemImageReadTest, InvalidOrigin3D) {
    std::vector<uint32_t> output(width * height * depth * 4, 42);
    ur_rect_offset_t bad_origin{0, 0, 1};
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_SIZE,
                     urEnqueueMemImageRead(queue, image3D, true, bad_origin,
                                           region3D, 0, 0, output.data(), 0,
                                           nullptr, nullptr));
}

TEST_P(urEnqueueMemImageReadTest, InvalidRegion1D) {
    std::vector<uint32_t> output(width * 4, 42);
    ur_rect_region_t bad_region{width + 1, 1, 1};
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_SIZE,
                     urEnqueueMemImageRead(queue, image1D, true, origin,
                                           bad_region, 0, 0, output.data(), 0,
                                           nullptr, nullptr));
}

TEST_P(urEnqueueMemImageReadTest, InvalidRegion2D) {
    std::vector<uint32_t> output(width * height * 4, 42);
    ur_rect_region_t bad_region{width, height + 1, 1};
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_SIZE,
                     urEnqueueMemImageRead(queue, image2D, true, origin,
                                           bad_region, 0, 0, output.data(), 0,
                                           nullptr, nullptr));
}

TEST_P(urEnqueueMemImageReadTest, InvalidRegion3D) {
    std::vector<uint32_t> output(width * height * depth * 4, 42);
    ur_rect_region_t bad_region{width, height, depth + 1};
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_SIZE,
                     urEnqueueMemImageRead(queue, image3D, true, origin,
                                           bad_region, 0, 0, output.data(), 0,
                                           nullptr, nullptr));
}
