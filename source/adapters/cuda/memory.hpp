//===--------- memory.hpp - CUDA Adapter ----------------------------------===//
//
// Copyright (C) 2023 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#pragma once

#include <cassert>
#include <cuda.h>
#include <ur_api.h>
#include <variant>

#include "common.hpp"

// Handler for plain, pointer-based CUDA allocations
struct BufferMem {

  struct BufferMap {
    /// Size of the active mapped region.
    size_t MapSize;
    /// Offset of the active mapped region.
    size_t MapOffset;
    /// Original flags for the mapped region
    ur_map_flags_t MapFlags;
    /// Allocated host memory used exclusively for this map.
    std::unique_ptr<unsigned char[]> MapMem;

    BufferMap(size_t MapSize, size_t MapOffset, ur_map_flags_t MapFlags)
        : MapSize(MapSize), MapOffset(MapOffset), MapFlags(MapFlags),
          MapMem(nullptr) {}

    BufferMap(size_t MapSize, size_t MapOffset, ur_map_flags_t MapFlags,
              std::unique_ptr<unsigned char[]> &&MapMem)
        : MapSize(MapSize), MapOffset(MapOffset), MapFlags(MapFlags),
          MapMem(std::move(MapMem)) {}

    size_t getMapSize() const noexcept { return MapSize; }

    size_t getMapOffset() const noexcept { return MapOffset; }

    ur_map_flags_t getMapFlags() const noexcept { return MapFlags; }
  };

  /** AllocMode
   * classic: Just a normal buffer allocated on the device via cuda malloc
   * use_host_ptr: Use an address on the host for the device
   * copy_in: The data for the device comes from the host but the host pointer
   * is not available later for re-use alloc_host_ptr: Uses pinned-memory
   * allocation
   */
  enum class AllocMode {
    Classic,
    UseHostPtr,
    CopyIn,
    AllocHostPtr,
  };

  using native_type = CUdeviceptr;

  /// If this allocation is a sub-buffer (i.e., a view on an existing
  /// allocation), this is the pointer to the parent handler structure
  ur_mem_handle_t Parent;
  /// CUDA handler for the pointer
  native_type Ptr;
  /// Pointer associated with this device on the host
  void *HostPtr;
  /// Size of the allocation in bytes
  size_t Size;
  /// A map that contains all the active mappings for this buffer.
  std::unordered_map<void *, BufferMap> PtrToBufferMap;

  AllocMode MemAllocMode;

  BufferMem(ur_mem_handle_t Parent, BufferMem::AllocMode Mode, CUdeviceptr Ptr,
            void *HostPtr, size_t Size)
      : Parent{Parent}, Ptr{Ptr}, HostPtr{HostPtr}, Size{Size},
        PtrToBufferMap{}, MemAllocMode{Mode} {};

  native_type get() const noexcept { return Ptr; }

  size_t getSize() const noexcept { return Size; }

  BufferMap *getMapDetails(void *Map) {
    auto details = PtrToBufferMap.find(Map);
    if (details != PtrToBufferMap.end()) {
      return &details->second;
    }
    return nullptr;
  }

  /// Returns a pointer to data visible on the host that contains
  /// the data on the device associated with this allocation.
  /// The offset is used to index into the CUDA allocation.
  void *mapToPtr(size_t MapSize, size_t MapOffset,
                 ur_map_flags_t MapFlags) noexcept {

    void *MapPtr = nullptr;
    if (HostPtr == nullptr) {
      /// If HostPtr is invalid, we need to create a Mapping that owns its own
      /// memory on the host.
      auto MapMem = std::make_unique<unsigned char[]>(MapSize);
      MapPtr = MapMem.get();
      PtrToBufferMap.insert(
          {MapPtr, BufferMap(MapSize, MapOffset, MapFlags, std::move(MapMem))});
    } else {
      /// However, if HostPtr already has valid memory (e.g. pinned allocation),
      /// we can just use that memory for the mapping.
      MapPtr = static_cast<char *>(HostPtr) + MapOffset;
      PtrToBufferMap.insert({MapPtr, BufferMap(MapSize, MapOffset, MapFlags)});
    }
    return MapPtr;
  }

  /// Detach the allocation from the host memory.
  void unmap(void *MapPtr) noexcept {
    assert(MapPtr != nullptr);
    PtrToBufferMap.erase(MapPtr);
  }
};

// Handler data for surface object (i.e. Images)
struct SurfaceMem {
  CUarray Array;
  CUsurfObject SurfObj;
  ur_mem_type_t ImageType;

  SurfaceMem(CUarray Array, CUsurfObject Surf, ur_mem_type_t ImageType,
             void *HostPtr)
      : Array{Array}, SurfObj{Surf}, ImageType{ImageType} {
    (void)HostPtr;
  }

