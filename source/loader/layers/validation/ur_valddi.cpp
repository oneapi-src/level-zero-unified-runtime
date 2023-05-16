/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 * @file ur_valddi.cpp
 *
 */
#include "ur_leak_check.hpp"
#include "ur_validation_layer.hpp"

namespace ur_validation_layer {

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urInit
__urdlllocal ur_result_t UR_APICALL urInit(
    ur_device_init_flags_t device_flags ///< [in] device initialization flags.
    ///< must be 0 (default) or a combination of ::ur_device_init_flag_t.
) {
    auto pfnInit = context.urDdiTable.Global.pfnInit;

    if (nullptr == pfnInit) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (UR_DEVICE_INIT_FLAGS_MASK & device_flags) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }
    }

    ur_result_t result = pfnInit(device_flags);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urTearDown
__urdlllocal ur_result_t UR_APICALL urTearDown(
    void *pParams ///< [in] pointer to tear down parameters
) {
    auto pfnTearDown = context.urDdiTable.Global.pfnTearDown;

    if (nullptr == pfnTearDown) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == pParams) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnTearDown(pParams);

    if (context.enableLeakChecking) {
        refCountContext.logInvalidReferences();
        refCountContext.clear();
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urPlatformGet
__urdlllocal ur_result_t UR_APICALL urPlatformGet(
    uint32_t
        NumEntries, ///< [in] the number of platforms to be added to phPlatforms.
    ///< If phPlatforms is not NULL, then NumEntries should be greater than
    ///< zero, otherwise ::UR_RESULT_ERROR_INVALID_SIZE,
    ///< will be returned.
    ur_platform_handle_t *
        phPlatforms, ///< [out][optional][range(0, NumEntries)] array of handle of platforms.
    ///< If NumEntries is less than the number of platforms available, then
    ///< ::urPlatformGet shall only retrieve that number of platforms.
    uint32_t *
        pNumPlatforms ///< [out][optional] returns the total number of platforms available.
) {
    auto pfnGet = context.urDdiTable.Platform.pfnGet;

    if (nullptr == pfnGet) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
    }

    ur_result_t result = pfnGet(NumEntries, phPlatforms, pNumPlatforms);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urPlatformGetInfo
__urdlllocal ur_result_t UR_APICALL urPlatformGetInfo(
    ur_platform_handle_t hPlatform, ///< [in] handle of the platform
    ur_platform_info_t propName,    ///< [in] type of the info to retrieve
    size_t propSize, ///< [in] the number of bytes pointed to by pPlatformInfo.
    void *
        pPropValue, ///< [out][optional][typename(propName, propSize)] array of bytes holding
                    ///< the info.
    ///< If Size is not equal to or greater to the real number of bytes needed
    ///< to return the info then the ::UR_RESULT_ERROR_INVALID_SIZE error is
    ///< returned and pPlatformInfo is not used.
    size_t *
        pSizeRet ///< [out][optional] pointer to the actual number of bytes being queried by pPlatformInfo.
) {
    auto pfnGetInfo = context.urDdiTable.Platform.pfnGetInfo;

    if (nullptr == pfnGetInfo) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hPlatform) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (UR_PLATFORM_INFO_BACKEND < propName) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }
    }

    ur_result_t result =
        pfnGetInfo(hPlatform, propName, propSize, pPropValue, pSizeRet);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urPlatformGetApiVersion
__urdlllocal ur_result_t UR_APICALL urPlatformGetApiVersion(
    ur_platform_handle_t hPlatform, ///< [in] handle of the platform
    ur_api_version_t *pVersion      ///< [out] api version
) {
    auto pfnGetApiVersion = context.urDdiTable.Platform.pfnGetApiVersion;

    if (nullptr == pfnGetApiVersion) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hPlatform) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pVersion) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnGetApiVersion(hPlatform, pVersion);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urPlatformGetNativeHandle
__urdlllocal ur_result_t UR_APICALL urPlatformGetNativeHandle(
    ur_platform_handle_t hPlatform, ///< [in] handle of the platform.
    ur_native_handle_t *
        phNativePlatform ///< [out] a pointer to the native handle of the platform.
) {
    auto pfnGetNativeHandle = context.urDdiTable.Platform.pfnGetNativeHandle;

    if (nullptr == pfnGetNativeHandle) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hPlatform) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == phNativePlatform) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnGetNativeHandle(hPlatform, phNativePlatform);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urPlatformCreateWithNativeHandle
__urdlllocal ur_result_t UR_APICALL urPlatformCreateWithNativeHandle(
    ur_native_handle_t
        hNativePlatform, ///< [in] the native handle of the platform.
    const ur_platform_native_properties_t *
        pProperties, ///< [in][optional] pointer to native platform properties struct.
    ur_platform_handle_t *
        phPlatform ///< [out] pointer to the handle of the platform object created.
) {
    auto pfnCreateWithNativeHandle =
        context.urDdiTable.Platform.pfnCreateWithNativeHandle;

    if (nullptr == pfnCreateWithNativeHandle) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hNativePlatform) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == phPlatform) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result =
        pfnCreateWithNativeHandle(hNativePlatform, pProperties, phPlatform);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urPlatformGetBackendOption
__urdlllocal ur_result_t UR_APICALL urPlatformGetBackendOption(
    ur_platform_handle_t hPlatform, ///< [in] handle of the platform instance.
    const char
        *pFrontendOption, ///< [in] string containing the frontend option.
    const char **
        ppPlatformOption ///< [out] returns the correct platform specific compiler option based on
                         ///< the frontend option.
) {
    auto pfnGetBackendOption = context.urDdiTable.Platform.pfnGetBackendOption;

    if (nullptr == pfnGetBackendOption) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hPlatform) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pFrontendOption) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == ppPlatformOption) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result =
        pfnGetBackendOption(hPlatform, pFrontendOption, ppPlatformOption);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urGetLastResult
__urdlllocal ur_result_t UR_APICALL urGetLastResult(
    ur_platform_handle_t hPlatform, ///< [in] handle of the platform instance
    const char **
        ppMessage ///< [out] pointer to a string containing adapter specific result in string
                  ///< representation.
) {
    auto pfnGetLastResult = context.urDdiTable.Global.pfnGetLastResult;

    if (nullptr == pfnGetLastResult) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hPlatform) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == ppMessage) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnGetLastResult(hPlatform, ppMessage);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urDeviceGet
__urdlllocal ur_result_t UR_APICALL urDeviceGet(
    ur_platform_handle_t hPlatform, ///< [in] handle of the platform instance
    ur_device_type_t DeviceType,    ///< [in] the type of the devices.
    uint32_t
        NumEntries, ///< [in] the number of devices to be added to phDevices.
    ///< If phDevices in not NULL then NumEntries should be greater than zero,
    ///< otherwise ::UR_RESULT_ERROR_INVALID_VALUE,
    ///< will be returned.
    ur_device_handle_t *
        phDevices, ///< [out][optional][range(0, NumEntries)] array of handle of devices.
    ///< If NumEntries is less than the number of devices available, then
    ///< platform shall only retrieve that number of devices.
    uint32_t *pNumDevices ///< [out][optional] pointer to the number of devices.
    ///< pNumDevices will be updated with the total number of devices available.
) {
    auto pfnGet = context.urDdiTable.Device.pfnGet;

    if (nullptr == pfnGet) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hPlatform) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (UR_DEVICE_TYPE_VPU < DeviceType) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }
    }

    ur_result_t result =
        pfnGet(hPlatform, DeviceType, NumEntries, phDevices, pNumDevices);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urDeviceGetInfo
__urdlllocal ur_result_t UR_APICALL urDeviceGetInfo(
    ur_device_handle_t hDevice, ///< [in] handle of the device instance
    ur_device_info_t propName,  ///< [in] type of the info to retrieve
    size_t propSize, ///< [in] the number of bytes pointed to by pPropValue.
    void *
        pPropValue, ///< [out][optional][typename(propName, propSize)] array of bytes holding
                    ///< the info.
    ///< If propSize is not equal to or greater than the real number of bytes
    ///< needed to return the info
    ///< then the ::UR_RESULT_ERROR_INVALID_VALUE error is returned and
    ///< pPropValue is not used.
    size_t *
        pPropSizeRet ///< [out][optional] pointer to the actual size in bytes of the queried propName.
) {
    auto pfnGetInfo = context.urDdiTable.Device.pfnGetInfo;

    if (nullptr == pfnGetInfo) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hDevice) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (UR_DEVICE_INFO_HOST_PIPE_READ_WRITE_SUPPORTED < propName) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }
    }

    ur_result_t result =
        pfnGetInfo(hDevice, propName, propSize, pPropValue, pPropSizeRet);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urDeviceRetain
__urdlllocal ur_result_t UR_APICALL urDeviceRetain(
    ur_device_handle_t
        hDevice ///< [in] handle of the device to get a reference of.
) {
    auto pfnRetain = context.urDdiTable.Device.pfnRetain;

    if (nullptr == pfnRetain) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hDevice) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnRetain(hDevice);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.incrementRefCount(hDevice);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urDeviceRelease
__urdlllocal ur_result_t UR_APICALL urDeviceRelease(
    ur_device_handle_t hDevice ///< [in] handle of the device to release.
) {
    auto pfnRelease = context.urDdiTable.Device.pfnRelease;

    if (nullptr == pfnRelease) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hDevice) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnRelease(hDevice);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.decrementRefCount(hDevice);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urDevicePartition
__urdlllocal ur_result_t UR_APICALL urDevicePartition(
    ur_device_handle_t hDevice, ///< [in] handle of the device to partition.
    const ur_device_partition_property_t *
        pProperties, ///< [in] null-terminated array of <$_device_partition_t enum, value> pairs.
    uint32_t NumDevices, ///< [in] the number of sub-devices.
    ur_device_handle_t *
        phSubDevices, ///< [out][optional][range(0, NumDevices)] array of handle of devices.
    ///< If NumDevices is less than the number of sub-devices available, then
    ///< the function shall only retrieve that number of sub-devices.
    uint32_t *
        pNumDevicesRet ///< [out][optional] pointer to the number of sub-devices the device can be
    ///< partitioned into according to the partitioning property.
) {
    auto pfnPartition = context.urDdiTable.Device.pfnPartition;

    if (nullptr == pfnPartition) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hDevice) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pProperties) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnPartition(hDevice, pProperties, NumDevices,
                                      phSubDevices, pNumDevicesRet);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urDeviceSelectBinary
__urdlllocal ur_result_t UR_APICALL urDeviceSelectBinary(
    ur_device_handle_t
        hDevice, ///< [in] handle of the device to select binary for.
    const ur_device_binary_t
        *pBinaries,       ///< [in] the array of binaries to select from.
    uint32_t NumBinaries, ///< [in] the number of binaries passed in ppBinaries.
                          ///< Must greater than or equal to zero otherwise
                          ///< ::UR_RESULT_ERROR_INVALID_VALUE is returned.
    uint32_t *
        pSelectedBinary ///< [out] the index of the selected binary in the input array of binaries.
    ///< If a suitable binary was not found the function returns ::UR_RESULT_ERROR_INVALID_BINARY.
) {
    auto pfnSelectBinary = context.urDdiTable.Device.pfnSelectBinary;

    if (nullptr == pfnSelectBinary) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hDevice) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pBinaries) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == pSelectedBinary) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NumBinaries == 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }
    }

    ur_result_t result =
        pfnSelectBinary(hDevice, pBinaries, NumBinaries, pSelectedBinary);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urDeviceGetNativeHandle
__urdlllocal ur_result_t UR_APICALL urDeviceGetNativeHandle(
    ur_device_handle_t hDevice, ///< [in] handle of the device.
    ur_native_handle_t
        *phNativeDevice ///< [out] a pointer to the native handle of the device.
) {
    auto pfnGetNativeHandle = context.urDdiTable.Device.pfnGetNativeHandle;

    if (nullptr == pfnGetNativeHandle) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hDevice) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == phNativeDevice) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnGetNativeHandle(hDevice, phNativeDevice);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urDeviceCreateWithNativeHandle
__urdlllocal ur_result_t UR_APICALL urDeviceCreateWithNativeHandle(
    ur_native_handle_t hNativeDevice, ///< [in] the native handle of the device.
    ur_platform_handle_t hPlatform,   ///< [in] handle of the platform instance
    const ur_device_native_properties_t *
        pProperties, ///< [in][optional] pointer to native device properties struct.
    ur_device_handle_t
        *phDevice ///< [out] pointer to the handle of the device object created.
) {
    auto pfnCreateWithNativeHandle =
        context.urDdiTable.Device.pfnCreateWithNativeHandle;

    if (nullptr == pfnCreateWithNativeHandle) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hNativeDevice) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hPlatform) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == phDevice) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnCreateWithNativeHandle(hNativeDevice, hPlatform,
                                                   pProperties, phDevice);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.createRefCount(*phDevice);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urDeviceGetGlobalTimestamps
__urdlllocal ur_result_t UR_APICALL urDeviceGetGlobalTimestamps(
    ur_device_handle_t hDevice, ///< [in] handle of the device instance
    uint64_t *
        pDeviceTimestamp, ///< [out][optional] pointer to the Device's global timestamp that
                          ///< correlates with the Host's global timestamp value
    uint64_t *
        pHostTimestamp ///< [out][optional] pointer to the Host's global timestamp that
                       ///< correlates with the Device's global timestamp value
) {
    auto pfnGetGlobalTimestamps =
        context.urDdiTable.Device.pfnGetGlobalTimestamps;

    if (nullptr == pfnGetGlobalTimestamps) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hDevice) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result =
        pfnGetGlobalTimestamps(hDevice, pDeviceTimestamp, pHostTimestamp);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urContextCreate
__urdlllocal ur_result_t UR_APICALL urContextCreate(
    uint32_t DeviceCount, ///< [in] the number of devices given in phDevices
    const ur_device_handle_t
        *phDevices, ///< [in][range(0, DeviceCount)] array of handle of devices.
    const ur_context_properties_t *
        pProperties, ///< [in][optional] pointer to context creation properties.
    ur_context_handle_t
        *phContext ///< [out] pointer to handle of context object created
) {
    auto pfnCreate = context.urDdiTable.Context.pfnCreate;

    if (nullptr == pfnCreate) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == phDevices) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == phContext) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result =
        pfnCreate(DeviceCount, phDevices, pProperties, phContext);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.createRefCount(*phContext);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urContextRetain
__urdlllocal ur_result_t UR_APICALL urContextRetain(
    ur_context_handle_t
        hContext ///< [in] handle of the context to get a reference of.
) {
    auto pfnRetain = context.urDdiTable.Context.pfnRetain;

    if (nullptr == pfnRetain) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnRetain(hContext);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.incrementRefCount(hContext);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urContextRelease
__urdlllocal ur_result_t UR_APICALL urContextRelease(
    ur_context_handle_t hContext ///< [in] handle of the context to release.
) {
    auto pfnRelease = context.urDdiTable.Context.pfnRelease;

    if (nullptr == pfnRelease) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnRelease(hContext);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.decrementRefCount(hContext);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urContextGetInfo
__urdlllocal ur_result_t UR_APICALL urContextGetInfo(
    ur_context_handle_t hContext, ///< [in] handle of the context
    ur_context_info_t propName,   ///< [in] type of the info to retrieve
    size_t
        propSize, ///< [in] the number of bytes of memory pointed to by pPropValue.
    void *
        pPropValue, ///< [out][optional][typename(propName, propSize)] array of bytes holding
                    ///< the info.
    ///< if propSize is not equal to or greater than the real number of bytes
    ///< needed to return
    ///< the info then the ::UR_RESULT_ERROR_INVALID_SIZE error is returned and
    ///< pPropValue is not used.
    size_t *
        pPropSizeRet ///< [out][optional] pointer to the actual size in bytes of the queried propName.
) {
    auto pfnGetInfo = context.urDdiTable.Context.pfnGetInfo;

    if (nullptr == pfnGetInfo) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (UR_CONTEXT_INFO_ATOMIC_FENCE_SCOPE_CAPABILITIES < propName) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }
    }

    ur_result_t result =
        pfnGetInfo(hContext, propName, propSize, pPropValue, pPropSizeRet);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urContextGetNativeHandle
__urdlllocal ur_result_t UR_APICALL urContextGetNativeHandle(
    ur_context_handle_t hContext, ///< [in] handle of the context.
    ur_native_handle_t *
        phNativeContext ///< [out] a pointer to the native handle of the context.
) {
    auto pfnGetNativeHandle = context.urDdiTable.Context.pfnGetNativeHandle;

    if (nullptr == pfnGetNativeHandle) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == phNativeContext) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnGetNativeHandle(hContext, phNativeContext);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urContextCreateWithNativeHandle
__urdlllocal ur_result_t UR_APICALL urContextCreateWithNativeHandle(
    ur_native_handle_t
        hNativeContext,  ///< [in] the native handle of the context.
    uint32_t numDevices, ///< [in] number of devices associated with the context
    const ur_device_handle_t *
        phDevices, ///< [in][range(0, numDevices)] list of devices associated with the context
    const ur_context_native_properties_t *
        pProperties, ///< [in][optional] pointer to native context properties struct
    ur_context_handle_t *
        phContext ///< [out] pointer to the handle of the context object created.
) {
    auto pfnCreateWithNativeHandle =
        context.urDdiTable.Context.pfnCreateWithNativeHandle;

    if (nullptr == pfnCreateWithNativeHandle) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hNativeContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == phDevices) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == phContext) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnCreateWithNativeHandle(
        hNativeContext, numDevices, phDevices, pProperties, phContext);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.createRefCount(*phContext);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urContextSetExtendedDeleter
