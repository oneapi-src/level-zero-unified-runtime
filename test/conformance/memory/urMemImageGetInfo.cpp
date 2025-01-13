// Copyright (C) 2023 Intel Corporation
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
// See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <uur/fixtures.h>

using urMemImageGetInfoTest = uur::urMemImageTest;
UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(urMemImageGetInfoTest);

bool operator==(ur_image_format_t lhs, ur_image_format_t rhs) {
    return lhs.channelOrder == rhs.channelOrder &&
           lhs.channelType == rhs.channelType;
}

TEST_P(urMemImageGetInfoTest, SuccessFormat) {
    size_t size = 0;
    auto info_type = UR_IMAGE_INFO_FORMAT;
    ASSERT_SUCCESS(urMemImageGetInfo(image, info_type, 0, nullptr, &size));
    ASSERT_EQ(sizeof(ur_image_format_t), size);

    ur_image_format_t returned_format = {UR_IMAGE_CHANNEL_ORDER_FORCE_UINT32,
                                         UR_IMAGE_CHANNEL_TYPE_FORCE_UINT32};

    ASSERT_SUCCESS(
        urMemImageGetInfo(image, info_type, size, &returned_format, nullptr));

    ASSERT_TRUE(returned_format == image_format);
}

TEST_P(urMemImageGetInfoTest, SuccessElementSize) {
    size_t size = 0;
    auto info_type = UR_IMAGE_INFO_ELEMENT_SIZE;
    ASSERT_SUCCESS(urMemImageGetInfo(image, info_type, 0, nullptr, &size));
    ASSERT_EQ(sizeof(size_t), size);

    size_t returned_size = 999;
    ASSERT_SUCCESS(
        urMemImageGetInfo(image, info_type, size, &returned_size, nullptr));

    ASSERT_NE(returned_size, 999);
}

TEST_P(urMemImageGetInfoTest, SuccessRowPitch) {
    size_t size = 0;
    auto info_type = UR_IMAGE_INFO_ROW_PITCH;
    ASSERT_SUCCESS(urMemImageGetInfo(image, info_type, 0, nullptr, &size));
    ASSERT_EQ(sizeof(size_t), size);

    size_t returned_row_pitch = 999;
    ASSERT_SUCCESS(urMemImageGetInfo(image, info_type, size,
                                     &returned_row_pitch, nullptr));

    ASSERT_NE(returned_row_pitch, 999);
}

TEST_P(urMemImageGetInfoTest, SuccessSlicePitch) {
    size_t size = 0;
    auto info_type = UR_IMAGE_INFO_SLICE_PITCH;
    ASSERT_SUCCESS(urMemImageGetInfo(image, info_type, 0, nullptr, &size));
    ASSERT_EQ(sizeof(size_t), size);

    size_t returned_slice_pitch = 999;
    ASSERT_SUCCESS(urMemImageGetInfo(image, info_type, size,
                                     &returned_slice_pitch, nullptr));

    ASSERT_EQ(returned_slice_pitch, image_desc.slicePitch);
}

TEST_P(urMemImageGetInfoTest, SuccessWidth) {
    size_t size = 0;
    auto info_type = UR_IMAGE_INFO_WIDTH;
    ASSERT_SUCCESS(urMemImageGetInfo(image, info_type, 0, nullptr, &size));
    ASSERT_EQ(sizeof(size_t), size);

    size_t returned_width = 999;
    ASSERT_SUCCESS(
        urMemImageGetInfo(image, info_type, size, &returned_width, nullptr));

    ASSERT_EQ(returned_width, image_desc.width);
}

TEST_P(urMemImageGetInfoTest, SuccessHeight) {
    size_t size = 0;
    auto info_type = UR_IMAGE_INFO_HEIGHT;
    ;
    ASSERT_SUCCESS(urMemImageGetInfo(image, info_type, 0, nullptr, &size));
    ASSERT_EQ(sizeof(size_t), size);

    size_t returned_height = 999;
    ASSERT_SUCCESS(
        urMemImageGetInfo(image, info_type, size, &returned_height, nullptr));

    ASSERT_EQ(returned_height, image_desc.height);
}

