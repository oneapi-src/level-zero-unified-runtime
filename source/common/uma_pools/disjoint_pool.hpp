//===---------- disjoint_pool.hpp - Allocator for USM memory --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef USM_ALLOCATOR
#define USM_ALLOCATOR

#include <atomic>
#include <memory>

#include "../uma_helpers.hpp"

namespace usm {

// Configuration for specific USM allocator instance
class DisjointPoolConfig {
  public:
    DisjointPoolConfig();

    std::string name = "";

    struct SharedLimits {
        SharedLimits() : TotalSize(0) {}

        // Maximum memory left unfreed
        size_t MaxSize = 16 * 1024 * 1024;

        // Total size of pooled memory
        std::atomic<size_t> TotalSize;
    };

    // Minimum allocation size that will be requested from the system.
    // By default this is the minimum allocation size of each memory type.
    size_t SlabMinSize = 0;

    // Allocations up to this limit will be subject to chunking/pooling
    size_t MaxPoolableSize = 0;

    // When pooling, each bucket will hold a max of 4 unfreed slabs
    size_t Capacity = 0;

    // Holds the minimum bucket size valid for allocation of a memory type.
    size_t MinBucketSize = 0;

    // Holds size of the pool managed by the allocator.
    size_t CurPoolSize = 0;

    // Whether to print pool usage statistics
    int PoolTrace = 0;

    std::shared_ptr<SharedLimits> limits;
};

class DisjointPool {
  public:
    class AllocImpl;
    using Config = DisjointPoolConfig;

    uma_result_t initialize(uma_memory_provider_handle_t *providers,
                            size_t numProviders,
                            DisjointPoolConfig parameters) noexcept;
    void *malloc(size_t size) noexcept;
    void *calloc(size_t, size_t) noexcept;
    void *realloc(void *, size_t) noexcept;
    void *aligned_malloc(size_t size, size_t alignment) noexcept;
    size_t malloc_usable_size(void *) noexcept;
    void free(void *ptr) noexcept;
    enum uma_result_t get_last_result(const char **ppMessage) noexcept;

    DisjointPool();
    ~DisjointPool();

  private:
    std::unique_ptr<AllocImpl> impl;
};

} // namespace usm

#endif