__urdlllocal ur_result_t UR_APICALL urContextSetExtendedDeleter(
    ur_context_handle_t hContext, ///< [in] handle of the context.
    ur_context_extended_deleter_t
        pfnDeleter, ///< [in] Function pointer to extended deleter.
    void *
        pUserData ///< [in][out][optional] pointer to data to be passed to callback.
) {
    auto pfnSetExtendedDeleter =
        context.urDdiTable.Context.pfnSetExtendedDeleter;

    if (nullptr == pfnSetExtendedDeleter) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pfnDeleter) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnSetExtendedDeleter(hContext, pfnDeleter, pUserData);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urMemImageCreate
__urdlllocal ur_result_t UR_APICALL urMemImageCreate(
    ur_context_handle_t hContext, ///< [in] handle of the context object
    ur_mem_flags_t flags, ///< [in] allocation and usage information flags
    const ur_image_format_t
        *pImageFormat, ///< [in] pointer to image format specification
    const ur_image_desc_t *pImageDesc, ///< [in] pointer to image description
    void *pHost,           ///< [in][optional] pointer to the buffer data
    ur_mem_handle_t *phMem ///< [out] pointer to handle of image object created
) {
    auto pfnImageCreate = context.urDdiTable.Mem.pfnImageCreate;

    if (nullptr == pfnImageCreate) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pImageFormat) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == pImageDesc) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == phMem) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (UR_MEM_FLAGS_MASK & flags) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }

        if (pImageDesc && UR_MEM_TYPE_IMAGE1D_BUFFER < pImageDesc->type) {
            return UR_RESULT_ERROR_INVALID_IMAGE_FORMAT_DESCRIPTOR;
        }

        if (pHost == NULL &&
            (flags & (UR_MEM_FLAG_USE_HOST_POINTER |
                      UR_MEM_FLAG_ALLOC_COPY_HOST_POINTER)) != 0) {
            return UR_RESULT_ERROR_INVALID_HOST_PTR;
        }

        if (pHost != NULL &&
            (flags & (UR_MEM_FLAG_USE_HOST_POINTER |
                      UR_MEM_FLAG_ALLOC_COPY_HOST_POINTER)) == 0) {
            return UR_RESULT_ERROR_INVALID_HOST_PTR;
        }
    }

    ur_result_t result =
        pfnImageCreate(hContext, flags, pImageFormat, pImageDesc, pHost, phMem);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urMemBufferCreate
__urdlllocal ur_result_t UR_APICALL urMemBufferCreate(
    ur_context_handle_t hContext, ///< [in] handle of the context object
    ur_mem_flags_t flags, ///< [in] allocation and usage information flags
    size_t size, ///< [in] size in bytes of the memory object to be allocated
    const ur_buffer_properties_t
        *pProperties, ///< [in][optional] pointer to buffer creation properties
    ur_mem_handle_t
        *phBuffer ///< [out] pointer to handle of the memory buffer created
) {
    auto pfnBufferCreate = context.urDdiTable.Mem.pfnBufferCreate;

    if (nullptr == pfnBufferCreate) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == phBuffer) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (UR_MEM_FLAGS_MASK & flags) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }

        if (pProperties == NULL &&
            (flags & (UR_MEM_FLAG_USE_HOST_POINTER |
                      UR_MEM_FLAG_ALLOC_COPY_HOST_POINTER)) != 0) {
            return UR_RESULT_ERROR_INVALID_HOST_PTR;
        }

        if (pProperties != NULL && pProperties->pHost == NULL &&
            (flags & (UR_MEM_FLAG_USE_HOST_POINTER |
                      UR_MEM_FLAG_ALLOC_COPY_HOST_POINTER)) != 0) {
            return UR_RESULT_ERROR_INVALID_HOST_PTR;
        }

        if (pProperties != NULL && pProperties->pHost != NULL &&
            (flags & (UR_MEM_FLAG_USE_HOST_POINTER |
                      UR_MEM_FLAG_ALLOC_COPY_HOST_POINTER)) == 0) {
            return UR_RESULT_ERROR_INVALID_HOST_PTR;
        }
    }

    ur_result_t result =
        pfnBufferCreate(hContext, flags, size, pProperties, phBuffer);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urMemRetain
__urdlllocal ur_result_t UR_APICALL urMemRetain(
    ur_mem_handle_t hMem ///< [in] handle of the memory object to get access
) {
    auto pfnRetain = context.urDdiTable.Mem.pfnRetain;

    if (nullptr == pfnRetain) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hMem) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnRetain(hMem);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.incrementRefCount(hMem);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urMemRelease
__urdlllocal ur_result_t UR_APICALL urMemRelease(
    ur_mem_handle_t hMem ///< [in] handle of the memory object to release
) {
    auto pfnRelease = context.urDdiTable.Mem.pfnRelease;

    if (nullptr == pfnRelease) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hMem) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnRelease(hMem);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.decrementRefCount(hMem);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urMemBufferPartition
__urdlllocal ur_result_t UR_APICALL urMemBufferPartition(
    ur_mem_handle_t
        hBuffer,          ///< [in] handle of the buffer object to allocate from
    ur_mem_flags_t flags, ///< [in] allocation and usage information flags
    ur_buffer_create_type_t bufferCreateType, ///< [in] buffer creation type
    const ur_buffer_region_t
        *pRegion, ///< [in] pointer to buffer create region information
    ur_mem_handle_t
        *phMem ///< [out] pointer to the handle of sub buffer created
) {
    auto pfnBufferPartition = context.urDdiTable.Mem.pfnBufferPartition;

    if (nullptr == pfnBufferPartition) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hBuffer) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pRegion) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == phMem) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (UR_MEM_FLAGS_MASK & flags) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }

        if (UR_BUFFER_CREATE_TYPE_REGION < bufferCreateType) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }
    }

    ur_result_t result =
        pfnBufferPartition(hBuffer, flags, bufferCreateType, pRegion, phMem);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urMemGetNativeHandle
__urdlllocal ur_result_t UR_APICALL urMemGetNativeHandle(
    ur_mem_handle_t hMem, ///< [in] handle of the mem.
    ur_native_handle_t
        *phNativeMem ///< [out] a pointer to the native handle of the mem.
) {
    auto pfnGetNativeHandle = context.urDdiTable.Mem.pfnGetNativeHandle;

    if (nullptr == pfnGetNativeHandle) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hMem) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == phNativeMem) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnGetNativeHandle(hMem, phNativeMem);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urMemBufferCreateWithNativeHandle
__urdlllocal ur_result_t UR_APICALL urMemBufferCreateWithNativeHandle(
    ur_native_handle_t hNativeMem, ///< [in] the native handle to the memory.
    ur_context_handle_t hContext,  ///< [in] handle of the context object.
    const ur_mem_native_properties_t *
        pProperties, ///< [in][optional] pointer to native memory creation properties.
    ur_mem_handle_t
        *phMem ///< [out] pointer to handle of buffer memory object created.
) {
    auto pfnBufferCreateWithNativeHandle =
        context.urDdiTable.Mem.pfnBufferCreateWithNativeHandle;

    if (nullptr == pfnBufferCreateWithNativeHandle) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hNativeMem) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == phMem) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnBufferCreateWithNativeHandle(hNativeMem, hContext,
                                                         pProperties, phMem);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urMemImageCreateWithNativeHandle
__urdlllocal ur_result_t UR_APICALL urMemImageCreateWithNativeHandle(
    ur_native_handle_t hNativeMem, ///< [in] the native handle to the memory.
    ur_context_handle_t hContext,  ///< [in] handle of the context object.
    const ur_image_format_t
        *pImageFormat, ///< [in] pointer to image format specification.
    const ur_image_desc_t *pImageDesc, ///< [in] pointer to image description.
    const ur_mem_native_properties_t *
        pProperties, ///< [in][optional] pointer to native memory creation properties.
    ur_mem_handle_t
        *phMem ///< [out] pointer to handle of image memory object created.
) {
    auto pfnImageCreateWithNativeHandle =
        context.urDdiTable.Mem.pfnImageCreateWithNativeHandle;

    if (nullptr == pfnImageCreateWithNativeHandle) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hNativeMem) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pImageFormat) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == pImageDesc) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == phMem) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnImageCreateWithNativeHandle(
        hNativeMem, hContext, pImageFormat, pImageDesc, pProperties, phMem);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urMemGetInfo
__urdlllocal ur_result_t UR_APICALL urMemGetInfo(
    ur_mem_handle_t
        hMemory,            ///< [in] handle to the memory object being queried.
    ur_mem_info_t propName, ///< [in] type of the info to retrieve.
    size_t
        propSize, ///< [in] the number of bytes of memory pointed to by pPropValue.
    void *
        pPropValue, ///< [out][optional][typename(propName, propSize)] array of bytes holding
                    ///< the info.
    ///< If propSize is less than the real number of bytes needed to return
    ///< the info then the ::UR_RESULT_ERROR_INVALID_SIZE error is returned and
    ///< pPropValue is not used.
    size_t *
        pPropSizeRet ///< [out][optional] pointer to the actual size in bytes of the queried propName.
) {
    auto pfnGetInfo = context.urDdiTable.Mem.pfnGetInfo;

    if (nullptr == pfnGetInfo) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hMemory) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (UR_MEM_INFO_CONTEXT < propName) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }
    }

    ur_result_t result =
        pfnGetInfo(hMemory, propName, propSize, pPropValue, pPropSizeRet);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urMemImageGetInfo
__urdlllocal ur_result_t UR_APICALL urMemImageGetInfo(
    ur_mem_handle_t hMemory, ///< [in] handle to the image object being queried.
    ur_image_info_t propName, ///< [in] type of image info to retrieve.
    size_t
        propSize, ///< [in] the number of bytes of memory pointer to by pPropValue.
    void *
        pPropValue, ///< [out][optional][typename(propName, propSize)] array of bytes holding
                    ///< the info.
    ///< If propSize is less than the real number of bytes needed to return
    ///< the info then the ::UR_RESULT_ERROR_INVALID_SIZE error is returned and
    ///< pPropValue is not used.
    size_t *
        pPropSizeRet ///< [out][optional] pointer to the actual size in bytes of the queried propName.
) {
    auto pfnImageGetInfo = context.urDdiTable.Mem.pfnImageGetInfo;

    if (nullptr == pfnImageGetInfo) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hMemory) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (UR_IMAGE_INFO_DEPTH < propName) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }
    }

    ur_result_t result =
        pfnImageGetInfo(hMemory, propName, propSize, pPropValue, pPropSizeRet);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urSamplerCreate
__urdlllocal ur_result_t UR_APICALL urSamplerCreate(
    ur_context_handle_t hContext,   ///< [in] handle of the context object
    const ur_sampler_desc_t *pDesc, ///< [in] pointer to the sampler description
    ur_sampler_handle_t
        *phSampler ///< [out] pointer to handle of sampler object created
) {
    auto pfnCreate = context.urDdiTable.Sampler.pfnCreate;

    if (nullptr == pfnCreate) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pDesc) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == phSampler) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (UR_SAMPLER_ADDRESSING_MODE_MIRRORED_REPEAT <
            pDesc->addressingMode) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }

        if (UR_SAMPLER_FILTER_MODE_LINEAR < pDesc->filterMode) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }
    }

    ur_result_t result = pfnCreate(hContext, pDesc, phSampler);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.createRefCount(*phSampler);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urSamplerRetain
__urdlllocal ur_result_t UR_APICALL urSamplerRetain(
    ur_sampler_handle_t
        hSampler ///< [in] handle of the sampler object to get access
) {
    auto pfnRetain = context.urDdiTable.Sampler.pfnRetain;

    if (nullptr == pfnRetain) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hSampler) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnRetain(hSampler);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.incrementRefCount(hSampler);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urSamplerRelease
__urdlllocal ur_result_t UR_APICALL urSamplerRelease(
    ur_sampler_handle_t
        hSampler ///< [in] handle of the sampler object to release
) {
    auto pfnRelease = context.urDdiTable.Sampler.pfnRelease;

    if (nullptr == pfnRelease) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hSampler) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnRelease(hSampler);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.decrementRefCount(hSampler);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urSamplerGetInfo
__urdlllocal ur_result_t UR_APICALL urSamplerGetInfo(
    ur_sampler_handle_t hSampler, ///< [in] handle of the sampler object
    ur_sampler_info_t propName, ///< [in] name of the sampler property to query
    size_t
        propSize, ///< [in] size in bytes of the sampler property value provided
    void *
        pPropValue, ///< [out][typename(propName, propSize)] value of the sampler property
    size_t *
        pPropSizeRet ///< [out] size in bytes returned in sampler property value
) {
    auto pfnGetInfo = context.urDdiTable.Sampler.pfnGetInfo;

    if (nullptr == pfnGetInfo) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hSampler) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pPropValue) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == pPropSizeRet) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (UR_SAMPLER_INFO_FILTER_MODE < propName) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }
    }

    ur_result_t result =
        pfnGetInfo(hSampler, propName, propSize, pPropValue, pPropSizeRet);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urSamplerGetNativeHandle
__urdlllocal ur_result_t UR_APICALL urSamplerGetNativeHandle(
    ur_sampler_handle_t hSampler, ///< [in] handle of the sampler.
    ur_native_handle_t *
        phNativeSampler ///< [out] a pointer to the native handle of the sampler.
) {
    auto pfnGetNativeHandle = context.urDdiTable.Sampler.pfnGetNativeHandle;

    if (nullptr == pfnGetNativeHandle) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hSampler) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == phNativeSampler) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnGetNativeHandle(hSampler, phNativeSampler);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urSamplerCreateWithNativeHandle
__urdlllocal ur_result_t UR_APICALL urSamplerCreateWithNativeHandle(
    ur_native_handle_t
        hNativeSampler,           ///< [in] the native handle of the sampler.
    ur_context_handle_t hContext, ///< [in] handle of the context object
    ur_sampler_handle_t *
        phSampler ///< [out] pointer to the handle of the sampler object created.
) {
    auto pfnCreateWithNativeHandle =
        context.urDdiTable.Sampler.pfnCreateWithNativeHandle;

    if (nullptr == pfnCreateWithNativeHandle) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hNativeSampler) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == phSampler) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result =
        pfnCreateWithNativeHandle(hNativeSampler, hContext, phSampler);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.createRefCount(*phSampler);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urUSMHostAlloc
__urdlllocal ur_result_t UR_APICALL urUSMHostAlloc(
    ur_context_handle_t hContext, ///< [in] handle of the context object
    const ur_usm_desc_t
        *pUSMDesc, ///< [in][optional] USM memory allocation descriptor
    ur_usm_pool_handle_t
        pool, ///< [in][optional] Pointer to a pool created using urUSMPoolCreate
    size_t
        size, ///< [in] size in bytes of the USM memory object to be allocated
    void **ppMem ///< [out] pointer to USM host memory object
) {
    auto pfnHostAlloc = context.urDdiTable.USM.pfnHostAlloc;

    if (nullptr == pfnHostAlloc) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == ppMem) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (pUSMDesc && pUSMDesc->align != 0 &&
            ((pUSMDesc->align & (pUSMDesc->align - 1)) != 0)) {
            return UR_RESULT_ERROR_INVALID_VALUE;
        }

        if (size == 0) {
            return UR_RESULT_ERROR_INVALID_USM_SIZE;
        }
    }

    ur_result_t result = pfnHostAlloc(hContext, pUSMDesc, pool, size, ppMem);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urUSMDeviceAlloc
__urdlllocal ur_result_t UR_APICALL urUSMDeviceAlloc(
    ur_context_handle_t hContext, ///< [in] handle of the context object
    ur_device_handle_t hDevice,   ///< [in] handle of the device object
    const ur_usm_desc_t
        *pUSMDesc, ///< [in][optional] USM memory allocation descriptor
    ur_usm_pool_handle_t
        pool, ///< [in][optional] Pointer to a pool created using urUSMPoolCreate
    size_t
        size, ///< [in] size in bytes of the USM memory object to be allocated
    void **ppMem ///< [out] pointer to USM device memory object
) {
    auto pfnDeviceAlloc = context.urDdiTable.USM.pfnDeviceAlloc;

    if (nullptr == pfnDeviceAlloc) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hDevice) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == ppMem) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (pUSMDesc && pUSMDesc->align != 0 &&
            ((pUSMDesc->align & (pUSMDesc->align - 1)) != 0)) {
            return UR_RESULT_ERROR_INVALID_VALUE;
        }

        if (size == 0) {
            return UR_RESULT_ERROR_INVALID_USM_SIZE;
        }
    }

    ur_result_t result =
        pfnDeviceAlloc(hContext, hDevice, pUSMDesc, pool, size, ppMem);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urUSMSharedAlloc
__urdlllocal ur_result_t UR_APICALL urUSMSharedAlloc(
    ur_context_handle_t hContext, ///< [in] handle of the context object
    ur_device_handle_t hDevice,   ///< [in] handle of the device object
    const ur_usm_desc_t *
        pUSMDesc, ///< [in][optional] Pointer to USM memory allocation descriptor.
    ur_usm_pool_handle_t
        pool, ///< [in][optional] Pointer to a pool created using urUSMPoolCreate
    size_t
        size, ///< [in] size in bytes of the USM memory object to be allocated
    void **ppMem ///< [out] pointer to USM shared memory object
) {
    auto pfnSharedAlloc = context.urDdiTable.USM.pfnSharedAlloc;

    if (nullptr == pfnSharedAlloc) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hDevice) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == ppMem) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (pUSMDesc && pUSMDesc->align != 0 &&
            ((pUSMDesc->align & (pUSMDesc->align - 1)) != 0)) {
            return UR_RESULT_ERROR_INVALID_VALUE;
        }

        if (size == 0) {
            return UR_RESULT_ERROR_INVALID_USM_SIZE;
        }
    }

    ur_result_t result =
        pfnSharedAlloc(hContext, hDevice, pUSMDesc, pool, size, ppMem);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urUSMFree
