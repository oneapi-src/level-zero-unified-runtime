/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef USM_POOL_MANAGER_HPP
#define USM_POOL_MANAGER_HPP 1

#include "ur_api.h"
#include "ur_pool_manager.hpp"
#include "ur_util.hpp"

#include <functional>
#include <vector>

namespace usm {

/// @brief describes an internal USM pool instance.
struct pool_descriptor {
    ur_usm_pool_handle_t poolHandle;

    ur_context_handle_t hContext;
    ur_device_handle_t hDevice;
    ur_usm_type_t type;
    bool deviceReadOnly;

    static bool equal(const pool_descriptor &lhs, const pool_descriptor &rhs);
    static std::size_t hash(const pool_descriptor &desc);
    static std::pair<ur_result_t, std::vector<pool_descriptor>>
    create(ur_usm_pool_handle_t poolHandle, ur_context_handle_t hContext);
};

static inline std::pair<ur_result_t, std::vector<ur_device_handle_t>>
urGetSubDevices(ur_device_handle_t hDevice) {
    uint32_t nComputeUnits;
    auto ret = urDeviceGetInfo(hDevice, UR_DEVICE_INFO_MAX_COMPUTE_UNITS,
                               sizeof(nComputeUnits), &nComputeUnits, nullptr);
    if (ret != UR_RESULT_SUCCESS) {
        return {ret, {}};
    }

    ur_device_partition_equally_desc_t part_eq_desc{
        UR_STRUCTURE_TYPE_DEVICE_PARTITION_EQUALLY_DESC, nullptr,
        nComputeUnits};

    ur_device_partition_desc_t part_desc{
        UR_STRUCTURE_TYPE_DEVICE_PARTITION_DESC, &part_eq_desc};

    // Get the number of devices that will be created
    uint32_t deviceCount;
    ret = urDevicePartition(hDevice, &part_desc, 0, nullptr, &deviceCount);
    if (ret != UR_RESULT_SUCCESS) {
        return {ret, {}};
    }

    std::vector<ur_device_handle_t> sub_devices(deviceCount);
    ret = urDevicePartition(hDevice, &part_desc,
                            static_cast<uint32_t>(sub_devices.size()),
                            sub_devices.data(), nullptr);
    if (ret != UR_RESULT_SUCCESS) {
        return {ret, {}};
    }

    return {UR_RESULT_SUCCESS, sub_devices};
}

inline std::pair<ur_result_t, std::vector<ur_device_handle_t>>
urGetAllDevicesAndSubDevices(ur_context_handle_t hContext) {
    size_t deviceCount;
    auto ret = urContextGetInfo(hContext, UR_CONTEXT_INFO_NUM_DEVICES,
                                sizeof(deviceCount), &deviceCount, nullptr);
    if (ret != UR_RESULT_SUCCESS) {
        return {ret, {}};
    }

    std::vector<ur_device_handle_t> devices(deviceCount);
    ret = urContextGetInfo(hContext, UR_CONTEXT_INFO_DEVICES,
                           sizeof(ur_device_handle_t) * deviceCount,
                           devices.data(), nullptr);
    if (ret != UR_RESULT_SUCCESS) {
        return {ret, {}};
    }

    std::vector<ur_device_handle_t> devicesAndSubDevices;
    std::function<ur_result_t(ur_device_handle_t)> addPoolsForDevicesRec =
        [&](ur_device_handle_t hDevice) {
            devicesAndSubDevices.push_back(hDevice);
            auto [ret, subDevices] = urGetSubDevices(hDevice);
            if (ret != UR_RESULT_SUCCESS) {
                return ret;
            }
            for (auto &subDevice : subDevices) {
                ret = addPoolsForDevicesRec(subDevice);
                if (ret != UR_RESULT_SUCCESS) {
                    return ret;
                }
            }
            return UR_RESULT_SUCCESS;
        };

    for (size_t i = 0; i < deviceCount; i++) {
        ret = addPoolsForDevicesRec(devices[i]);
        if (ret != UR_RESULT_SUCCESS) {
            return {ret, {}};
        }
    }

    return {UR_RESULT_SUCCESS, devicesAndSubDevices};
}

static inline bool
isSharedAllocationReadOnlyOnDevice(const pool_descriptor &desc) {
    return desc.type == UR_USM_TYPE_SHARED && desc.deviceReadOnly;
}

inline bool pool_descriptor::equal(const pool_descriptor &lhs,
                                   const pool_descriptor &rhs) {
    ur_native_handle_t lhsNative, rhsNative;

    // We want to share a memory pool for sub-devices and sub-sub devices.
    // Sub-devices and sub-sub-devices might be represented by different ur_device_handle_t but
    // they share the same native_handle_t (which is used by UMA provider).
    // Ref: https://github.com/intel/llvm/commit/86511c5dc84b5781dcfd828caadcb5cac157eae1
    // TODO: is this L0 specific?
    auto ret = urDeviceGetNativeHandle(lhs.hDevice, &lhsNative);
    if (ret != UR_RESULT_SUCCESS) {
        throw ret;
    }
    ret = urDeviceGetNativeHandle(rhs.hDevice, &rhsNative);
    if (ret != UR_RESULT_SUCCESS) {
        throw ret;
    }

    return lhsNative == rhsNative && lhs.type == rhs.type &&
           (isSharedAllocationReadOnlyOnDevice(lhs) ==
            isSharedAllocationReadOnlyOnDevice(rhs)) &&
           lhs.poolHandle == rhs.poolHandle;
}

inline std::size_t pool_descriptor::hash(const pool_descriptor &desc) {
    ur_native_handle_t native;
    auto ret = urDeviceGetNativeHandle(desc.hDevice, &native);
    if (ret != UR_RESULT_SUCCESS) {
        throw ret;
    }

    return combine_hashes(0, desc.type, native,
                          isSharedAllocationReadOnlyOnDevice(desc),
                          desc.poolHandle);
}

inline std::pair<ur_result_t, std::vector<pool_descriptor>>
pool_descriptor::create(ur_usm_pool_handle_t poolHandle,
                        ur_context_handle_t hContext) {
    auto [ret, devices] = urGetAllDevicesAndSubDevices(hContext);
    if (ret != UR_RESULT_SUCCESS) {
        return {ret, {}};
    }

    std::vector<pool_descriptor> descriptors;
    pool_descriptor &desc = descriptors.emplace_back();
    desc.poolHandle = poolHandle;
    desc.hContext = hContext;
    desc.type = UR_USM_TYPE_HOST;

    for (auto &device : devices) {
        {
            pool_descriptor &desc = descriptors.emplace_back();
            desc.poolHandle = poolHandle;
            desc.hContext = hContext;
            desc.type = UR_USM_TYPE_DEVICE;
        }
        {
            pool_descriptor &desc = descriptors.emplace_back();
            desc.poolHandle = poolHandle;
            desc.hContext = hContext;
            desc.type = UR_USM_TYPE_SHARED;
            desc.hDevice = device;
            desc.deviceReadOnly = false;
        }
        {
            pool_descriptor &desc = descriptors.emplace_back();
            desc.poolHandle = poolHandle;
            desc.hContext = hContext;
            desc.type = UR_USM_TYPE_SHARED;
            desc.hDevice = device;
            desc.deviceReadOnly = true;
        }
    }

    return {ret, descriptors};
}

} // namespace usm

#endif /* USM_POOL_MANAGER_HPP */