TEST_P(urMemImageGetInfoTest, SuccessDepth) {
    size_t size = 0;
    auto info_type = UR_IMAGE_INFO_DEPTH;
    ASSERT_SUCCESS(urMemImageGetInfo(image, info_type, 0, nullptr, &size));
    ASSERT_EQ(sizeof(size_t), size);

    size_t returned_depth = 999;
    ASSERT_SUCCESS(
        urMemImageGetInfo(image, info_type, size, &returned_depth, nullptr));

    ASSERT_NE(returned_depth, 999);
}

TEST_P(urMemImageGetInfoTest, SuccessArraySize) {
    size_t size = 0;
    auto info_type = UR_IMAGE_INFO_ARRAY_SIZE;
    ASSERT_SUCCESS(urMemImageGetInfo(image, info_type, 0, nullptr, &size));
    ASSERT_EQ(sizeof(size_t), size);

    size_t returned_depth = 999;
    ASSERT_SUCCESS(
        urMemImageGetInfo(image, info_type, size, &returned_depth, nullptr));

    ASSERT_NE(returned_depth, 999);
}

TEST_P(urMemImageGetInfoTest, SuccessNumMipMaps) {
    size_t size = 0;
    auto info_type = UR_IMAGE_INFO_NUM_MIP_LEVELS;
    ASSERT_SUCCESS(urMemImageGetInfo(image, info_type, 0, nullptr, &size));
    ASSERT_EQ(sizeof(size_t), size);

    size_t returned_depth = 999;
    ASSERT_SUCCESS(
        urMemImageGetInfo(image, info_type, size, &returned_depth, nullptr));

    ASSERT_NE(returned_depth, 999);
}

TEST_P(urMemImageGetInfoTest, SuccessNumSamples) {
    size_t size = 0;
    auto info_type = UR_IMAGE_INFO_NUM_SAMPLES;
    ASSERT_SUCCESS(urMemImageGetInfo(image, info_type, 0, nullptr, &size));
    ASSERT_EQ(sizeof(size_t), size);

    size_t returned_depth = 999;
    ASSERT_SUCCESS(
        urMemImageGetInfo(image, info_type, size, &returned_depth, nullptr));

    ASSERT_NE(returned_depth, 999);
}

TEST_P(urMemImageGetInfoTest, InvalidNullHandleImage) {
    size_t info_size = 0;
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_HANDLE,
                     urMemImageGetInfo(nullptr, UR_IMAGE_INFO_FORMAT,
                                       sizeof(size_t), &info_size, nullptr));
}

TEST_P(urMemImageGetInfoTest, InvalidEnumerationImageInfoType) {
    size_t info_size = 0;
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_ENUMERATION,
                     urMemImageGetInfo(image, UR_IMAGE_INFO_FORCE_UINT32,
                                       sizeof(size_t), &info_size, nullptr));
}

TEST_P(urMemImageGetInfoTest, InvalidSizeZero) {
    size_t info_size = 0;
    ASSERT_EQ_RESULT(
        urMemImageGetInfo(image, UR_IMAGE_INFO_FORMAT, 0, &info_size, nullptr),
        UR_RESULT_ERROR_INVALID_SIZE);
}

TEST_P(urMemImageGetInfoTest, InvalidSizeSmall) {
    // This fail is specific to the "Multi device testing" ci job.
    UUR_KNOWN_FAILURE_ON(uur::LevelZero{});

    int info_size = 0;
    ASSERT_EQ_RESULT(urMemImageGetInfo(image, UR_IMAGE_INFO_FORMAT,
                                       sizeof(info_size) - 1, &info_size,
                                       nullptr),
                     UR_RESULT_ERROR_INVALID_SIZE);
}

TEST_P(urMemImageGetInfoTest, InvalidNullPointerParamValue) {
    size_t info_size = 0;
    ASSERT_EQ_RESULT(urMemImageGetInfo(image, UR_IMAGE_INFO_FORMAT,
                                       sizeof(info_size), nullptr, nullptr),
                     UR_RESULT_ERROR_INVALID_NULL_POINTER);
}

TEST_P(urMemImageGetInfoTest, InvalidNullPointerPropSizeRet) {
    ASSERT_EQ_RESULT(
        urMemImageGetInfo(image, UR_IMAGE_INFO_FORMAT, 0, nullptr, nullptr),
        UR_RESULT_ERROR_INVALID_NULL_POINTER);
}