__urdlllocal ur_result_t UR_APICALL urUSMFree(
    ur_context_handle_t hContext, ///< [in] handle of the context object
    void *pMem                    ///< [in] pointer to USM memory object
) {
    auto pfnFree = context.urDdiTable.USM.pfnFree;

    if (nullptr == pfnFree) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pMem) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnFree(hContext, pMem);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urUSMGetMemAllocInfo
__urdlllocal ur_result_t UR_APICALL urUSMGetMemAllocInfo(
    ur_context_handle_t hContext, ///< [in] handle of the context object
    const void *pMem,             ///< [in] pointer to USM memory object
    ur_usm_alloc_info_t
        propName, ///< [in] the name of the USM allocation property to query
    size_t
        propSize, ///< [in] size in bytes of the USM allocation property value
    void *
        pPropValue, ///< [out][optional][typename(propName, propSize)] value of the USM
                    ///< allocation property
    size_t *
        pPropSizeRet ///< [out][optional] bytes returned in USM allocation property
) {
    auto pfnGetMemAllocInfo = context.urDdiTable.USM.pfnGetMemAllocInfo;

    if (nullptr == pfnGetMemAllocInfo) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pMem) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (UR_USM_ALLOC_INFO_POOL < propName) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }
    }

    ur_result_t result = pfnGetMemAllocInfo(hContext, pMem, propName, propSize,
                                            pPropValue, pPropSizeRet);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urUSMPoolCreate
__urdlllocal ur_result_t UR_APICALL urUSMPoolCreate(
    ur_context_handle_t hContext, ///< [in] handle of the context object
    ur_usm_pool_desc_t *
        pPoolDesc, ///< [in] pointer to USM pool descriptor. Can be chained with
                   ///< ::ur_usm_pool_limits_desc_t
    ur_usm_pool_handle_t *ppPool ///< [out] pointer to USM memory pool
) {
    auto pfnPoolCreate = context.urDdiTable.USM.pfnPoolCreate;

    if (nullptr == pfnPoolCreate) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pPoolDesc) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == ppPool) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (UR_USM_POOL_FLAGS_MASK & pPoolDesc->flags) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }
    }

    ur_result_t result = pfnPoolCreate(hContext, pPoolDesc, ppPool);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urUSMPoolDestroy
__urdlllocal ur_result_t UR_APICALL urUSMPoolDestroy(
    ur_context_handle_t hContext, ///< [in] handle of the context object
    ur_usm_pool_handle_t pPool    ///< [in] pointer to USM memory pool
) {
    auto pfnPoolDestroy = context.urDdiTable.USM.pfnPoolDestroy;

    if (nullptr == pfnPoolDestroy) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pPool) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnPoolDestroy(hContext, pPool);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urProgramCreateWithIL
__urdlllocal ur_result_t UR_APICALL urProgramCreateWithIL(
    ur_context_handle_t hContext, ///< [in] handle of the context instance
    const void *pIL,              ///< [in] pointer to IL binary.
    size_t length,                ///< [in] length of `pIL` in bytes.
    const ur_program_properties_t *
        pProperties, ///< [in][optional] pointer to program creation properties.
    ur_program_handle_t
        *phProgram ///< [out] pointer to handle of program object created.
) {
    auto pfnCreateWithIL = context.urDdiTable.Program.pfnCreateWithIL;

    if (nullptr == pfnCreateWithIL) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pIL) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == phProgram) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL != pProperties && pProperties->count > 0 &&
            NULL == pProperties->pMetadatas) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL != pProperties && NULL != pProperties->pMetadatas &&
            pProperties->count == 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }
    }

    ur_result_t result =
        pfnCreateWithIL(hContext, pIL, length, pProperties, phProgram);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.createRefCount(*phProgram);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urProgramCreateWithBinary
__urdlllocal ur_result_t UR_APICALL urProgramCreateWithBinary(
    ur_context_handle_t hContext, ///< [in] handle of the context instance
    ur_device_handle_t
        hDevice,            ///< [in] handle to device associated with binary.
    size_t size,            ///< [in] size in bytes.
    const uint8_t *pBinary, ///< [in] pointer to binary.
    const ur_program_properties_t *
        pProperties, ///< [in][optional] pointer to program creation properties.
    ur_program_handle_t
        *phProgram ///< [out] pointer to handle of Program object created.
) {
    auto pfnCreateWithBinary = context.urDdiTable.Program.pfnCreateWithBinary;

    if (nullptr == pfnCreateWithBinary) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hDevice) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pBinary) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == phProgram) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL != pProperties && pProperties->count > 0 &&
            NULL == pProperties->pMetadatas) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL != pProperties && NULL != pProperties->pMetadatas &&
            pProperties->count == 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }
    }

    ur_result_t result = pfnCreateWithBinary(hContext, hDevice, size, pBinary,
                                             pProperties, phProgram);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.createRefCount(*phProgram);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urProgramBuild
__urdlllocal ur_result_t UR_APICALL urProgramBuild(
    ur_context_handle_t hContext, ///< [in] handle of the context instance.
    ur_program_handle_t hProgram, ///< [in] Handle of the program to build.
    const char *
        pOptions ///< [in][optional] pointer to build options null-terminated string.
) {
    auto pfnBuild = context.urDdiTable.Program.pfnBuild;

    if (nullptr == pfnBuild) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hProgram) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnBuild(hContext, hProgram, pOptions);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urProgramCompile
__urdlllocal ur_result_t UR_APICALL urProgramCompile(
    ur_context_handle_t hContext, ///< [in] handle of the context instance.
    ur_program_handle_t
        hProgram, ///< [in][out] handle of the program to compile.
    const char *
        pOptions ///< [in][optional] pointer to build options null-terminated string.
) {
    auto pfnCompile = context.urDdiTable.Program.pfnCompile;

    if (nullptr == pfnCompile) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hProgram) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnCompile(hContext, hProgram, pOptions);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urProgramLink
__urdlllocal ur_result_t UR_APICALL urProgramLink(
    ur_context_handle_t hContext, ///< [in] handle of the context instance.
    uint32_t count, ///< [in] number of program handles in `phPrograms`.
    const ur_program_handle_t *
        phPrograms, ///< [in][range(0, count)] pointer to array of program handles.
    const char *
        pOptions, ///< [in][optional] pointer to linker options null-terminated string.
    ur_program_handle_t
        *phProgram ///< [out] pointer to handle of program object created.
) {
    auto pfnLink = context.urDdiTable.Program.pfnLink;

    if (nullptr == pfnLink) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == phPrograms) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == phProgram) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (count == 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }
    }

    ur_result_t result =
        pfnLink(hContext, count, phPrograms, pOptions, phProgram);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urProgramRetain
__urdlllocal ur_result_t UR_APICALL urProgramRetain(
    ur_program_handle_t hProgram ///< [in] handle for the Program to retain
) {
    auto pfnRetain = context.urDdiTable.Program.pfnRetain;

    if (nullptr == pfnRetain) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hProgram) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnRetain(hProgram);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.incrementRefCount(hProgram);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urProgramRelease
__urdlllocal ur_result_t UR_APICALL urProgramRelease(
    ur_program_handle_t hProgram ///< [in] handle for the Program to release
) {
    auto pfnRelease = context.urDdiTable.Program.pfnRelease;

    if (nullptr == pfnRelease) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hProgram) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnRelease(hProgram);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.decrementRefCount(hProgram);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urProgramGetFunctionPointer
__urdlllocal ur_result_t UR_APICALL urProgramGetFunctionPointer(
    ur_device_handle_t
        hDevice, ///< [in] handle of the device to retrieve pointer for.
    ur_program_handle_t
        hProgram, ///< [in] handle of the program to search for function in.
    ///< The program must already be built to the specified device, or
    ///< otherwise ::UR_RESULT_ERROR_INVALID_PROGRAM_EXECUTABLE is returned.
    const char *
        pFunctionName, ///< [in] A null-terminates string denoting the mangled function name.
    void **
        ppFunctionPointer ///< [out] Returns the pointer to the function if it is found in the program.
) {
    auto pfnGetFunctionPointer =
        context.urDdiTable.Program.pfnGetFunctionPointer;

    if (nullptr == pfnGetFunctionPointer) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hDevice) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hProgram) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pFunctionName) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == ppFunctionPointer) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnGetFunctionPointer(hDevice, hProgram, pFunctionName,
                                               ppFunctionPointer);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urProgramGetInfo
__urdlllocal ur_result_t UR_APICALL urProgramGetInfo(
    ur_program_handle_t hProgram, ///< [in] handle of the Program object
    ur_program_info_t propName, ///< [in] name of the Program property to query
    size_t propSize,            ///< [in] the size of the Program property.
    void *
        pPropValue, ///< [in,out][optional][typename(propName, propSize)] array of bytes of
                    ///< holding the program info property.
    ///< If propSize is not equal to or greater than the real number of bytes
    ///< needed to return
    ///< the info then the ::UR_RESULT_ERROR_INVALID_SIZE error is returned and
    ///< pPropValue is not used.
    size_t *
        pPropSizeRet ///< [out][optional] pointer to the actual size in bytes of the queried propName.
) {
    auto pfnGetInfo = context.urDdiTable.Program.pfnGetInfo;

    if (nullptr == pfnGetInfo) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hProgram) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (UR_PROGRAM_INFO_KERNEL_NAMES < propName) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }
    }

    ur_result_t result =
        pfnGetInfo(hProgram, propName, propSize, pPropValue, pPropSizeRet);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urProgramGetBuildInfo
__urdlllocal ur_result_t UR_APICALL urProgramGetBuildInfo(
    ur_program_handle_t hProgram, ///< [in] handle of the Program object
    ur_device_handle_t hDevice,   ///< [in] handle of the Device object
    ur_program_build_info_t
        propName,    ///< [in] name of the Program build info to query
    size_t propSize, ///< [in] size of the Program build info property.
    void *
        pPropValue, ///< [in,out][optional][typename(propName, propSize)] value of the Program
                    ///< build property.
    ///< If propSize is not equal to or greater than the real number of bytes
    ///< needed to return the info then the ::UR_RESULT_ERROR_INVALID_SIZE
    ///< error is returned and pPropValue is not used.
    size_t *
        pPropSizeRet ///< [out][optional] pointer to the actual size in bytes of data being
                     ///< queried by propName.
) {
    auto pfnGetBuildInfo = context.urDdiTable.Program.pfnGetBuildInfo;

    if (nullptr == pfnGetBuildInfo) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hProgram) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hDevice) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (UR_PROGRAM_BUILD_INFO_BINARY_TYPE < propName) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }
    }

    ur_result_t result = pfnGetBuildInfo(hProgram, hDevice, propName, propSize,
                                         pPropValue, pPropSizeRet);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urProgramSetSpecializationConstants
__urdlllocal ur_result_t UR_APICALL urProgramSetSpecializationConstants(
    ur_program_handle_t hProgram, ///< [in] handle of the Program object
    uint32_t count, ///< [in] the number of elements in the pSpecConstants array
    const ur_specialization_constant_info_t *
        pSpecConstants ///< [in][range(0, count)] array of specialization constant value
                       ///< descriptions
) {
    auto pfnSetSpecializationConstants =
        context.urDdiTable.Program.pfnSetSpecializationConstants;

    if (nullptr == pfnSetSpecializationConstants) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hProgram) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pSpecConstants) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (count == 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }
    }

    ur_result_t result =
        pfnSetSpecializationConstants(hProgram, count, pSpecConstants);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urProgramGetNativeHandle
__urdlllocal ur_result_t UR_APICALL urProgramGetNativeHandle(
    ur_program_handle_t hProgram, ///< [in] handle of the program.
    ur_native_handle_t *
        phNativeProgram ///< [out] a pointer to the native handle of the program.
) {
    auto pfnGetNativeHandle = context.urDdiTable.Program.pfnGetNativeHandle;

    if (nullptr == pfnGetNativeHandle) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hProgram) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == phNativeProgram) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnGetNativeHandle(hProgram, phNativeProgram);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urProgramCreateWithNativeHandle
__urdlllocal ur_result_t UR_APICALL urProgramCreateWithNativeHandle(
    ur_native_handle_t
        hNativeProgram,           ///< [in] the native handle of the program.
    ur_context_handle_t hContext, ///< [in] handle of the context instance
    const ur_program_native_properties_t *
        pProperties, ///< [in][optional] pointer to native program properties struct.
    ur_program_handle_t *
        phProgram ///< [out] pointer to the handle of the program object created.
) {
    auto pfnCreateWithNativeHandle =
        context.urDdiTable.Program.pfnCreateWithNativeHandle;

    if (nullptr == pfnCreateWithNativeHandle) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hNativeProgram) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == phProgram) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnCreateWithNativeHandle(hNativeProgram, hContext,
                                                   pProperties, phProgram);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.createRefCount(*phProgram);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urKernelCreate
__urdlllocal ur_result_t UR_APICALL urKernelCreate(
    ur_program_handle_t hProgram, ///< [in] handle of the program instance
    const char *pKernelName,      ///< [in] pointer to null-terminated string.
    ur_kernel_handle_t
        *phKernel ///< [out] pointer to handle of kernel object created.
) {
    auto pfnCreate = context.urDdiTable.Kernel.pfnCreate;

    if (nullptr == pfnCreate) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hProgram) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pKernelName) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == phKernel) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnCreate(hProgram, pKernelName, phKernel);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.createRefCount(*phKernel);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urKernelSetArgValue
__urdlllocal ur_result_t UR_APICALL urKernelSetArgValue(
    ur_kernel_handle_t hKernel, ///< [in] handle of the kernel object
    uint32_t argIndex, ///< [in] argument index in range [0, num args - 1]
    size_t argSize,    ///< [in] size of argument type
    const void
        *pArgValue ///< [in] argument value represented as matching arg type.
) {
    auto pfnSetArgValue = context.urDdiTable.Kernel.pfnSetArgValue;

    if (nullptr == pfnSetArgValue) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hKernel) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pArgValue) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnSetArgValue(hKernel, argIndex, argSize, pArgValue);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urKernelSetArgLocal
__urdlllocal ur_result_t UR_APICALL urKernelSetArgLocal(
    ur_kernel_handle_t hKernel, ///< [in] handle of the kernel object
    uint32_t argIndex, ///< [in] argument index in range [0, num args - 1]
    size_t
        argSize ///< [in] size of the local buffer to be allocated by the runtime
) {
    auto pfnSetArgLocal = context.urDdiTable.Kernel.pfnSetArgLocal;

    if (nullptr == pfnSetArgLocal) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hKernel) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnSetArgLocal(hKernel, argIndex, argSize);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urKernelGetInfo
__urdlllocal ur_result_t UR_APICALL urKernelGetInfo(
    ur_kernel_handle_t hKernel, ///< [in] handle of the Kernel object
    ur_kernel_info_t propName,  ///< [in] name of the Kernel property to query
    size_t propSize,            ///< [in] the size of the Kernel property value.
    void *
        pPropValue, ///< [in,out][optional][typename(propName, propSize)] array of bytes
                    ///< holding the kernel info property.
    ///< If propSize is not equal to or greater than the real number of bytes
    ///< needed to return
    ///< the info then the ::UR_RESULT_ERROR_INVALID_SIZE error is returned and
    ///< pPropValue is not used.
    size_t *
        pPropSizeRet ///< [out][optional] pointer to the actual size in bytes of data being
                     ///< queried by propName.
) {
    auto pfnGetInfo = context.urDdiTable.Kernel.pfnGetInfo;

    if (nullptr == pfnGetInfo) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hKernel) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (UR_KERNEL_INFO_NUM_REGS < propName) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }
    }

    ur_result_t result =
        pfnGetInfo(hKernel, propName, propSize, pPropValue, pPropSizeRet);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urKernelGetGroupInfo
__urdlllocal ur_result_t UR_APICALL urKernelGetGroupInfo(
    ur_kernel_handle_t hKernel, ///< [in] handle of the Kernel object
    ur_device_handle_t hDevice, ///< [in] handle of the Device object
    ur_kernel_group_info_t
        propName,    ///< [in] name of the work Group property to query
    size_t propSize, ///< [in] size of the Kernel Work Group property value
    void *
        pPropValue, ///< [in,out][optional][typename(propName, propSize)] value of the Kernel
                    ///< Work Group property.
    size_t *
        pPropSizeRet ///< [out][optional] pointer to the actual size in bytes of data being
                     ///< queried by propName.
) {
    auto pfnGetGroupInfo = context.urDdiTable.Kernel.pfnGetGroupInfo;

    if (nullptr == pfnGetGroupInfo) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hKernel) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hDevice) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (UR_KERNEL_GROUP_INFO_PRIVATE_MEM_SIZE < propName) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }
    }

    ur_result_t result = pfnGetGroupInfo(hKernel, hDevice, propName, propSize,
                                         pPropValue, pPropSizeRet);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urKernelGetSubGroupInfo
