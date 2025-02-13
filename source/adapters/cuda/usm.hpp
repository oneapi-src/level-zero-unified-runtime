//===--------- usm.hpp - CUDA Adapter -------------------------------------===//
//
// Copyright (C) 2023 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "common.hpp"

#include <umf_helpers.hpp>
#include <umf_pools/disjoint_pool_config_parser.hpp>

usm::DisjointPoolAllConfigs InitializeDisjointPoolConfig();

// A ur_usm_pool_handle_t can represent different types of memory pools. It may
// sit on top of a UMF pool or a CUmemoryPool, but not both.
struct ur_usm_pool_handle_t_ {
  std::atomic_uint32_t RefCount = 1;

  ur_context_handle_t Context = nullptr;
  ur_device_handle_t Device = nullptr;

  usm::DisjointPoolAllConfigs DisjointPoolConfigs =
      usm::DisjointPoolAllConfigs();

  umf::pool_unique_handle_t DeviceMemPool;
  umf::pool_unique_handle_t SharedMemPool;
  umf::pool_unique_handle_t HostMemPool;

  CUmemoryPool CUmemPool{0};
  bool CUHostMemPool = false;

  ur_usm_pool_handle_t_(ur_context_handle_t Context,
                        ur_usm_pool_desc_t *PoolDesc);

  // TODO: do we need the context param?
  ur_usm_pool_handle_t_(ur_context_handle_t Context, ur_device_handle_t Device,
                        ur_usm_pool_desc_t *PoolDesc);

  // TODO: this is messy
  ur_usm_pool_handle_t_(ur_context_handle_t Context, ur_device_handle_t Device,
                        CUmemoryPool CUmemPool);

  uint32_t incrementReferenceCount() noexcept { return ++RefCount; }

  uint32_t decrementReferenceCount() noexcept { return --RefCount; }

  uint32_t getReferenceCount() const noexcept { return RefCount; }

  bool hasUMFPool(umf_memory_pool_t *umf_pool);

  // To be used if ur_usm_pool_handle_t represents a CUmemoryPool
  bool usesCudaPool() const { return CUmemPool != CUmemoryPool{0}; };
  bool usesCudaHostPool() const { return CUHostMemPool; };
  CUmemoryPool getCudaPool() { return CUmemPool; };
};

// Exception type to pass allocation errors
class UsmAllocationException {
  const ur_result_t Error;

public:
  UsmAllocationException(ur_result_t Err) : Error{Err} {}
  ur_result_t getError() const { return Error; }
};

// Implements memory allocation via driver API for USM allocator interface.
class USMMemoryProvider {
private:
  ur_result_t &getLastStatusRef() {
    static thread_local ur_result_t LastStatus = UR_RESULT_SUCCESS;
    return LastStatus;
  }

protected:
  ur_context_handle_t Context;
  ur_device_handle_t Device;
  size_t MinPageSize;

  // Internal allocation routine which must be implemented for each allocation
  // type
  virtual ur_result_t allocateImpl(void **ResultPtr, size_t Size,
                                   uint32_t Alignment) = 0;

public:
  umf_result_t initialize(ur_context_handle_t Ctx, ur_device_handle_t Dev);
  umf_result_t alloc(size_t Size, size_t Align, void **Ptr);
  umf_result_t free(void *Ptr, size_t Size);
  void get_last_native_error(const char **ErrMsg, int32_t *ErrCode);
  umf_result_t get_min_page_size(void *, size_t *);
  umf_result_t get_recommended_page_size(size_t, size_t *) {
    return UMF_RESULT_ERROR_NOT_SUPPORTED;
  };
  umf_result_t purge_lazy(void *, size_t) {
    return UMF_RESULT_ERROR_NOT_SUPPORTED;
  };
  umf_result_t purge_force(void *, size_t) {
    return UMF_RESULT_ERROR_NOT_SUPPORTED;
  };
  umf_result_t allocation_merge(void *, void *, size_t) {
    return UMF_RESULT_ERROR_UNKNOWN;
  }
  umf_result_t allocation_split(void *, size_t, size_t) {
    return UMF_RESULT_ERROR_UNKNOWN;
  }
  virtual const char *get_name() = 0;

  virtual ~USMMemoryProvider() = default;
};

// Allocation routines for shared memory type
class USMSharedMemoryProvider final : public USMMemoryProvider {
public:
  const char *get_name() override { return "USMSharedMemoryProvider"; }

protected:
  ur_result_t allocateImpl(void **ResultPtr, size_t Size,
                           uint32_t Alignment) override;
};

// Allocation routines for device memory type
class USMDeviceMemoryProvider final : public USMMemoryProvider {
public:
  const char *get_name() override { return "USMSharedMemoryProvider"; }

protected:
  ur_result_t allocateImpl(void **ResultPtr, size_t Size,
                           uint32_t Alignment) override;
};

// Allocation routines for host memory type
class USMHostMemoryProvider final : public USMMemoryProvider {
public:
  const char *get_name() override { return "USMSharedMemoryProvider"; }

protected:
  ur_result_t allocateImpl(void **ResultPtr, size_t Size,
                           uint32_t Alignment) override;
};

ur_result_t USMDeviceAllocImpl(void **ResultPtr, ur_context_handle_t Context,
                               ur_device_handle_t Device,
                               ur_usm_device_mem_flags_t Flags, size_t Size,
                               uint32_t Alignment);

ur_result_t USMSharedAllocImpl(void **ResultPtr, ur_context_handle_t Context,
                               ur_device_handle_t Device,
                               ur_usm_host_mem_flags_t,
                               ur_usm_device_mem_flags_t, size_t Size,
                               uint32_t Alignment);

ur_result_t USMHostAllocImpl(void **ResultPtr, ur_context_handle_t Context,
                             ur_usm_host_mem_flags_t Flags, size_t Size,
                             uint32_t Alignment);
