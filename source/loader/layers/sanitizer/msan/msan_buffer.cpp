/*
 *
 * Copyright (C) 2024 Intel Corporation
 *
 * Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
 * Exceptions. See LICENSE.TXT
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 * @file msan_buffer.cpp
 *
 */

#include "msan_buffer.hpp"
#include "msan_interceptor.hpp"
#include "sanitizer_common/sanitizer_utils.hpp"
#include "ur_sanitizer_layer.hpp"

namespace ur_sanitizer_layer {
namespace msan {

ur_result_t EnqueueMemCopyRectHelper(
    ur_queue_handle_t Queue, char *pSrc, char *pDst, ur_rect_offset_t SrcOffset,
    ur_rect_offset_t DstOffset, ur_rect_region_t Region, size_t SrcRowPitch,
    size_t SrcSlicePitch, size_t DstRowPitch, size_t DstSlicePitch,
    bool Blocking, uint32_t NumEventsInWaitList,
    const ur_event_handle_t *EventWaitList, ur_event_handle_t *Event) {
  // If user doesn't determine src/dst row pitch and slice pitch, just use
  // region for it.
  if (SrcRowPitch == 0) {
    SrcRowPitch = Region.width;
  }

  if (SrcSlicePitch == 0) {
    SrcSlicePitch = SrcRowPitch * Region.height;
  }

  if (DstRowPitch == 0) {
    DstRowPitch = Region.width;
  }

  if (DstSlicePitch == 0) {
    DstSlicePitch = DstRowPitch * Region.height;
  }

  // Calculate the src and dst addresses that actually will be copied.
  char *SrcOrigin = pSrc + SrcOffset.x + SrcRowPitch * SrcOffset.y +
                    SrcSlicePitch * SrcOffset.z;
  char *DstOrigin = pDst + DstOffset.x + DstRowPitch * DstOffset.y +
                    DstSlicePitch * DstOffset.z;

  const bool IsDstDeviceUSM =
      getMsanInterceptor()->findAllocInfoByAddress((uptr)DstOrigin).has_value();
  const bool IsSrcDeviceUSM =
      getMsanInterceptor()->findAllocInfoByAddress((uptr)SrcOrigin).has_value();

  ur_device_handle_t Device = GetDevice(Queue);
  std::shared_ptr<DeviceInfo> DeviceInfo =
      getMsanInterceptor()->getDeviceInfo(Device);
  std::vector<ur_event_handle_t> Events;

  // For now, USM doesn't support 3D memory copy operation, so we can only
  // loop call 2D memory copy function to implement it.
  for (size_t i = 0; i < Region.depth; i++) {
    ur_event_handle_t NewEvent{};
    UR_CALL(getContext()->urDdiTable.Enqueue.pfnUSMMemcpy2D(
        Queue, false, DstOrigin + (i * DstSlicePitch), DstRowPitch,
        SrcOrigin + (i * SrcSlicePitch), SrcRowPitch, Region.width,
        Region.height, NumEventsInWaitList, EventWaitList, &NewEvent));
    Events.push_back(NewEvent);

    // Update shadow memory
    if (IsDstDeviceUSM && IsSrcDeviceUSM) {
      NewEvent = nullptr;
      uptr DstShadowAddr = DeviceInfo->Shadow->MemToShadow((uptr)DstOrigin +
                                                           (i * DstSlicePitch));
      uptr SrcShadowAddr = DeviceInfo->Shadow->MemToShadow((uptr)SrcOrigin +
                                                           (i * SrcSlicePitch));
      UR_CALL(getContext()->urDdiTable.Enqueue.pfnUSMMemcpy2D(
          Queue, false, (void *)DstShadowAddr, DstRowPitch,
          (void *)SrcShadowAddr, SrcRowPitch, Region.width, Region.height,
          NumEventsInWaitList, EventWaitList, &NewEvent));
      Events.push_back(NewEvent);
    } else if (IsDstDeviceUSM && !IsSrcDeviceUSM) {
      uptr DstShadowAddr = DeviceInfo->Shadow->MemToShadow((uptr)DstOrigin +
                                                           (i * DstSlicePitch));
      const char Val = 0;
      // opencl & l0 adapter doesn't implement urEnqueueUSMFill2D, so
      // emulate the operation with urEnqueueUSMFill.
      for (size_t HeightIndex = 0; HeightIndex < Region.height; HeightIndex++) {
        NewEvent = nullptr;
        UR_CALL(getContext()->urDdiTable.Enqueue.pfnUSMFill(
            Queue, (void *)(DstShadowAddr + HeightIndex * DstRowPitch), 1, &Val,
            Region.width, NumEventsInWaitList, EventWaitList, &NewEvent));
        Events.push_back(NewEvent);
      }
    }
  }

  if (Blocking) {
    UR_CALL(getContext()->urDdiTable.Event.pfnWait(Events.size(), &Events[0]));
  }

  if (Event) {
    UR_CALL(getContext()->urDdiTable.Enqueue.pfnEventsWait(Queue, Events.size(),
                                                           &Events[0], Event));
  }

  return UR_RESULT_SUCCESS;
}

ur_result_t MemBuffer::getHandle(ur_device_handle_t Device, char *&Handle) {
  // Sub-buffers don't maintain own allocations but rely on parent buffer.
  if (SubBuffer) {
    UR_CALL(SubBuffer->Parent->getHandle(Device, Handle));
    Handle += SubBuffer->Origin;
    return UR_RESULT_SUCCESS;
  }

  // Device may be null, we follow the L0 adapter's practice to use the first
  // device
  if (!Device) {
    auto Devices = GetDevices(Context);
    assert(Devices.size() > 0 && "Devices should not be empty");
    Device = Devices[0];
  }
  assert((void *)Device != nullptr && "Device cannot be nullptr");

  std::scoped_lock<ur_shared_mutex> Guard(Mutex);
  auto &Allocation = Allocations[Device];
  ur_result_t URes = UR_RESULT_SUCCESS;
  if (!Allocation) {
    ur_usm_desc_t USMDesc{};
    USMDesc.align = getAlignment();
    ur_usm_pool_handle_t Pool{};
    URes = getMsanInterceptor()->allocateMemory(Context, Device, &USMDesc, Pool,
                                                Size, AllocType::DEVICE_USM,
                                                ur_cast<void **>(&Allocation));
    if (URes != UR_RESULT_SUCCESS) {
      URLOG_CTX(ERR, "Failed to allocate {} bytes memory for buffer {}", Size,
                this);
      return URes;
    }

    if (HostPtr) {
      ManagedQueue Queue(Context, Device);
      URes = getContext()->urDdiTable.Enqueue.pfnUSMMemcpy(
          Queue, true, Allocation, HostPtr, Size, 0, nullptr, nullptr);
      if (URes != UR_RESULT_SUCCESS) {
        URLOG_CTX(ERR,
                  "Failed to copy {} bytes data from host "
                  "pointer {} to buffer {}",
                  Size, HostPtr, this);
        return URes;
      }

      // Update shadow memory
      std::shared_ptr<DeviceInfo> DeviceInfo =
          getMsanInterceptor()->getDeviceInfo(Device);
      UR_CALL(DeviceInfo->Shadow->EnqueuePoisonShadow(Queue, (uptr)Allocation,
                                                      Size, 0));
    }
  }

  Handle = Allocation;

  if (!LastSyncedDevice.hDevice) {
    LastSyncedDevice = MemBuffer::Device_t{Device, Handle};
    return URes;
  }

  // If the device required to allocate memory is not the previous one, we
  // need to do data migration.
  if (Device != LastSyncedDevice.hDevice) {
    auto &HostAllocation = Allocations[nullptr];
    if (!HostAllocation) {
      ur_usm_desc_t USMDesc{};
      USMDesc.align = getAlignment();
      ur_usm_pool_handle_t Pool{};
      URes = getContext()->urDdiTable.USM.pfnHostAlloc(
          Context, &USMDesc, Pool, Size, ur_cast<void **>(&HostAllocation));
      if (URes != UR_RESULT_SUCCESS) {
        URLOG_CTX(ERR,
                  "Failed to allocate {} bytes host "
                  "USM for buffer {} migration",
                  Size, this);
        return URes;
      }
    }

    // Copy data from last synced device to host
    {
      ManagedQueue Queue(Context, LastSyncedDevice.hDevice);
      URes = getContext()->urDdiTable.Enqueue.pfnUSMMemcpy(
          Queue, true, HostAllocation, LastSyncedDevice.MemHandle, Size, 0,
          nullptr, nullptr);
      if (URes != UR_RESULT_SUCCESS) {
        URLOG_CTX(ERR, "Failed to migrate memory buffer data");
        return URes;
      }
    }

    // Sync data back to device
    {
      ManagedQueue Queue(Context, Device);
      URes = getContext()->urDdiTable.Enqueue.pfnUSMMemcpy(
          Queue, true, Allocation, HostAllocation, Size, 0, nullptr, nullptr);
      if (URes != UR_RESULT_SUCCESS) {
        URLOG_CTX(ERR, "Failed to migrate memory buffer data");
        return URes;
      }
    }
  }

  LastSyncedDevice = MemBuffer::Device_t{Device, Handle};

  return URes;
}

ur_result_t MemBuffer::free() {
  for (const auto &[_, Ptr] : Allocations) {
    ur_result_t URes = getContext()->urDdiTable.USM.pfnFree(Context, Ptr);
    if (URes != UR_RESULT_SUCCESS) {
      URLOG_CTX(ERR, "Failed to free buffer handle {}", Ptr);
      return URes;
    }
  }
  Allocations.clear();
  return UR_RESULT_SUCCESS;
}

size_t MemBuffer::getAlignment() {
  // Choose an alignment that is at most 128 and is the next power of 2
  // for sizes less than 128.
  // TODO: If we don't set the alignment size explicitly, the device will
  // usually choose a very large size (more than 1k). Then sanitizer will
  // allocate extra unnessary memory. Not sure if this will impact
  // performance.
  size_t MsbIdx = 63 - __builtin_clzl(Size);
  size_t Alignment = (1ULL << (MsbIdx + 1));
  if (Alignment > 128) {
    Alignment = 128;
  }
  return Alignment;
}

} // namespace msan
} // namespace ur_sanitizer_layer