__urdlllocal ur_result_t UR_APICALL urKernelGetSubGroupInfo(
    ur_kernel_handle_t hKernel, ///< [in] handle of the Kernel object
    ur_device_handle_t hDevice, ///< [in] handle of the Device object
    ur_kernel_sub_group_info_t
        propName,    ///< [in] name of the SubGroup property to query
    size_t propSize, ///< [in] size of the Kernel SubGroup property value
    void *
        pPropValue, ///< [in,out][optional][typename(propName, propSize)] value of the Kernel
                    ///< SubGroup property.
    size_t *
        pPropSizeRet ///< [out][optional] pointer to the actual size in bytes of data being
                     ///< queried by propName.
) {
    auto pfnGetSubGroupInfo = context.urDdiTable.Kernel.pfnGetSubGroupInfo;

    if (nullptr == pfnGetSubGroupInfo) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hKernel) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hDevice) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (UR_KERNEL_SUB_GROUP_INFO_SUB_GROUP_SIZE_INTEL < propName) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }
    }

    ur_result_t result = pfnGetSubGroupInfo(hKernel, hDevice, propName,
                                            propSize, pPropValue, pPropSizeRet);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urKernelRetain
__urdlllocal ur_result_t UR_APICALL urKernelRetain(
    ur_kernel_handle_t hKernel ///< [in] handle for the Kernel to retain
) {
    auto pfnRetain = context.urDdiTable.Kernel.pfnRetain;

    if (nullptr == pfnRetain) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hKernel) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnRetain(hKernel);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.incrementRefCount(hKernel);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urKernelRelease
__urdlllocal ur_result_t UR_APICALL urKernelRelease(
    ur_kernel_handle_t hKernel ///< [in] handle for the Kernel to release
) {
    auto pfnRelease = context.urDdiTable.Kernel.pfnRelease;

    if (nullptr == pfnRelease) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hKernel) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnRelease(hKernel);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.decrementRefCount(hKernel);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urKernelSetArgPointer
__urdlllocal ur_result_t UR_APICALL urKernelSetArgPointer(
    ur_kernel_handle_t hKernel, ///< [in] handle of the kernel object
    uint32_t argIndex, ///< [in] argument index in range [0, num args - 1]
    const void *
        pArgValue ///< [in][optional] SVM pointer to memory location holding the argument
                  ///< value. If null then argument value is considered null.
) {
    auto pfnSetArgPointer = context.urDdiTable.Kernel.pfnSetArgPointer;

    if (nullptr == pfnSetArgPointer) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hKernel) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnSetArgPointer(hKernel, argIndex, pArgValue);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urKernelSetExecInfo
__urdlllocal ur_result_t UR_APICALL urKernelSetExecInfo(
    ur_kernel_handle_t hKernel,     ///< [in] handle of the kernel object
    ur_kernel_exec_info_t propName, ///< [in] name of the execution attribute
    size_t propSize,                ///< [in] size in byte the attribute value
    const void *
        pPropValue ///< [in][typename(propName, propSize)] pointer to memory location holding
                   ///< the property value.
) {
    auto pfnSetExecInfo = context.urDdiTable.Kernel.pfnSetExecInfo;

    if (nullptr == pfnSetExecInfo) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hKernel) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pPropValue) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (UR_KERNEL_EXEC_INFO_CACHE_CONFIG < propName) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }
    }

    ur_result_t result =
        pfnSetExecInfo(hKernel, propName, propSize, pPropValue);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urKernelSetArgSampler
__urdlllocal ur_result_t UR_APICALL urKernelSetArgSampler(
    ur_kernel_handle_t hKernel, ///< [in] handle of the kernel object
    uint32_t argIndex, ///< [in] argument index in range [0, num args - 1]
    ur_sampler_handle_t hArgValue ///< [in] handle of Sampler object.
) {
    auto pfnSetArgSampler = context.urDdiTable.Kernel.pfnSetArgSampler;

    if (nullptr == pfnSetArgSampler) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hKernel) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hArgValue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnSetArgSampler(hKernel, argIndex, hArgValue);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urKernelSetArgMemObj
__urdlllocal ur_result_t UR_APICALL urKernelSetArgMemObj(
    ur_kernel_handle_t hKernel, ///< [in] handle of the kernel object
    uint32_t argIndex, ///< [in] argument index in range [0, num args - 1]
    ur_mem_handle_t hArgValue ///< [in] handle of Memory object.
) {
    auto pfnSetArgMemObj = context.urDdiTable.Kernel.pfnSetArgMemObj;

    if (nullptr == pfnSetArgMemObj) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hKernel) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hArgValue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnSetArgMemObj(hKernel, argIndex, hArgValue);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urKernelSetSpecializationConstants
__urdlllocal ur_result_t UR_APICALL urKernelSetSpecializationConstants(
    ur_kernel_handle_t hKernel, ///< [in] handle of the kernel object
    uint32_t count, ///< [in] the number of elements in the pSpecConstants array
    const ur_specialization_constant_info_t *
        pSpecConstants ///< [in] array of specialization constant value descriptions
) {
    auto pfnSetSpecializationConstants =
        context.urDdiTable.Kernel.pfnSetSpecializationConstants;

    if (nullptr == pfnSetSpecializationConstants) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hKernel) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pSpecConstants) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (count == 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }
    }

    ur_result_t result =
        pfnSetSpecializationConstants(hKernel, count, pSpecConstants);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urKernelGetNativeHandle
__urdlllocal ur_result_t UR_APICALL urKernelGetNativeHandle(
    ur_kernel_handle_t hKernel, ///< [in] handle of the kernel.
    ur_native_handle_t
        *phNativeKernel ///< [out] a pointer to the native handle of the kernel.
) {
    auto pfnGetNativeHandle = context.urDdiTable.Kernel.pfnGetNativeHandle;

    if (nullptr == pfnGetNativeHandle) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hKernel) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == phNativeKernel) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnGetNativeHandle(hKernel, phNativeKernel);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urKernelCreateWithNativeHandle
__urdlllocal ur_result_t UR_APICALL urKernelCreateWithNativeHandle(
    ur_native_handle_t hNativeKernel, ///< [in] the native handle of the kernel.
    ur_context_handle_t hContext,     ///< [in] handle of the context object
    ur_program_handle_t
        hProgram, ///< [in] handle of the program associated with the kernel
    const ur_kernel_native_properties_t *
        pProperties, ///< [in][optional] pointer to native kernel properties struct
    ur_kernel_handle_t
        *phKernel ///< [out] pointer to the handle of the kernel object created.
) {
    auto pfnCreateWithNativeHandle =
        context.urDdiTable.Kernel.pfnCreateWithNativeHandle;

    if (nullptr == pfnCreateWithNativeHandle) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hNativeKernel) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hProgram) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == phKernel) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnCreateWithNativeHandle(
        hNativeKernel, hContext, hProgram, pProperties, phKernel);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.createRefCount(*phKernel);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urQueueGetInfo
__urdlllocal ur_result_t UR_APICALL urQueueGetInfo(
    ur_queue_handle_t hQueue, ///< [in] handle of the queue object
    ur_queue_info_t propName, ///< [in] name of the queue property to query
    size_t
        propSize, ///< [in] size in bytes of the queue property value provided
    void *
        pPropValue, ///< [out][optional][typename(propName, propSize)] value of the queue
                    ///< property
    size_t *
        pPropSizeRet ///< [out][optional] size in bytes returned in queue property value
) {
    auto pfnGetInfo = context.urDdiTable.Queue.pfnGetInfo;

    if (nullptr == pfnGetInfo) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (UR_QUEUE_INFO_EMPTY < propName) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }
    }

    ur_result_t result =
        pfnGetInfo(hQueue, propName, propSize, pPropValue, pPropSizeRet);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urQueueCreate
__urdlllocal ur_result_t UR_APICALL urQueueCreate(
    ur_context_handle_t hContext, ///< [in] handle of the context object
    ur_device_handle_t hDevice,   ///< [in] handle of the device object
    const ur_queue_properties_t
        *pProperties, ///< [in][optional] pointer to queue creation properties.
    ur_queue_handle_t
        *phQueue ///< [out] pointer to handle of queue object created
) {
    auto pfnCreate = context.urDdiTable.Queue.pfnCreate;

    if (nullptr == pfnCreate) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hDevice) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == phQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnCreate(hContext, hDevice, pProperties, phQueue);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.createRefCount(*phQueue);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urQueueRetain
__urdlllocal ur_result_t UR_APICALL urQueueRetain(
    ur_queue_handle_t hQueue ///< [in] handle of the queue object to get access
) {
    auto pfnRetain = context.urDdiTable.Queue.pfnRetain;

    if (nullptr == pfnRetain) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnRetain(hQueue);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.incrementRefCount(hQueue);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urQueueRelease
__urdlllocal ur_result_t UR_APICALL urQueueRelease(
    ur_queue_handle_t hQueue ///< [in] handle of the queue object to release
) {
    auto pfnRelease = context.urDdiTable.Queue.pfnRelease;

    if (nullptr == pfnRelease) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnRelease(hQueue);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.decrementRefCount(hQueue);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urQueueGetNativeHandle
__urdlllocal ur_result_t UR_APICALL urQueueGetNativeHandle(
    ur_queue_handle_t hQueue, ///< [in] handle of the queue.
    ur_native_handle_t
        *phNativeQueue ///< [out] a pointer to the native handle of the queue.
) {
    auto pfnGetNativeHandle = context.urDdiTable.Queue.pfnGetNativeHandle;

    if (nullptr == pfnGetNativeHandle) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == phNativeQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnGetNativeHandle(hQueue, phNativeQueue);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urQueueCreateWithNativeHandle
__urdlllocal ur_result_t UR_APICALL urQueueCreateWithNativeHandle(
    ur_native_handle_t hNativeQueue, ///< [in] the native handle of the queue.
    ur_context_handle_t hContext,    ///< [in] handle of the context object
    ur_device_handle_t hDevice,      ///< [in] handle of the device object
    const ur_queue_native_properties_t *
        pProperties, ///< [in][optional] pointer to native queue properties struct
    ur_queue_handle_t
        *phQueue ///< [out] pointer to the handle of the queue object created.
) {
    auto pfnCreateWithNativeHandle =
        context.urDdiTable.Queue.pfnCreateWithNativeHandle;

    if (nullptr == pfnCreateWithNativeHandle) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hNativeQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hDevice) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == phQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnCreateWithNativeHandle(
        hNativeQueue, hContext, hDevice, pProperties, phQueue);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.createRefCount(*phQueue);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urQueueFinish
__urdlllocal ur_result_t UR_APICALL urQueueFinish(
    ur_queue_handle_t hQueue ///< [in] handle of the queue to be finished.
) {
    auto pfnFinish = context.urDdiTable.Queue.pfnFinish;

    if (nullptr == pfnFinish) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnFinish(hQueue);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urQueueFlush
__urdlllocal ur_result_t UR_APICALL urQueueFlush(
    ur_queue_handle_t hQueue ///< [in] handle of the queue to be flushed.
) {
    auto pfnFlush = context.urDdiTable.Queue.pfnFlush;

    if (nullptr == pfnFlush) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnFlush(hQueue);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEventGetInfo
__urdlllocal ur_result_t UR_APICALL urEventGetInfo(
    ur_event_handle_t hEvent, ///< [in] handle of the event object
    ur_event_info_t propName, ///< [in] the name of the event property to query
    size_t propSize, ///< [in] size in bytes of the event property value
    void *
        pPropValue, ///< [out][optional][typename(propName, propSize)] value of the event
                    ///< property
    size_t *pPropSizeRet ///< [out][optional] bytes returned in event property
) {
    auto pfnGetInfo = context.urDdiTable.Event.pfnGetInfo;

    if (nullptr == pfnGetInfo) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hEvent) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (UR_EVENT_INFO_REFERENCE_COUNT < propName) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }

        if (pPropValue && propSize == 0) {
            return UR_RESULT_ERROR_INVALID_VALUE;
        }
    }

    ur_result_t result =
        pfnGetInfo(hEvent, propName, propSize, pPropValue, pPropSizeRet);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEventGetProfilingInfo
__urdlllocal ur_result_t UR_APICALL urEventGetProfilingInfo(
    ur_event_handle_t hEvent, ///< [in] handle of the event object
    ur_profiling_info_t
        propName,    ///< [in] the name of the profiling property to query
    size_t propSize, ///< [in] size in bytes of the profiling property value
    void *
        pPropValue, ///< [out][optional][typename(propName, propSize)] value of the profiling
                    ///< property
    size_t *
        pPropSizeRet ///< [out][optional] pointer to the actual size in bytes returned in
                     ///< propValue
) {
    auto pfnGetProfilingInfo = context.urDdiTable.Event.pfnGetProfilingInfo;

    if (nullptr == pfnGetProfilingInfo) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hEvent) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (UR_PROFILING_INFO_COMMAND_END < propName) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }

        if (pPropValue && propSize == 0) {
            return UR_RESULT_ERROR_INVALID_VALUE;
        }
    }

    ur_result_t result = pfnGetProfilingInfo(hEvent, propName, propSize,
                                             pPropValue, pPropSizeRet);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEventWait
__urdlllocal ur_result_t UR_APICALL urEventWait(
    uint32_t numEvents, ///< [in] number of events in the event list
    const ur_event_handle_t *
        phEventWaitList ///< [in][range(0, numEvents)] pointer to a list of events to wait for
                        ///< completion
) {
    auto pfnWait = context.urDdiTable.Event.pfnWait;

    if (nullptr == pfnWait) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == phEventWaitList) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (numEvents == 0) {
            return UR_RESULT_ERROR_INVALID_VALUE;
        }
    }

    ur_result_t result = pfnWait(numEvents, phEventWaitList);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEventRetain
__urdlllocal ur_result_t UR_APICALL urEventRetain(
    ur_event_handle_t hEvent ///< [in] handle of the event object
) {
    auto pfnRetain = context.urDdiTable.Event.pfnRetain;

    if (nullptr == pfnRetain) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hEvent) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnRetain(hEvent);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.incrementRefCount(hEvent);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEventRelease
__urdlllocal ur_result_t UR_APICALL urEventRelease(
    ur_event_handle_t hEvent ///< [in] handle of the event object
) {
    auto pfnRelease = context.urDdiTable.Event.pfnRelease;

    if (nullptr == pfnRelease) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hEvent) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }
    }

    ur_result_t result = pfnRelease(hEvent);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.decrementRefCount(hEvent);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEventGetNativeHandle
__urdlllocal ur_result_t UR_APICALL urEventGetNativeHandle(
    ur_event_handle_t hEvent, ///< [in] handle of the event.
    ur_native_handle_t
        *phNativeEvent ///< [out] a pointer to the native handle of the event.
) {
    auto pfnGetNativeHandle = context.urDdiTable.Event.pfnGetNativeHandle;

    if (nullptr == pfnGetNativeHandle) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hEvent) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == phNativeEvent) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result = pfnGetNativeHandle(hEvent, phNativeEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEventCreateWithNativeHandle
__urdlllocal ur_result_t UR_APICALL urEventCreateWithNativeHandle(
    ur_native_handle_t hNativeEvent, ///< [in] the native handle of the event.
    ur_context_handle_t hContext,    ///< [in] handle of the context object
    const ur_event_native_properties_t *
        pProperties, ///< [in][optional] pointer to native event properties struct
    ur_event_handle_t
        *phEvent ///< [out] pointer to the handle of the event object created.
) {
    auto pfnCreateWithNativeHandle =
        context.urDdiTable.Event.pfnCreateWithNativeHandle;

    if (nullptr == pfnCreateWithNativeHandle) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hNativeEvent) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hContext) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == phEvent) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }
    }

    ur_result_t result =
        pfnCreateWithNativeHandle(hNativeEvent, hContext, pProperties, phEvent);

    if (context.enableLeakChecking && result == UR_RESULT_SUCCESS) {
        refCountContext.createRefCount(*phEvent);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEventSetCallback
__urdlllocal ur_result_t UR_APICALL urEventSetCallback(
    ur_event_handle_t hEvent,       ///< [in] handle of the event object
    ur_execution_info_t execStatus, ///< [in] execution status of the event
    ur_event_callback_t pfnNotify,  ///< [in] execution status of the event
    void *
        pUserData ///< [in][out][optional] pointer to data to be passed to callback.
) {
    auto pfnSetCallback = context.urDdiTable.Event.pfnSetCallback;

    if (nullptr == pfnSetCallback) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hEvent) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pfnNotify) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (UR_EXECUTION_INFO_EXECUTION_INFO_QUEUED < execStatus) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }
    }

    ur_result_t result =
        pfnSetCallback(hEvent, execStatus, pfnNotify, pUserData);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueKernelLaunch
