#pragma once
#include <napalm/cuda/napalm_CUDA_export.h>
#include <napalm/napalm.h>
//namespace napalm {
//    namespace cuda
extern "C" {
    NAPALM_CUDA_EXPORT napalm::Context * createContext(int32_t platform_id, int32_t device_id, int32_t stream_count);

    NAPALM_CUDA_EXPORT napalm::PlatformAndDeviceInfo * getPlatformAndDeviceInfo();
    NAPALM_CUDA_EXPORT void destroyPlatformAndDeviceInfo(napalm::PlatformAndDeviceInfo* dev_info);
}
  //  }
//}