  CUarray getArray() const noexcept { return Array; }

  CUsurfObject getSurface() const noexcept { return SurfObj; }

  ur_mem_type_t getImageType() const noexcept { return ImageType; }
};

// For sampled/unsampled images
struct ImageMem {
  CUarray Array;
  void *Handle;
  ur_mem_type_t ImageType;
  ur_sampler_handle_t Sampler;

  ImageMem(CUarray Array, void *Handle, ur_mem_type_t ImageType,
           ur_sampler_handle_t Sampler)
      : Array{Array}, Handle{Handle}, ImageType{ImageType}, Sampler{Sampler} {};

  CUarray get_array() const noexcept { return Array; }

  void *get_handle() const noexcept { return Handle; }

  ur_mem_type_t get_image_type() const noexcept { return ImageType; }

  ur_sampler_handle_t get_sampler() const noexcept { return Sampler; }
};

/// UR Mem mapping to CUDA memory allocations, both data and texture/surface.
/// \brief Represents non-SVM allocations on the CUDA backend.
/// Keeps tracks of all mapped regions used for Map/Unmap calls.
/// Only one region can be active at the same time per allocation.
struct ur_mem_handle_t_ {
  // Context where the memory object is accessible
  ur_context_handle_t Context;

  /// Reference counting of the handler
  std::atomic_uint32_t RefCount;
  enum class Type { Buffer, Surface, Texture } MemType;

  // Original mem flags passed
  ur_mem_flags_t MemFlags;

  /// A UR Memory object represents either plain memory allocations ("Buffers"
  /// in OpenCL) or typed allocations ("Images" in OpenCL).
  /// In CUDA their API handlers are different. Whereas "Buffers" are allocated
  /// as pointer-like structs, "Images" are stored in Textures or Surfaces.
  /// This union allows implementation to use either from the same handler.
  std::variant<BufferMem, SurfaceMem, ImageMem> Mem;

  /// Constructs the UR mem handler for a non-typed allocation ("buffer")
  ur_mem_handle_t_(ur_context_handle_t Context, ur_mem_handle_t Parent,
                   ur_mem_flags_t MemFlags, BufferMem::AllocMode Mode,
                   CUdeviceptr Ptr, void *HostPtr, size_t Size)
      : Context{Context}, RefCount{1}, MemType{Type::Buffer},
        MemFlags{MemFlags}, Mem{BufferMem{Parent, Mode, Ptr, HostPtr, Size}} {
    if (isSubBuffer()) {
      urMemRetain(std::get<BufferMem>(Mem).Parent);
    } else {
      urContextRetain(Context);
    }
  };

  /// Constructs the UR allocation for an Image object (surface in CUDA)
  ur_mem_handle_t_(ur_context_handle_t Context, CUarray Array,
                   CUsurfObject Surf, ur_mem_flags_t MemFlags,
                   ur_mem_type_t ImageType, void *HostPtr)
      : Context{Context}, RefCount{1}, MemType{Type::Surface},
        MemFlags{MemFlags}, Mem{SurfaceMem{Array, Surf, ImageType, HostPtr}} {
    urContextRetain(Context);
  }

  /// Constructs the UR allocation for an unsampled image object
  ur_mem_handle_t_(ur_context_handle_t Context, CUarray Array,
                   CUsurfObject Surf, ur_mem_type_t ImageType)
      : Context{Context}, RefCount{1}, MemType{Type::Surface}, MemFlags{0},
        Mem{ImageMem{Array, (void *)Surf, ImageType, nullptr}} {
    urContextRetain(Context);
  }

  /// Constructs the UR allocation for a sampled image object
  ur_mem_handle_t_(ur_context_handle_t Context, CUarray Array, CUtexObject Tex,
                   ur_sampler_handle_t Sampler, ur_mem_type_t ImageType)
      : Context{Context}, RefCount{1}, MemType{Type::Texture}, MemFlags{0},
        Mem{ImageMem{Array, (void *)Tex, ImageType, Sampler}} {
    urContextRetain(Context);
  }

  ~ur_mem_handle_t_() {
    if (isBuffer() && isSubBuffer()) {
      urMemRelease(std::get<BufferMem>(Mem).Parent);
      return;
    }
    urContextRelease(Context);
  }

  bool isBuffer() const noexcept { return MemType == Type::Buffer; }

  bool isSubBuffer() const noexcept {
    return (isBuffer() && (std::get<BufferMem>(Mem).Parent != nullptr));
  }

  bool isImage() const noexcept { return MemType == Type::Surface; }

  ur_context_handle_t getContext() const noexcept { return Context; }

  uint32_t incrementReferenceCount() noexcept { return ++RefCount; }

  uint32_t decrementReferenceCount() noexcept { return --RefCount; }

  uint32_t getReferenceCount() const noexcept { return RefCount; }
};