__urdlllocal ur_result_t UR_APICALL urEnqueueKernelLaunch(
    ur_queue_handle_t hQueue,   ///< [in] handle of the queue object
    ur_kernel_handle_t hKernel, ///< [in] handle of the kernel object
    uint32_t
        workDim, ///< [in] number of dimensions, from 1 to 3, to specify the global and
                 ///< work-group work-items
    const size_t *
        pGlobalWorkOffset, ///< [in] pointer to an array of workDim unsigned values that specify the
    ///< offset used to calculate the global ID of a work-item
    const size_t *
        pGlobalWorkSize, ///< [in] pointer to an array of workDim unsigned values that specify the
    ///< number of global work-items in workDim that will execute the kernel
    ///< function
    const size_t *
        pLocalWorkSize, ///< [in][optional] pointer to an array of workDim unsigned values that
    ///< specify the number of local work-items forming a work-group that will
    ///< execute the kernel function.
    ///< If nullptr, the runtime implementation will choose the work-group
    ///< size.
    uint32_t numEventsInWaitList, ///< [in] size of the event wait list
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before the kernel execution.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that no wait
    ///< event.
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< kernel execution instance.
) {
    auto pfnKernelLaunch = context.urDdiTable.Enqueue.pfnKernelLaunch;

    if (nullptr == pfnKernelLaunch) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hKernel) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pGlobalWorkOffset) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == pGlobalWorkSize) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }
    }

    ur_result_t result = pfnKernelLaunch(
        hQueue, hKernel, workDim, pGlobalWorkOffset, pGlobalWorkSize,
        pLocalWorkSize, numEventsInWaitList, phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueEventsWait
__urdlllocal ur_result_t UR_APICALL urEnqueueEventsWait(
    ur_queue_handle_t hQueue,     ///< [in] handle of the queue object
    uint32_t numEventsInWaitList, ///< [in] size of the event wait list
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before this command can be executed.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that all
    ///< previously enqueued commands
    ///< must be complete.
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< command instance.
) {
    auto pfnEventsWait = context.urDdiTable.Enqueue.pfnEventsWait;

    if (nullptr == pfnEventsWait) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }
    }

    ur_result_t result =
        pfnEventsWait(hQueue, numEventsInWaitList, phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueEventsWaitWithBarrier
__urdlllocal ur_result_t UR_APICALL urEnqueueEventsWaitWithBarrier(
    ur_queue_handle_t hQueue,     ///< [in] handle of the queue object
    uint32_t numEventsInWaitList, ///< [in] size of the event wait list
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before this command can be executed.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that all
    ///< previously enqueued commands
    ///< must be complete.
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< command instance.
) {
    auto pfnEventsWaitWithBarrier =
        context.urDdiTable.Enqueue.pfnEventsWaitWithBarrier;

    if (nullptr == pfnEventsWaitWithBarrier) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }
    }

    ur_result_t result = pfnEventsWaitWithBarrier(hQueue, numEventsInWaitList,
                                                  phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueMemBufferRead
__urdlllocal ur_result_t UR_APICALL urEnqueueMemBufferRead(
    ur_queue_handle_t hQueue, ///< [in] handle of the queue object
    ur_mem_handle_t hBuffer,  ///< [in] handle of the buffer object
    bool blockingRead, ///< [in] indicates blocking (true), non-blocking (false)
    size_t offset,     ///< [in] offset in bytes in the buffer object
    size_t size,       ///< [in] size in bytes of data being read
    void *pDst, ///< [in] pointer to host memory where data is to be read into
    uint32_t numEventsInWaitList, ///< [in] size of the event wait list
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before this command can be executed.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that this
    ///< command does not wait on any event to complete.
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< command instance.
) {
    auto pfnMemBufferRead = context.urDdiTable.Enqueue.pfnMemBufferRead;

    if (nullptr == pfnMemBufferRead) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hBuffer) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pDst) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }
    }

    ur_result_t result =
        pfnMemBufferRead(hQueue, hBuffer, blockingRead, offset, size, pDst,
                         numEventsInWaitList, phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueMemBufferWrite
__urdlllocal ur_result_t UR_APICALL urEnqueueMemBufferWrite(
    ur_queue_handle_t hQueue, ///< [in] handle of the queue object
    ur_mem_handle_t hBuffer,  ///< [in] handle of the buffer object
    bool
        blockingWrite, ///< [in] indicates blocking (true), non-blocking (false)
    size_t offset,     ///< [in] offset in bytes in the buffer object
    size_t size,       ///< [in] size in bytes of data being written
    const void
        *pSrc, ///< [in] pointer to host memory where data is to be written from
    uint32_t numEventsInWaitList, ///< [in] size of the event wait list
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before this command can be executed.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that this
    ///< command does not wait on any event to complete.
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< command instance.
) {
    auto pfnMemBufferWrite = context.urDdiTable.Enqueue.pfnMemBufferWrite;

    if (nullptr == pfnMemBufferWrite) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hBuffer) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pSrc) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }
    }

    ur_result_t result =
        pfnMemBufferWrite(hQueue, hBuffer, blockingWrite, offset, size, pSrc,
                          numEventsInWaitList, phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueMemBufferReadRect
__urdlllocal ur_result_t UR_APICALL urEnqueueMemBufferReadRect(
    ur_queue_handle_t hQueue, ///< [in] handle of the queue object
    ur_mem_handle_t hBuffer,  ///< [in] handle of the buffer object
    bool blockingRead, ///< [in] indicates blocking (true), non-blocking (false)
    ur_rect_offset_t bufferOrigin, ///< [in] 3D offset in the buffer
    ur_rect_offset_t hostOrigin,   ///< [in] 3D offset in the host region
    ur_rect_region_t
        region, ///< [in] 3D rectangular region descriptor: width, height, depth
    size_t
        bufferRowPitch, ///< [in] length of each row in bytes in the buffer object
    size_t
        bufferSlicePitch, ///< [in] length of each 2D slice in bytes in the buffer object being read
    size_t
        hostRowPitch, ///< [in] length of each row in bytes in the host memory region pointed by
                      ///< dst
    size_t
        hostSlicePitch, ///< [in] length of each 2D slice in bytes in the host memory region
                        ///< pointed by dst
    void *pDst, ///< [in] pointer to host memory where data is to be read into
    uint32_t numEventsInWaitList, ///< [in] size of the event wait list
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before this command can be executed.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that this
    ///< command does not wait on any event to complete.
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< command instance.
) {
    auto pfnMemBufferReadRect = context.urDdiTable.Enqueue.pfnMemBufferReadRect;

    if (nullptr == pfnMemBufferReadRect) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hBuffer) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pDst) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (region.width == 0 || region.height == 0 || region.width == 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (bufferRowPitch != 0 && bufferRowPitch < region.width) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (hostRowPitch != 0 && hostRowPitch < region.width) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (bufferSlicePitch != 0 &&
            bufferSlicePitch < region.height * bufferRowPitch) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (bufferSlicePitch != 0 && bufferSlicePitch % bufferRowPitch != 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (hostSlicePitch != 0 &&
            hostSlicePitch < region.height * hostRowPitch) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (hostSlicePitch != 0 && hostSlicePitch % hostRowPitch != 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }
    }

    ur_result_t result = pfnMemBufferReadRect(
        hQueue, hBuffer, blockingRead, bufferOrigin, hostOrigin, region,
        bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, pDst,
        numEventsInWaitList, phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueMemBufferWriteRect
__urdlllocal ur_result_t UR_APICALL urEnqueueMemBufferWriteRect(
    ur_queue_handle_t hQueue, ///< [in] handle of the queue object
    ur_mem_handle_t hBuffer,  ///< [in] handle of the buffer object
    bool
        blockingWrite, ///< [in] indicates blocking (true), non-blocking (false)
    ur_rect_offset_t bufferOrigin, ///< [in] 3D offset in the buffer
    ur_rect_offset_t hostOrigin,   ///< [in] 3D offset in the host region
    ur_rect_region_t
        region, ///< [in] 3D rectangular region descriptor: width, height, depth
    size_t
        bufferRowPitch, ///< [in] length of each row in bytes in the buffer object
    size_t
        bufferSlicePitch, ///< [in] length of each 2D slice in bytes in the buffer object being
                          ///< written
    size_t
        hostRowPitch, ///< [in] length of each row in bytes in the host memory region pointed by
                      ///< src
    size_t
        hostSlicePitch, ///< [in] length of each 2D slice in bytes in the host memory region
                        ///< pointed by src
    void
        *pSrc, ///< [in] pointer to host memory where data is to be written from
    uint32_t numEventsInWaitList, ///< [in] size of the event wait list
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] points to a list of
    ///< events that must be complete before this command can be executed.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that this
    ///< command does not wait on any event to complete.
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< command instance.
) {
    auto pfnMemBufferWriteRect =
        context.urDdiTable.Enqueue.pfnMemBufferWriteRect;

    if (nullptr == pfnMemBufferWriteRect) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hBuffer) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pSrc) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (region.width == 0 || region.height == 0 || region.width == 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (bufferRowPitch != 0 && bufferRowPitch < region.width) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (hostRowPitch != 0 && hostRowPitch < region.width) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (bufferSlicePitch != 0 &&
            bufferSlicePitch < region.height * bufferRowPitch) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (bufferSlicePitch != 0 && bufferSlicePitch % bufferRowPitch != 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (hostSlicePitch != 0 &&
            hostSlicePitch < region.height * hostRowPitch) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (hostSlicePitch != 0 && hostSlicePitch % hostRowPitch != 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }
    }

    ur_result_t result = pfnMemBufferWriteRect(
        hQueue, hBuffer, blockingWrite, bufferOrigin, hostOrigin, region,
        bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, pSrc,
        numEventsInWaitList, phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueMemBufferCopy
__urdlllocal ur_result_t UR_APICALL urEnqueueMemBufferCopy(
    ur_queue_handle_t hQueue,   ///< [in] handle of the queue object
    ur_mem_handle_t hBufferSrc, ///< [in] handle of the src buffer object
    ur_mem_handle_t hBufferDst, ///< [in] handle of the dest buffer object
    size_t srcOffset, ///< [in] offset into hBufferSrc to begin copying from
    size_t dstOffset, ///< [in] offset info hBufferDst to begin copying into
    size_t size,      ///< [in] size in bytes of data being copied
    uint32_t numEventsInWaitList, ///< [in] size of the event wait list
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before this command can be executed.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that this
    ///< command does not wait on any event to complete.
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< command instance.
) {
    auto pfnMemBufferCopy = context.urDdiTable.Enqueue.pfnMemBufferCopy;

    if (nullptr == pfnMemBufferCopy) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hBufferSrc) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hBufferDst) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }
    }

    ur_result_t result =
        pfnMemBufferCopy(hQueue, hBufferSrc, hBufferDst, srcOffset, dstOffset,
                         size, numEventsInWaitList, phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueMemBufferCopyRect
__urdlllocal ur_result_t UR_APICALL urEnqueueMemBufferCopyRect(
    ur_queue_handle_t hQueue,   ///< [in] handle of the queue object
    ur_mem_handle_t hBufferSrc, ///< [in] handle of the source buffer object
    ur_mem_handle_t hBufferDst, ///< [in] handle of the dest buffer object
    ur_rect_offset_t srcOrigin, ///< [in] 3D offset in the source buffer
    ur_rect_offset_t dstOrigin, ///< [in] 3D offset in the destination buffer
    ur_rect_region_t
        region, ///< [in] source 3D rectangular region descriptor: width, height, depth
    size_t
        srcRowPitch, ///< [in] length of each row in bytes in the source buffer object
    size_t
        srcSlicePitch, ///< [in] length of each 2D slice in bytes in the source buffer object
    size_t
        dstRowPitch, ///< [in] length of each row in bytes in the destination buffer object
    size_t
        dstSlicePitch, ///< [in] length of each 2D slice in bytes in the destination buffer object
    uint32_t numEventsInWaitList, ///< [in] size of the event wait list
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before this command can be executed.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that this
    ///< command does not wait on any event to complete.
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< command instance.
) {
    auto pfnMemBufferCopyRect = context.urDdiTable.Enqueue.pfnMemBufferCopyRect;

    if (nullptr == pfnMemBufferCopyRect) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hBufferSrc) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hBufferDst) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (region.width == 0 || region.height == 0 || region.depth == 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (srcRowPitch != 0 && srcRowPitch < region.height) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (dstRowPitch != 0 && dstRowPitch < region.height) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (srcSlicePitch != 0 && srcSlicePitch < region.height * srcRowPitch) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (srcSlicePitch != 0 && srcSlicePitch % srcRowPitch != 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (dstSlicePitch != 0 && dstSlicePitch < region.height * dstRowPitch) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (dstSlicePitch != 0 && dstSlicePitch % dstRowPitch != 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }
    }

    ur_result_t result = pfnMemBufferCopyRect(
        hQueue, hBufferSrc, hBufferDst, srcOrigin, dstOrigin, region,
        srcRowPitch, srcSlicePitch, dstRowPitch, dstSlicePitch,
        numEventsInWaitList, phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueMemBufferFill
__urdlllocal ur_result_t UR_APICALL urEnqueueMemBufferFill(
    ur_queue_handle_t hQueue, ///< [in] handle of the queue object
    ur_mem_handle_t hBuffer,  ///< [in] handle of the buffer object
    const void *pPattern,     ///< [in] pointer to the fill pattern
    size_t patternSize,       ///< [in] size in bytes of the pattern
    size_t offset,            ///< [in] offset into the buffer
    size_t size, ///< [in] fill size in bytes, must be a multiple of patternSize
    uint32_t numEventsInWaitList, ///< [in] size of the event wait list
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before this command can be executed.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that this
    ///< command does not wait on any event to complete.
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< command instance.
) {
    auto pfnMemBufferFill = context.urDdiTable.Enqueue.pfnMemBufferFill;

    if (nullptr == pfnMemBufferFill) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hBuffer) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pPattern) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }
    }

    ur_result_t result =
        pfnMemBufferFill(hQueue, hBuffer, pPattern, patternSize, offset, size,
                         numEventsInWaitList, phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueMemImageRead
__urdlllocal ur_result_t UR_APICALL urEnqueueMemImageRead(
    ur_queue_handle_t hQueue, ///< [in] handle of the queue object
    ur_mem_handle_t hImage,   ///< [in] handle of the image object
    bool blockingRead, ///< [in] indicates blocking (true), non-blocking (false)
    ur_rect_offset_t
        origin, ///< [in] defines the (x,y,z) offset in pixels in the 1D, 2D, or 3D image
    ur_rect_region_t
        region, ///< [in] defines the (width, height, depth) in pixels of the 1D, 2D, or 3D
                ///< image
    size_t rowPitch,   ///< [in] length of each row in bytes
    size_t slicePitch, ///< [in] length of each 2D slice of the 3D image
    void *pDst, ///< [in] pointer to host memory where image is to be read into
    uint32_t numEventsInWaitList, ///< [in] size of the event wait list
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before this command can be executed.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that this
    ///< command does not wait on any event to complete.
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< command instance.
) {
    auto pfnMemImageRead = context.urDdiTable.Enqueue.pfnMemImageRead;

    if (nullptr == pfnMemImageRead) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hImage) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pDst) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }
    }

    ur_result_t result = pfnMemImageRead(
        hQueue, hImage, blockingRead, origin, region, rowPitch, slicePitch,
        pDst, numEventsInWaitList, phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueMemImageWrite
__urdlllocal ur_result_t UR_APICALL urEnqueueMemImageWrite(
    ur_queue_handle_t hQueue, ///< [in] handle of the queue object
    ur_mem_handle_t hImage,   ///< [in] handle of the image object
    bool
        blockingWrite, ///< [in] indicates blocking (true), non-blocking (false)
    ur_rect_offset_t
        origin, ///< [in] defines the (x,y,z) offset in pixels in the 1D, 2D, or 3D image
    ur_rect_region_t
        region, ///< [in] defines the (width, height, depth) in pixels of the 1D, 2D, or 3D
                ///< image
    size_t rowPitch,   ///< [in] length of each row in bytes
    size_t slicePitch, ///< [in] length of each 2D slice of the 3D image
    void *pSrc, ///< [in] pointer to host memory where image is to be read into
    uint32_t numEventsInWaitList, ///< [in] size of the event wait list
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before this command can be executed.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that this
    ///< command does not wait on any event to complete.
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< command instance.
) {
    auto pfnMemImageWrite = context.urDdiTable.Enqueue.pfnMemImageWrite;

    if (nullptr == pfnMemImageWrite) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hImage) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pSrc) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }
    }

    ur_result_t result = pfnMemImageWrite(
        hQueue, hImage, blockingWrite, origin, region, rowPitch, slicePitch,
        pSrc, numEventsInWaitList, phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueMemImageCopy
__urdlllocal ur_result_t UR_APICALL urEnqueueMemImageCopy(
    ur_queue_handle_t hQueue,  ///< [in] handle of the queue object
    ur_mem_handle_t hImageSrc, ///< [in] handle of the src image object
    ur_mem_handle_t hImageDst, ///< [in] handle of the dest image object
    ur_rect_offset_t
        srcOrigin, ///< [in] defines the (x,y,z) offset in pixels in the source 1D, 2D, or 3D
                   ///< image
    ur_rect_offset_t
        dstOrigin, ///< [in] defines the (x,y,z) offset in pixels in the destination 1D, 2D,
                   ///< or 3D image
    ur_rect_region_t
        region, ///< [in] defines the (width, height, depth) in pixels of the 1D, 2D, or 3D
                ///< image
    uint32_t numEventsInWaitList, ///< [in] size of the event wait list
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before this command can be executed.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that this
    ///< command does not wait on any event to complete.
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< command instance.
) {
    auto pfnMemImageCopy = context.urDdiTable.Enqueue.pfnMemImageCopy;

    if (nullptr == pfnMemImageCopy) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hImageSrc) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hImageDst) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }
    }

    ur_result_t result =
        pfnMemImageCopy(hQueue, hImageSrc, hImageDst, srcOrigin, dstOrigin,
                        region, numEventsInWaitList, phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueMemBufferMap
__urdlllocal ur_result_t UR_APICALL urEnqueueMemBufferMap(
    ur_queue_handle_t hQueue, ///< [in] handle of the queue object
    ur_mem_handle_t hBuffer,  ///< [in] handle of the buffer object
    bool blockingMap, ///< [in] indicates blocking (true), non-blocking (false)
    ur_map_flags_t mapFlags, ///< [in] flags for read, write, readwrite mapping
    size_t offset, ///< [in] offset in bytes of the buffer region being mapped
    size_t size,   ///< [in] size in bytes of the buffer region being mapped
    uint32_t numEventsInWaitList, ///< [in] size of the event wait list
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before this command can be executed.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that this
    ///< command does not wait on any event to complete.
    ur_event_handle_t *
        phEvent, ///< [out][optional] return an event object that identifies this particular
                 ///< command instance.
    void **ppRetMap ///< [out] return mapped pointer.  TODO: move it before
                    ///< numEventsInWaitList?
) {
    auto pfnMemBufferMap = context.urDdiTable.Enqueue.pfnMemBufferMap;

    if (nullptr == pfnMemBufferMap) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hBuffer) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == ppRetMap) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (UR_MAP_FLAGS_MASK & mapFlags) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }
    }

    ur_result_t result = pfnMemBufferMap(hQueue, hBuffer, blockingMap, mapFlags,
                                         offset, size, numEventsInWaitList,
                                         phEventWaitList, phEvent, ppRetMap);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueMemUnmap
