#pragma once
#include <napalm/cuda/napalm_CUDA_export.h>
#include <napalm/napalm.h>
extern "C" {
    NAPALM_CUDA_EXPORT napalm::Context * createContextCUDA(int32_t platform_id, int32_t device_id, int32_t stream_count, napalm::GLSharedInfo * window_data);

    NAPALM_CUDA_EXPORT napalm::PlatformAndDeviceInfo * getPlatformAndDeviceInfoCUDA();
    NAPALM_CUDA_EXPORT void destroyPlatformAndDeviceInfoCUDA(napalm::PlatformAndDeviceInfo* dev_info);
}