__urdlllocal ur_result_t UR_APICALL urEnqueueMemUnmap(
    ur_queue_handle_t hQueue, ///< [in] handle of the queue object
    ur_mem_handle_t
        hMem,         ///< [in] handle of the memory (buffer or image) object
    void *pMappedPtr, ///< [in] mapped host address
    uint32_t numEventsInWaitList, ///< [in] size of the event wait list
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before this command can be executed.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that this
    ///< command does not wait on any event to complete.
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< command instance.
) {
    auto pfnMemUnmap = context.urDdiTable.Enqueue.pfnMemUnmap;

    if (nullptr == pfnMemUnmap) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hMem) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pMappedPtr) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }
    }

    ur_result_t result =
        pfnMemUnmap(hQueue, hMem, pMappedPtr, numEventsInWaitList,
                    phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueUSMFill
__urdlllocal ur_result_t UR_APICALL urEnqueueUSMFill(
    ur_queue_handle_t hQueue, ///< [in] handle of the queue object
    void *ptr,                ///< [in] pointer to USM memory object
    size_t
        patternSize, ///< [in] the size in bytes of the pattern. Must be a power of 2 and less
                     ///< than or equal to width.
    const void
        *pPattern, ///< [in] pointer with the bytes of the pattern to set.
    size_t
        size, ///< [in] size in bytes to be set. Must be a multiple of patternSize.
    uint32_t numEventsInWaitList, ///< [in] size of the event wait list
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before this command can be executed.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that this
    ///< command does not wait on any event to complete.
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< command instance.
) {
    auto pfnUSMFill = context.urDdiTable.Enqueue.pfnUSMFill;

    if (nullptr == pfnUSMFill) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == ptr) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == pPattern) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (size == 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (size % patternSize != 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (patternSize == 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (patternSize > size) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (patternSize != 0 && ((patternSize & (patternSize - 1)) != 0)) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }
    }

    ur_result_t result =
        pfnUSMFill(hQueue, ptr, patternSize, pPattern, size,
                   numEventsInWaitList, phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueUSMMemcpy
__urdlllocal ur_result_t UR_APICALL urEnqueueUSMMemcpy(
    ur_queue_handle_t hQueue, ///< [in] handle of the queue object
    bool blocking,            ///< [in] blocking or non-blocking copy
    void *pDst,       ///< [in] pointer to the destination USM memory object
    const void *pSrc, ///< [in] pointer to the source USM memory object
    size_t size,      ///< [in] size in bytes to be copied
    uint32_t numEventsInWaitList, ///< [in] size of the event wait list
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before this command can be executed.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that this
    ///< command does not wait on any event to complete.
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< command instance.
) {
    auto pfnUSMMemcpy = context.urDdiTable.Enqueue.pfnUSMMemcpy;

    if (nullptr == pfnUSMMemcpy) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pDst) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == pSrc) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (size == 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }
    }

    ur_result_t result =
        pfnUSMMemcpy(hQueue, blocking, pDst, pSrc, size, numEventsInWaitList,
                     phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueUSMPrefetch
__urdlllocal ur_result_t UR_APICALL urEnqueueUSMPrefetch(
    ur_queue_handle_t hQueue,       ///< [in] handle of the queue object
    const void *pMem,               ///< [in] pointer to the USM memory object
    size_t size,                    ///< [in] size in bytes to be fetched
    ur_usm_migration_flags_t flags, ///< [in] USM prefetch flags
    uint32_t numEventsInWaitList,   ///< [in] size of the event wait list
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before this command can be executed.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that this
    ///< command does not wait on any event to complete.
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< command instance.
) {
    auto pfnUSMPrefetch = context.urDdiTable.Enqueue.pfnUSMPrefetch;

    if (nullptr == pfnUSMPrefetch) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pMem) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (UR_USM_MIGRATION_FLAGS_MASK & flags) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }

        if (size == 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }
    }

    ur_result_t result =
        pfnUSMPrefetch(hQueue, pMem, size, flags, numEventsInWaitList,
                       phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueUSMAdvise
__urdlllocal ur_result_t UR_APICALL urEnqueueUSMAdvise(
    ur_queue_handle_t hQueue,     ///< [in] handle of the queue object
    const void *pMem,             ///< [in] pointer to the USM memory object
    size_t size,                  ///< [in] size in bytes to be advised
    ur_usm_advice_flags_t advice, ///< [in] USM memory advice
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< command instance.
) {
    auto pfnUSMAdvise = context.urDdiTable.Enqueue.pfnUSMAdvise;

    if (nullptr == pfnUSMAdvise) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pMem) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (UR_USM_ADVICE_FLAGS_MASK & advice) {
            return UR_RESULT_ERROR_INVALID_ENUMERATION;
        }

        if (size == 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }
    }

    ur_result_t result = pfnUSMAdvise(hQueue, pMem, size, advice, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueUSMFill2D
__urdlllocal ur_result_t UR_APICALL urEnqueueUSMFill2D(
    ur_queue_handle_t hQueue, ///< [in] handle of the queue to submit to.
    void *pMem,               ///< [in] pointer to memory to be filled.
    size_t
        pitch, ///< [in] the total width of the destination memory including padding.
    size_t
        patternSize, ///< [in] the size in bytes of the pattern. Must be a power of 2 and less
                     ///< than or equal to width.
    const void
        *pPattern, ///< [in] pointer with the bytes of the pattern to set.
    size_t
        width, ///< [in] the width in bytes of each row to fill. Must be a multiple of
               ///< patternSize.
    size_t height,                ///< [in] the height of the columns to fill.
    uint32_t numEventsInWaitList, ///< [in] size of the event wait list
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before the kernel execution.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that no wait
    ///< event.
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< kernel execution instance.
) {
    auto pfnUSMFill2D = context.urDdiTable.Enqueue.pfnUSMFill2D;

    if (nullptr == pfnUSMFill2D) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pMem) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == pPattern) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (pitch == 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (pitch < width) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (width == 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (width % patternSize != 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (height == 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (patternSize == 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (patternSize > width) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (patternSize != 0 && ((patternSize & (patternSize - 1)) != 0)) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }
    }

    ur_result_t result =
        pfnUSMFill2D(hQueue, pMem, pitch, patternSize, pPattern, width, height,
                     numEventsInWaitList, phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueUSMMemcpy2D
__urdlllocal ur_result_t UR_APICALL urEnqueueUSMMemcpy2D(
    ur_queue_handle_t hQueue, ///< [in] handle of the queue to submit to.
    bool blocking, ///< [in] indicates if this operation should block the host.
    void *pDst,    ///< [in] pointer to memory where data will be copied.
    size_t
        dstPitch, ///< [in] the total width of the source memory including padding.
    const void *pSrc, ///< [in] pointer to memory to be copied.
    size_t
        srcPitch, ///< [in] the total width of the source memory including padding.
    size_t width,  ///< [in] the width in bytes of each row to be copied.
    size_t height, ///< [in] the height of columns to be copied.
    uint32_t numEventsInWaitList, ///< [in] size of the event wait list
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before the kernel execution.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that no wait
    ///< event.
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< kernel execution instance.
) {
    auto pfnUSMMemcpy2D = context.urDdiTable.Enqueue.pfnUSMMemcpy2D;

    if (nullptr == pfnUSMMemcpy2D) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pDst) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == pSrc) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (srcPitch == 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (dstPitch == 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (srcPitch < width) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (dstPitch < width) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (height == 0) {
            return UR_RESULT_ERROR_INVALID_SIZE;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }
    }

    ur_result_t result =
        pfnUSMMemcpy2D(hQueue, blocking, pDst, dstPitch, pSrc, srcPitch, width,
                       height, numEventsInWaitList, phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueDeviceGlobalVariableWrite
__urdlllocal ur_result_t UR_APICALL urEnqueueDeviceGlobalVariableWrite(
    ur_queue_handle_t hQueue, ///< [in] handle of the queue to submit to.
    ur_program_handle_t
        hProgram, ///< [in] handle of the program containing the device global variable.
    const char
        *name, ///< [in] the unique identifier for the device global variable.
    bool blockingWrite, ///< [in] indicates if this operation should block.
    size_t count,       ///< [in] the number of bytes to copy.
    size_t
        offset, ///< [in] the byte offset into the device global variable to start copying.
    const void *pSrc, ///< [in] pointer to where the data must be copied from.
    uint32_t numEventsInWaitList, ///< [in] size of the event wait list.
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before the kernel execution.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that no wait
    ///< event.
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< kernel execution instance.
) {
    auto pfnDeviceGlobalVariableWrite =
        context.urDdiTable.Enqueue.pfnDeviceGlobalVariableWrite;

    if (nullptr == pfnDeviceGlobalVariableWrite) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hProgram) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == name) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == pSrc) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }
    }

    ur_result_t result = pfnDeviceGlobalVariableWrite(
        hQueue, hProgram, name, blockingWrite, count, offset, pSrc,
        numEventsInWaitList, phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueDeviceGlobalVariableRead
__urdlllocal ur_result_t UR_APICALL urEnqueueDeviceGlobalVariableRead(
    ur_queue_handle_t hQueue, ///< [in] handle of the queue to submit to.
    ur_program_handle_t
        hProgram, ///< [in] handle of the program containing the device global variable.
    const char
        *name, ///< [in] the unique identifier for the device global variable.
    bool blockingRead, ///< [in] indicates if this operation should block.
    size_t count,      ///< [in] the number of bytes to copy.
    size_t
        offset, ///< [in] the byte offset into the device global variable to start copying.
    void *pDst, ///< [in] pointer to where the data must be copied to.
    uint32_t numEventsInWaitList, ///< [in] size of the event wait list.
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before the kernel execution.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that no wait
    ///< event.
    ur_event_handle_t *
        phEvent ///< [out][optional] return an event object that identifies this particular
                ///< kernel execution instance.
) {
    auto pfnDeviceGlobalVariableRead =
        context.urDdiTable.Enqueue.pfnDeviceGlobalVariableRead;

    if (nullptr == pfnDeviceGlobalVariableRead) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hProgram) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == name) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == pDst) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }
    }

    ur_result_t result = pfnDeviceGlobalVariableRead(
        hQueue, hProgram, name, blockingRead, count, offset, pDst,
        numEventsInWaitList, phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueReadHostPipe
__urdlllocal ur_result_t UR_APICALL urEnqueueReadHostPipe(
    ur_queue_handle_t
        hQueue, ///< [in] a valid host command-queue in which the read command
    ///< will be queued. hQueue and hProgram must be created with the same
    ///< UR context.
    ur_program_handle_t
        hProgram, ///< [in] a program object with a successfully built executable.
    const char *
        pipe_symbol, ///< [in] the name of the program scope pipe global variable.
    bool
        blocking, ///< [in] indicate if the read operation is blocking or non-blocking.
    void *
        pDst, ///< [in] a pointer to buffer in host memory that will hold resulting data
              ///< from pipe.
    size_t size, ///< [in] size of the memory region to read, in bytes.
    uint32_t numEventsInWaitList, ///< [in] number of events in the wait list.
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before the host pipe read.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that no wait event.
    ur_event_handle_t *
        phEvent ///< [out][optional] returns an event object that identifies this read
                ///< command
    ///< and can be used to query or queue a wait for this command to complete.
) {
    auto pfnReadHostPipe = context.urDdiTable.Enqueue.pfnReadHostPipe;

    if (nullptr == pfnReadHostPipe) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hProgram) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pipe_symbol) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == pDst) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }
    }

    ur_result_t result =
        pfnReadHostPipe(hQueue, hProgram, pipe_symbol, blocking, pDst, size,
                        numEventsInWaitList, phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Intercept function for urEnqueueWriteHostPipe
__urdlllocal ur_result_t UR_APICALL urEnqueueWriteHostPipe(
    ur_queue_handle_t
        hQueue, ///< [in] a valid host command-queue in which the write command
    ///< will be queued. hQueue and hProgram must be created with the same
    ///< UR context.
    ur_program_handle_t
        hProgram, ///< [in] a program object with a successfully built executable.
    const char *
        pipe_symbol, ///< [in] the name of the program scope pipe global variable.
    bool
        blocking, ///< [in] indicate if the read and write operations are blocking or
                  ///< non-blocking.
    void *
        pSrc, ///< [in] a pointer to buffer in host memory that holds data to be written
              ///< to the host pipe.
    size_t size, ///< [in] size of the memory region to read or write, in bytes.
    uint32_t numEventsInWaitList, ///< [in] number of events in the wait list.
    const ur_event_handle_t *
        phEventWaitList, ///< [in][optional][range(0, numEventsInWaitList)] pointer to a list of
    ///< events that must be complete before the host pipe write.
    ///< If nullptr, the numEventsInWaitList must be 0, indicating that no wait event.
    ur_event_handle_t *
        phEvent ///< [out] returns an event object that identifies this write command
    ///< and can be used to query or queue a wait for this command to complete.
) {
    auto pfnWriteHostPipe = context.urDdiTable.Enqueue.pfnWriteHostPipe;

    if (nullptr == pfnWriteHostPipe) {
        return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }

    if (context.enableParameterValidation) {
        if (NULL == hQueue) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == hProgram) {
            return UR_RESULT_ERROR_INVALID_NULL_HANDLE;
        }

        if (NULL == pipe_symbol) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == pSrc) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (NULL == phEvent) {
            return UR_RESULT_ERROR_INVALID_NULL_POINTER;
        }

        if (phEventWaitList == NULL && numEventsInWaitList > 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }

        if (phEventWaitList != NULL && numEventsInWaitList == 0) {
            return UR_RESULT_ERROR_INVALID_EVENT_WAIT_LIST;
        }
    }

    ur_result_t result =
        pfnWriteHostPipe(hQueue, hProgram, pipe_symbol, blocking, pSrc, size,
                         numEventsInWaitList, phEventWaitList, phEvent);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Exported function for filling application's Global table
///        with current process' addresses
///
/// @returns
///     - ::UR_RESULT_SUCCESS
///     - ::UR_RESULT_ERROR_INVALID_NULL_POINTER
///     - ::UR_RESULT_ERROR_UNSUPPORTED_VERSION
UR_DLLEXPORT ur_result_t UR_APICALL urGetGlobalProcAddrTable(
    ur_api_version_t version, ///< [in] API version requested
    ur_global_dditable_t
        *pDdiTable ///< [in,out] pointer to table of DDI function pointers
) {
    auto &dditable = ur_validation_layer::context.urDdiTable.Global;

    if (nullptr == pDdiTable) {
        return UR_RESULT_ERROR_INVALID_NULL_POINTER;
    }

    if (UR_MAJOR_VERSION(ur_validation_layer::context.version) !=
            UR_MAJOR_VERSION(version) ||
        UR_MINOR_VERSION(ur_validation_layer::context.version) >
            UR_MINOR_VERSION(version)) {
        return UR_RESULT_ERROR_UNSUPPORTED_VERSION;
    }

    ur_result_t result = UR_RESULT_SUCCESS;

    dditable.pfnInit = pDdiTable->pfnInit;
    pDdiTable->pfnInit = ur_validation_layer::urInit;

    dditable.pfnGetLastResult = pDdiTable->pfnGetLastResult;
    pDdiTable->pfnGetLastResult = ur_validation_layer::urGetLastResult;

    dditable.pfnTearDown = pDdiTable->pfnTearDown;
    pDdiTable->pfnTearDown = ur_validation_layer::urTearDown;

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Exported function for filling application's Context table
///        with current process' addresses
///
/// @returns
///     - ::UR_RESULT_SUCCESS
///     - ::UR_RESULT_ERROR_INVALID_NULL_POINTER
///     - ::UR_RESULT_ERROR_UNSUPPORTED_VERSION
UR_DLLEXPORT ur_result_t UR_APICALL urGetContextProcAddrTable(
    ur_api_version_t version, ///< [in] API version requested
    ur_context_dditable_t
        *pDdiTable ///< [in,out] pointer to table of DDI function pointers
) {
    auto &dditable = ur_validation_layer::context.urDdiTable.Context;

    if (nullptr == pDdiTable) {
        return UR_RESULT_ERROR_INVALID_NULL_POINTER;
    }

    if (UR_MAJOR_VERSION(ur_validation_layer::context.version) !=
            UR_MAJOR_VERSION(version) ||
        UR_MINOR_VERSION(ur_validation_layer::context.version) >
            UR_MINOR_VERSION(version)) {
        return UR_RESULT_ERROR_UNSUPPORTED_VERSION;
    }

    ur_result_t result = UR_RESULT_SUCCESS;

    dditable.pfnCreate = pDdiTable->pfnCreate;
    pDdiTable->pfnCreate = ur_validation_layer::urContextCreate;

    dditable.pfnRetain = pDdiTable->pfnRetain;
    pDdiTable->pfnRetain = ur_validation_layer::urContextRetain;

    dditable.pfnRelease = pDdiTable->pfnRelease;
    pDdiTable->pfnRelease = ur_validation_layer::urContextRelease;

    dditable.pfnGetInfo = pDdiTable->pfnGetInfo;
    pDdiTable->pfnGetInfo = ur_validation_layer::urContextGetInfo;

    dditable.pfnGetNativeHandle = pDdiTable->pfnGetNativeHandle;
    pDdiTable->pfnGetNativeHandle =
        ur_validation_layer::urContextGetNativeHandle;

    dditable.pfnCreateWithNativeHandle = pDdiTable->pfnCreateWithNativeHandle;
    pDdiTable->pfnCreateWithNativeHandle =
        ur_validation_layer::urContextCreateWithNativeHandle;

    dditable.pfnSetExtendedDeleter = pDdiTable->pfnSetExtendedDeleter;
    pDdiTable->pfnSetExtendedDeleter =
        ur_validation_layer::urContextSetExtendedDeleter;

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Exported function for filling application's Enqueue table
///        with current process' addresses
///
/// @returns
///     - ::UR_RESULT_SUCCESS
///     - ::UR_RESULT_ERROR_INVALID_NULL_POINTER
///     - ::UR_RESULT_ERROR_UNSUPPORTED_VERSION
UR_DLLEXPORT ur_result_t UR_APICALL urGetEnqueueProcAddrTable(
    ur_api_version_t version, ///< [in] API version requested
    ur_enqueue_dditable_t
        *pDdiTable ///< [in,out] pointer to table of DDI function pointers
) {
    auto &dditable = ur_validation_layer::context.urDdiTable.Enqueue;

    if (nullptr == pDdiTable) {
        return UR_RESULT_ERROR_INVALID_NULL_POINTER;
    }

    if (UR_MAJOR_VERSION(ur_validation_layer::context.version) !=
            UR_MAJOR_VERSION(version) ||
        UR_MINOR_VERSION(ur_validation_layer::context.version) >
            UR_MINOR_VERSION(version)) {
        return UR_RESULT_ERROR_UNSUPPORTED_VERSION;
    }

    ur_result_t result = UR_RESULT_SUCCESS;

    dditable.pfnKernelLaunch = pDdiTable->pfnKernelLaunch;
    pDdiTable->pfnKernelLaunch = ur_validation_layer::urEnqueueKernelLaunch;

    dditable.pfnEventsWait = pDdiTable->pfnEventsWait;
    pDdiTable->pfnEventsWait = ur_validation_layer::urEnqueueEventsWait;

    dditable.pfnEventsWaitWithBarrier = pDdiTable->pfnEventsWaitWithBarrier;
    pDdiTable->pfnEventsWaitWithBarrier =
        ur_validation_layer::urEnqueueEventsWaitWithBarrier;

    dditable.pfnMemBufferRead = pDdiTable->pfnMemBufferRead;
    pDdiTable->pfnMemBufferRead = ur_validation_layer::urEnqueueMemBufferRead;

    dditable.pfnMemBufferWrite = pDdiTable->pfnMemBufferWrite;
    pDdiTable->pfnMemBufferWrite = ur_validation_layer::urEnqueueMemBufferWrite;

    dditable.pfnMemBufferReadRect = pDdiTable->pfnMemBufferReadRect;
    pDdiTable->pfnMemBufferReadRect =
        ur_validation_layer::urEnqueueMemBufferReadRect;

    dditable.pfnMemBufferWriteRect = pDdiTable->pfnMemBufferWriteRect;
    pDdiTable->pfnMemBufferWriteRect =
        ur_validation_layer::urEnqueueMemBufferWriteRect;

    dditable.pfnMemBufferCopy = pDdiTable->pfnMemBufferCopy;
    pDdiTable->pfnMemBufferCopy = ur_validation_layer::urEnqueueMemBufferCopy;

    dditable.pfnMemBufferCopyRect = pDdiTable->pfnMemBufferCopyRect;
    pDdiTable->pfnMemBufferCopyRect =
        ur_validation_layer::urEnqueueMemBufferCopyRect;

    dditable.pfnMemBufferFill = pDdiTable->pfnMemBufferFill;
    pDdiTable->pfnMemBufferFill = ur_validation_layer::urEnqueueMemBufferFill;

    dditable.pfnMemImageRead = pDdiTable->pfnMemImageRead;
    pDdiTable->pfnMemImageRead = ur_validation_layer::urEnqueueMemImageRead;

    dditable.pfnMemImageWrite = pDdiTable->pfnMemImageWrite;
    pDdiTable->pfnMemImageWrite = ur_validation_layer::urEnqueueMemImageWrite;

    dditable.pfnMemImageCopy = pDdiTable->pfnMemImageCopy;
    pDdiTable->pfnMemImageCopy = ur_validation_layer::urEnqueueMemImageCopy;

    dditable.pfnMemBufferMap = pDdiTable->pfnMemBufferMap;
    pDdiTable->pfnMemBufferMap = ur_validation_layer::urEnqueueMemBufferMap;

    dditable.pfnMemUnmap = pDdiTable->pfnMemUnmap;
    pDdiTable->pfnMemUnmap = ur_validation_layer::urEnqueueMemUnmap;

    dditable.pfnUSMFill = pDdiTable->pfnUSMFill;
    pDdiTable->pfnUSMFill = ur_validation_layer::urEnqueueUSMFill;

    dditable.pfnUSMMemcpy = pDdiTable->pfnUSMMemcpy;
    pDdiTable->pfnUSMMemcpy = ur_validation_layer::urEnqueueUSMMemcpy;

    dditable.pfnUSMPrefetch = pDdiTable->pfnUSMPrefetch;
    pDdiTable->pfnUSMPrefetch = ur_validation_layer::urEnqueueUSMPrefetch;

    dditable.pfnUSMAdvise = pDdiTable->pfnUSMAdvise;
    pDdiTable->pfnUSMAdvise = ur_validation_layer::urEnqueueUSMAdvise;

    dditable.pfnUSMFill2D = pDdiTable->pfnUSMFill2D;
    pDdiTable->pfnUSMFill2D = ur_validation_layer::urEnqueueUSMFill2D;

    dditable.pfnUSMMemcpy2D = pDdiTable->pfnUSMMemcpy2D;
    pDdiTable->pfnUSMMemcpy2D = ur_validation_layer::urEnqueueUSMMemcpy2D;

    dditable.pfnDeviceGlobalVariableWrite =
        pDdiTable->pfnDeviceGlobalVariableWrite;
    pDdiTable->pfnDeviceGlobalVariableWrite =
        ur_validation_layer::urEnqueueDeviceGlobalVariableWrite;

    dditable.pfnDeviceGlobalVariableRead =
        pDdiTable->pfnDeviceGlobalVariableRead;
    pDdiTable->pfnDeviceGlobalVariableRead =
        ur_validation_layer::urEnqueueDeviceGlobalVariableRead;

    dditable.pfnReadHostPipe = pDdiTable->pfnReadHostPipe;
    pDdiTable->pfnReadHostPipe = ur_validation_layer::urEnqueueReadHostPipe;

    dditable.pfnWriteHostPipe = pDdiTable->pfnWriteHostPipe;
    pDdiTable->pfnWriteHostPipe = ur_validation_layer::urEnqueueWriteHostPipe;

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Exported function for filling application's Event table
///        with current process' addresses
///
/// @returns
///     - ::UR_RESULT_SUCCESS
///     - ::UR_RESULT_ERROR_INVALID_NULL_POINTER
///     - ::UR_RESULT_ERROR_UNSUPPORTED_VERSION
UR_DLLEXPORT ur_result_t UR_APICALL urGetEventProcAddrTable(
    ur_api_version_t version, ///< [in] API version requested
    ur_event_dditable_t
        *pDdiTable ///< [in,out] pointer to table of DDI function pointers
) {
    auto &dditable = ur_validation_layer::context.urDdiTable.Event;

    if (nullptr == pDdiTable) {
        return UR_RESULT_ERROR_INVALID_NULL_POINTER;
    }

    if (UR_MAJOR_VERSION(ur_validation_layer::context.version) !=
            UR_MAJOR_VERSION(version) ||
        UR_MINOR_VERSION(ur_validation_layer::context.version) >
            UR_MINOR_VERSION(version)) {
        return UR_RESULT_ERROR_UNSUPPORTED_VERSION;
    }

    ur_result_t result = UR_RESULT_SUCCESS;

    dditable.pfnGetInfo = pDdiTable->pfnGetInfo;
    pDdiTable->pfnGetInfo = ur_validation_layer::urEventGetInfo;

    dditable.pfnGetProfilingInfo = pDdiTable->pfnGetProfilingInfo;
    pDdiTable->pfnGetProfilingInfo =
        ur_validation_layer::urEventGetProfilingInfo;

    dditable.pfnWait = pDdiTable->pfnWait;
    pDdiTable->pfnWait = ur_validation_layer::urEventWait;

    dditable.pfnRetain = pDdiTable->pfnRetain;
    pDdiTable->pfnRetain = ur_validation_layer::urEventRetain;

    dditable.pfnRelease = pDdiTable->pfnRelease;
    pDdiTable->pfnRelease = ur_validation_layer::urEventRelease;

    dditable.pfnGetNativeHandle = pDdiTable->pfnGetNativeHandle;
    pDdiTable->pfnGetNativeHandle = ur_validation_layer::urEventGetNativeHandle;

    dditable.pfnCreateWithNativeHandle = pDdiTable->pfnCreateWithNativeHandle;
    pDdiTable->pfnCreateWithNativeHandle =
        ur_validation_layer::urEventCreateWithNativeHandle;

    dditable.pfnSetCallback = pDdiTable->pfnSetCallback;
    pDdiTable->pfnSetCallback = ur_validation_layer::urEventSetCallback;

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Exported function for filling application's Kernel table
///        with current process' addresses
///
/// @returns
///     - ::UR_RESULT_SUCCESS
///     - ::UR_RESULT_ERROR_INVALID_NULL_POINTER
///     - ::UR_RESULT_ERROR_UNSUPPORTED_VERSION
UR_DLLEXPORT ur_result_t UR_APICALL urGetKernelProcAddrTable(
    ur_api_version_t version, ///< [in] API version requested
    ur_kernel_dditable_t
        *pDdiTable ///< [in,out] pointer to table of DDI function pointers
) {
    auto &dditable = ur_validation_layer::context.urDdiTable.Kernel;

    if (nullptr == pDdiTable) {
        return UR_RESULT_ERROR_INVALID_NULL_POINTER;
    }

    if (UR_MAJOR_VERSION(ur_validation_layer::context.version) !=
            UR_MAJOR_VERSION(version) ||
        UR_MINOR_VERSION(ur_validation_layer::context.version) >
            UR_MINOR_VERSION(version)) {
        return UR_RESULT_ERROR_UNSUPPORTED_VERSION;
    }

    ur_result_t result = UR_RESULT_SUCCESS;

    dditable.pfnCreate = pDdiTable->pfnCreate;
    pDdiTable->pfnCreate = ur_validation_layer::urKernelCreate;

    dditable.pfnGetInfo = pDdiTable->pfnGetInfo;
    pDdiTable->pfnGetInfo = ur_validation_layer::urKernelGetInfo;

    dditable.pfnGetGroupInfo = pDdiTable->pfnGetGroupInfo;
    pDdiTable->pfnGetGroupInfo = ur_validation_layer::urKernelGetGroupInfo;

    dditable.pfnGetSubGroupInfo = pDdiTable->pfnGetSubGroupInfo;
    pDdiTable->pfnGetSubGroupInfo =
        ur_validation_layer::urKernelGetSubGroupInfo;

    dditable.pfnRetain = pDdiTable->pfnRetain;
    pDdiTable->pfnRetain = ur_validation_layer::urKernelRetain;

    dditable.pfnRelease = pDdiTable->pfnRelease;
    pDdiTable->pfnRelease = ur_validation_layer::urKernelRelease;

    dditable.pfnGetNativeHandle = pDdiTable->pfnGetNativeHandle;
    pDdiTable->pfnGetNativeHandle =
        ur_validation_layer::urKernelGetNativeHandle;

    dditable.pfnCreateWithNativeHandle = pDdiTable->pfnCreateWithNativeHandle;
    pDdiTable->pfnCreateWithNativeHandle =
        ur_validation_layer::urKernelCreateWithNativeHandle;

    dditable.pfnSetArgValue = pDdiTable->pfnSetArgValue;
    pDdiTable->pfnSetArgValue = ur_validation_layer::urKernelSetArgValue;

    dditable.pfnSetArgLocal = pDdiTable->pfnSetArgLocal;
    pDdiTable->pfnSetArgLocal = ur_validation_layer::urKernelSetArgLocal;

    dditable.pfnSetArgPointer = pDdiTable->pfnSetArgPointer;
    pDdiTable->pfnSetArgPointer = ur_validation_layer::urKernelSetArgPointer;

    dditable.pfnSetExecInfo = pDdiTable->pfnSetExecInfo;
    pDdiTable->pfnSetExecInfo = ur_validation_layer::urKernelSetExecInfo;

    dditable.pfnSetArgSampler = pDdiTable->pfnSetArgSampler;
    pDdiTable->pfnSetArgSampler = ur_validation_layer::urKernelSetArgSampler;

    dditable.pfnSetArgMemObj = pDdiTable->pfnSetArgMemObj;
    pDdiTable->pfnSetArgMemObj = ur_validation_layer::urKernelSetArgMemObj;

    dditable.pfnSetSpecializationConstants =
        pDdiTable->pfnSetSpecializationConstants;
    pDdiTable->pfnSetSpecializationConstants =
        ur_validation_layer::urKernelSetSpecializationConstants;

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Exported function for filling application's Mem table
///        with current process' addresses
///
/// @returns
///     - ::UR_RESULT_SUCCESS
///     - ::UR_RESULT_ERROR_INVALID_NULL_POINTER
///     - ::UR_RESULT_ERROR_UNSUPPORTED_VERSION
UR_DLLEXPORT ur_result_t UR_APICALL urGetMemProcAddrTable(
    ur_api_version_t version, ///< [in] API version requested
    ur_mem_dditable_t
        *pDdiTable ///< [in,out] pointer to table of DDI function pointers
) {
    auto &dditable = ur_validation_layer::context.urDdiTable.Mem;

    if (nullptr == pDdiTable) {
        return UR_RESULT_ERROR_INVALID_NULL_POINTER;
    }

    if (UR_MAJOR_VERSION(ur_validation_layer::context.version) !=
            UR_MAJOR_VERSION(version) ||
        UR_MINOR_VERSION(ur_validation_layer::context.version) >
            UR_MINOR_VERSION(version)) {
        return UR_RESULT_ERROR_UNSUPPORTED_VERSION;
    }

    ur_result_t result = UR_RESULT_SUCCESS;

    dditable.pfnImageCreate = pDdiTable->pfnImageCreate;
    pDdiTable->pfnImageCreate = ur_validation_layer::urMemImageCreate;

    dditable.pfnBufferCreate = pDdiTable->pfnBufferCreate;
    pDdiTable->pfnBufferCreate = ur_validation_layer::urMemBufferCreate;

    dditable.pfnRetain = pDdiTable->pfnRetain;
    pDdiTable->pfnRetain = ur_validation_layer::urMemRetain;

    dditable.pfnRelease = pDdiTable->pfnRelease;
    pDdiTable->pfnRelease = ur_validation_layer::urMemRelease;

    dditable.pfnBufferPartition = pDdiTable->pfnBufferPartition;
    pDdiTable->pfnBufferPartition = ur_validation_layer::urMemBufferPartition;

    dditable.pfnGetNativeHandle = pDdiTable->pfnGetNativeHandle;
    pDdiTable->pfnGetNativeHandle = ur_validation_layer::urMemGetNativeHandle;

    dditable.pfnBufferCreateWithNativeHandle =
        pDdiTable->pfnBufferCreateWithNativeHandle;
    pDdiTable->pfnBufferCreateWithNativeHandle =
        ur_validation_layer::urMemBufferCreateWithNativeHandle;

    dditable.pfnImageCreateWithNativeHandle =
        pDdiTable->pfnImageCreateWithNativeHandle;
    pDdiTable->pfnImageCreateWithNativeHandle =
        ur_validation_layer::urMemImageCreateWithNativeHandle;

    dditable.pfnGetInfo = pDdiTable->pfnGetInfo;
    pDdiTable->pfnGetInfo = ur_validation_layer::urMemGetInfo;

    dditable.pfnImageGetInfo = pDdiTable->pfnImageGetInfo;
    pDdiTable->pfnImageGetInfo = ur_validation_layer::urMemImageGetInfo;

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Exported function for filling application's Platform table
///        with current process' addresses
///
/// @returns
///     - ::UR_RESULT_SUCCESS
///     - ::UR_RESULT_ERROR_INVALID_NULL_POINTER
///     - ::UR_RESULT_ERROR_UNSUPPORTED_VERSION
UR_DLLEXPORT ur_result_t UR_APICALL urGetPlatformProcAddrTable(
    ur_api_version_t version, ///< [in] API version requested
    ur_platform_dditable_t
        *pDdiTable ///< [in,out] pointer to table of DDI function pointers
) {
    auto &dditable = ur_validation_layer::context.urDdiTable.Platform;

    if (nullptr == pDdiTable) {
        return UR_RESULT_ERROR_INVALID_NULL_POINTER;
    }

    if (UR_MAJOR_VERSION(ur_validation_layer::context.version) !=
            UR_MAJOR_VERSION(version) ||
        UR_MINOR_VERSION(ur_validation_layer::context.version) >
            UR_MINOR_VERSION(version)) {
        return UR_RESULT_ERROR_UNSUPPORTED_VERSION;
    }

    ur_result_t result = UR_RESULT_SUCCESS;

    dditable.pfnGet = pDdiTable->pfnGet;
    pDdiTable->pfnGet = ur_validation_layer::urPlatformGet;

    dditable.pfnGetInfo = pDdiTable->pfnGetInfo;
    pDdiTable->pfnGetInfo = ur_validation_layer::urPlatformGetInfo;

    dditable.pfnGetNativeHandle = pDdiTable->pfnGetNativeHandle;
    pDdiTable->pfnGetNativeHandle =
        ur_validation_layer::urPlatformGetNativeHandle;

    dditable.pfnCreateWithNativeHandle = pDdiTable->pfnCreateWithNativeHandle;
    pDdiTable->pfnCreateWithNativeHandle =
        ur_validation_layer::urPlatformCreateWithNativeHandle;

    dditable.pfnGetApiVersion = pDdiTable->pfnGetApiVersion;
    pDdiTable->pfnGetApiVersion = ur_validation_layer::urPlatformGetApiVersion;

    dditable.pfnGetBackendOption = pDdiTable->pfnGetBackendOption;
    pDdiTable->pfnGetBackendOption =
        ur_validation_layer::urPlatformGetBackendOption;

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Exported function for filling application's Program table
///        with current process' addresses
///
/// @returns
///     - ::UR_RESULT_SUCCESS
///     - ::UR_RESULT_ERROR_INVALID_NULL_POINTER
///     - ::UR_RESULT_ERROR_UNSUPPORTED_VERSION
UR_DLLEXPORT ur_result_t UR_APICALL urGetProgramProcAddrTable(
    ur_api_version_t version, ///< [in] API version requested
    ur_program_dditable_t
        *pDdiTable ///< [in,out] pointer to table of DDI function pointers
) {
    auto &dditable = ur_validation_layer::context.urDdiTable.Program;

    if (nullptr == pDdiTable) {
        return UR_RESULT_ERROR_INVALID_NULL_POINTER;
    }

    if (UR_MAJOR_VERSION(ur_validation_layer::context.version) !=
            UR_MAJOR_VERSION(version) ||
        UR_MINOR_VERSION(ur_validation_layer::context.version) >
            UR_MINOR_VERSION(version)) {
        return UR_RESULT_ERROR_UNSUPPORTED_VERSION;
    }

    ur_result_t result = UR_RESULT_SUCCESS;

    dditable.pfnCreateWithIL = pDdiTable->pfnCreateWithIL;
    pDdiTable->pfnCreateWithIL = ur_validation_layer::urProgramCreateWithIL;

    dditable.pfnCreateWithBinary = pDdiTable->pfnCreateWithBinary;
    pDdiTable->pfnCreateWithBinary =
        ur_validation_layer::urProgramCreateWithBinary;

    dditable.pfnBuild = pDdiTable->pfnBuild;
    pDdiTable->pfnBuild = ur_validation_layer::urProgramBuild;

    dditable.pfnCompile = pDdiTable->pfnCompile;
    pDdiTable->pfnCompile = ur_validation_layer::urProgramCompile;

    dditable.pfnLink = pDdiTable->pfnLink;
    pDdiTable->pfnLink = ur_validation_layer::urProgramLink;

    dditable.pfnRetain = pDdiTable->pfnRetain;
    pDdiTable->pfnRetain = ur_validation_layer::urProgramRetain;

    dditable.pfnRelease = pDdiTable->pfnRelease;
    pDdiTable->pfnRelease = ur_validation_layer::urProgramRelease;

    dditable.pfnGetFunctionPointer = pDdiTable->pfnGetFunctionPointer;
    pDdiTable->pfnGetFunctionPointer =
        ur_validation_layer::urProgramGetFunctionPointer;

    dditable.pfnGetInfo = pDdiTable->pfnGetInfo;
    pDdiTable->pfnGetInfo = ur_validation_layer::urProgramGetInfo;

    dditable.pfnGetBuildInfo = pDdiTable->pfnGetBuildInfo;
    pDdiTable->pfnGetBuildInfo = ur_validation_layer::urProgramGetBuildInfo;

    dditable.pfnSetSpecializationConstants =
        pDdiTable->pfnSetSpecializationConstants;
    pDdiTable->pfnSetSpecializationConstants =
        ur_validation_layer::urProgramSetSpecializationConstants;

    dditable.pfnGetNativeHandle = pDdiTable->pfnGetNativeHandle;
    pDdiTable->pfnGetNativeHandle =
        ur_validation_layer::urProgramGetNativeHandle;

    dditable.pfnCreateWithNativeHandle = pDdiTable->pfnCreateWithNativeHandle;
    pDdiTable->pfnCreateWithNativeHandle =
        ur_validation_layer::urProgramCreateWithNativeHandle;

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Exported function for filling application's Queue table
///        with current process' addresses
///
/// @returns
///     - ::UR_RESULT_SUCCESS
///     - ::UR_RESULT_ERROR_INVALID_NULL_POINTER
///     - ::UR_RESULT_ERROR_UNSUPPORTED_VERSION
UR_DLLEXPORT ur_result_t UR_APICALL urGetQueueProcAddrTable(
    ur_api_version_t version, ///< [in] API version requested
    ur_queue_dditable_t
        *pDdiTable ///< [in,out] pointer to table of DDI function pointers
) {
    auto &dditable = ur_validation_layer::context.urDdiTable.Queue;

    if (nullptr == pDdiTable) {
        return UR_RESULT_ERROR_INVALID_NULL_POINTER;
    }

    if (UR_MAJOR_VERSION(ur_validation_layer::context.version) !=
            UR_MAJOR_VERSION(version) ||
        UR_MINOR_VERSION(ur_validation_layer::context.version) >
            UR_MINOR_VERSION(version)) {
        return UR_RESULT_ERROR_UNSUPPORTED_VERSION;
    }

    ur_result_t result = UR_RESULT_SUCCESS;

    dditable.pfnGetInfo = pDdiTable->pfnGetInfo;
    pDdiTable->pfnGetInfo = ur_validation_layer::urQueueGetInfo;

    dditable.pfnCreate = pDdiTable->pfnCreate;
    pDdiTable->pfnCreate = ur_validation_layer::urQueueCreate;

    dditable.pfnRetain = pDdiTable->pfnRetain;
    pDdiTable->pfnRetain = ur_validation_layer::urQueueRetain;

    dditable.pfnRelease = pDdiTable->pfnRelease;
    pDdiTable->pfnRelease = ur_validation_layer::urQueueRelease;

    dditable.pfnGetNativeHandle = pDdiTable->pfnGetNativeHandle;
    pDdiTable->pfnGetNativeHandle = ur_validation_layer::urQueueGetNativeHandle;

    dditable.pfnCreateWithNativeHandle = pDdiTable->pfnCreateWithNativeHandle;
    pDdiTable->pfnCreateWithNativeHandle =
        ur_validation_layer::urQueueCreateWithNativeHandle;

    dditable.pfnFinish = pDdiTable->pfnFinish;
    pDdiTable->pfnFinish = ur_validation_layer::urQueueFinish;

    dditable.pfnFlush = pDdiTable->pfnFlush;
    pDdiTable->pfnFlush = ur_validation_layer::urQueueFlush;

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Exported function for filling application's Sampler table
///        with current process' addresses
///
/// @returns
///     - ::UR_RESULT_SUCCESS
///     - ::UR_RESULT_ERROR_INVALID_NULL_POINTER
///     - ::UR_RESULT_ERROR_UNSUPPORTED_VERSION
UR_DLLEXPORT ur_result_t UR_APICALL urGetSamplerProcAddrTable(
    ur_api_version_t version, ///< [in] API version requested
    ur_sampler_dditable_t
        *pDdiTable ///< [in,out] pointer to table of DDI function pointers
) {
    auto &dditable = ur_validation_layer::context.urDdiTable.Sampler;

    if (nullptr == pDdiTable) {
        return UR_RESULT_ERROR_INVALID_NULL_POINTER;
    }

    if (UR_MAJOR_VERSION(ur_validation_layer::context.version) !=
            UR_MAJOR_VERSION(version) ||
        UR_MINOR_VERSION(ur_validation_layer::context.version) >
            UR_MINOR_VERSION(version)) {
        return UR_RESULT_ERROR_UNSUPPORTED_VERSION;
    }

    ur_result_t result = UR_RESULT_SUCCESS;

    dditable.pfnCreate = pDdiTable->pfnCreate;
    pDdiTable->pfnCreate = ur_validation_layer::urSamplerCreate;

    dditable.pfnRetain = pDdiTable->pfnRetain;
    pDdiTable->pfnRetain = ur_validation_layer::urSamplerRetain;

    dditable.pfnRelease = pDdiTable->pfnRelease;
    pDdiTable->pfnRelease = ur_validation_layer::urSamplerRelease;

    dditable.pfnGetInfo = pDdiTable->pfnGetInfo;
    pDdiTable->pfnGetInfo = ur_validation_layer::urSamplerGetInfo;

    dditable.pfnGetNativeHandle = pDdiTable->pfnGetNativeHandle;
    pDdiTable->pfnGetNativeHandle =
        ur_validation_layer::urSamplerGetNativeHandle;

    dditable.pfnCreateWithNativeHandle = pDdiTable->pfnCreateWithNativeHandle;
    pDdiTable->pfnCreateWithNativeHandle =
        ur_validation_layer::urSamplerCreateWithNativeHandle;

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Exported function for filling application's USM table
///        with current process' addresses
///
/// @returns
///     - ::UR_RESULT_SUCCESS
///     - ::UR_RESULT_ERROR_INVALID_NULL_POINTER
///     - ::UR_RESULT_ERROR_UNSUPPORTED_VERSION
UR_DLLEXPORT ur_result_t UR_APICALL urGetUSMProcAddrTable(
    ur_api_version_t version, ///< [in] API version requested
    ur_usm_dditable_t
        *pDdiTable ///< [in,out] pointer to table of DDI function pointers
) {
    auto &dditable = ur_validation_layer::context.urDdiTable.USM;

    if (nullptr == pDdiTable) {
        return UR_RESULT_ERROR_INVALID_NULL_POINTER;
    }

    if (UR_MAJOR_VERSION(ur_validation_layer::context.version) !=
            UR_MAJOR_VERSION(version) ||
        UR_MINOR_VERSION(ur_validation_layer::context.version) >
            UR_MINOR_VERSION(version)) {
        return UR_RESULT_ERROR_UNSUPPORTED_VERSION;
    }

    ur_result_t result = UR_RESULT_SUCCESS;

    dditable.pfnHostAlloc = pDdiTable->pfnHostAlloc;
    pDdiTable->pfnHostAlloc = ur_validation_layer::urUSMHostAlloc;

    dditable.pfnDeviceAlloc = pDdiTable->pfnDeviceAlloc;
    pDdiTable->pfnDeviceAlloc = ur_validation_layer::urUSMDeviceAlloc;

    dditable.pfnSharedAlloc = pDdiTable->pfnSharedAlloc;
    pDdiTable->pfnSharedAlloc = ur_validation_layer::urUSMSharedAlloc;

    dditable.pfnFree = pDdiTable->pfnFree;
    pDdiTable->pfnFree = ur_validation_layer::urUSMFree;

    dditable.pfnGetMemAllocInfo = pDdiTable->pfnGetMemAllocInfo;
    pDdiTable->pfnGetMemAllocInfo = ur_validation_layer::urUSMGetMemAllocInfo;

    dditable.pfnPoolCreate = pDdiTable->pfnPoolCreate;
    pDdiTable->pfnPoolCreate = ur_validation_layer::urUSMPoolCreate;

    dditable.pfnPoolDestroy = pDdiTable->pfnPoolDestroy;
    pDdiTable->pfnPoolDestroy = ur_validation_layer::urUSMPoolDestroy;

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Exported function for filling application's Device table
///        with current process' addresses
///
/// @returns
///     - ::UR_RESULT_SUCCESS
///     - ::UR_RESULT_ERROR_INVALID_NULL_POINTER
///     - ::UR_RESULT_ERROR_UNSUPPORTED_VERSION
UR_DLLEXPORT ur_result_t UR_APICALL urGetDeviceProcAddrTable(
    ur_api_version_t version, ///< [in] API version requested
    ur_device_dditable_t
        *pDdiTable ///< [in,out] pointer to table of DDI function pointers
) {
    auto &dditable = ur_validation_layer::context.urDdiTable.Device;

    if (nullptr == pDdiTable) {
        return UR_RESULT_ERROR_INVALID_NULL_POINTER;
    }

    if (UR_MAJOR_VERSION(ur_validation_layer::context.version) !=
            UR_MAJOR_VERSION(version) ||
        UR_MINOR_VERSION(ur_validation_layer::context.version) >
            UR_MINOR_VERSION(version)) {
        return UR_RESULT_ERROR_UNSUPPORTED_VERSION;
    }

    ur_result_t result = UR_RESULT_SUCCESS;

    dditable.pfnGet = pDdiTable->pfnGet;
    pDdiTable->pfnGet = ur_validation_layer::urDeviceGet;

    dditable.pfnGetInfo = pDdiTable->pfnGetInfo;
    pDdiTable->pfnGetInfo = ur_validation_layer::urDeviceGetInfo;

    dditable.pfnRetain = pDdiTable->pfnRetain;
    pDdiTable->pfnRetain = ur_validation_layer::urDeviceRetain;

    dditable.pfnRelease = pDdiTable->pfnRelease;
    pDdiTable->pfnRelease = ur_validation_layer::urDeviceRelease;

    dditable.pfnPartition = pDdiTable->pfnPartition;
    pDdiTable->pfnPartition = ur_validation_layer::urDevicePartition;

    dditable.pfnSelectBinary = pDdiTable->pfnSelectBinary;
    pDdiTable->pfnSelectBinary = ur_validation_layer::urDeviceSelectBinary;

    dditable.pfnGetNativeHandle = pDdiTable->pfnGetNativeHandle;
    pDdiTable->pfnGetNativeHandle =
        ur_validation_layer::urDeviceGetNativeHandle;

    dditable.pfnCreateWithNativeHandle = pDdiTable->pfnCreateWithNativeHandle;
    pDdiTable->pfnCreateWithNativeHandle =
        ur_validation_layer::urDeviceCreateWithNativeHandle;

    dditable.pfnGetGlobalTimestamps = pDdiTable->pfnGetGlobalTimestamps;
    pDdiTable->pfnGetGlobalTimestamps =
        ur_validation_layer::urDeviceGetGlobalTimestamps;

    return result;
}

ur_result_t context_t::init(ur_dditable_t *dditable) {
    ur_result_t result = UR_RESULT_SUCCESS;

    if (UR_RESULT_SUCCESS == result) {
        result = ur_validation_layer::urGetGlobalProcAddrTable(
            UR_API_VERSION_CURRENT, &dditable->Global);
    }

    if (UR_RESULT_SUCCESS == result) {
        result = ur_validation_layer::urGetContextProcAddrTable(
            UR_API_VERSION_CURRENT, &dditable->Context);
    }

    if (UR_RESULT_SUCCESS == result) {
        result = ur_validation_layer::urGetEnqueueProcAddrTable(
            UR_API_VERSION_CURRENT, &dditable->Enqueue);
    }

    if (UR_RESULT_SUCCESS == result) {
        result = ur_validation_layer::urGetEventProcAddrTable(
            UR_API_VERSION_CURRENT, &dditable->Event);
    }

    if (UR_RESULT_SUCCESS == result) {
        result = ur_validation_layer::urGetKernelProcAddrTable(
            UR_API_VERSION_CURRENT, &dditable->Kernel);
    }

    if (UR_RESULT_SUCCESS == result) {
        result = ur_validation_layer::urGetMemProcAddrTable(
            UR_API_VERSION_CURRENT, &dditable->Mem);
    }

    if (UR_RESULT_SUCCESS == result) {
        result = ur_validation_layer::urGetPlatformProcAddrTable(
            UR_API_VERSION_CURRENT, &dditable->Platform);
    }

    if (UR_RESULT_SUCCESS == result) {
        result = ur_validation_layer::urGetProgramProcAddrTable(
            UR_API_VERSION_CURRENT, &dditable->Program);
    }

    if (UR_RESULT_SUCCESS == result) {
        result = ur_validation_layer::urGetQueueProcAddrTable(
            UR_API_VERSION_CURRENT, &dditable->Queue);
    }

    if (UR_RESULT_SUCCESS == result) {
        result = ur_validation_layer::urGetSamplerProcAddrTable(
            UR_API_VERSION_CURRENT, &dditable->Sampler);
    }

    if (UR_RESULT_SUCCESS == result) {
        result = ur_validation_layer::urGetUSMProcAddrTable(
            UR_API_VERSION_CURRENT, &dditable->USM);
    }

    if (UR_RESULT_SUCCESS == result) {
        result = ur_validation_layer::urGetDeviceProcAddrTable(
            UR_API_VERSION_CURRENT, &dditable->Device);
    }

    return result;
}

} // namespace ur_validation_layer
