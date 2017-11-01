#pragma once
#include <napalm/opencl/napalm_OpenCL_export.h>
#include <napalm/napalm.h>
extern "C" {
    NAPALM_OPENCL_EXPORT napalm::Context * createContext(int32_t platform_id, int32_t device_id, int32_t stream_count);

    NAPALM_OPENCL_EXPORT napalm::PlatformAndDeviceInfo * getPlatformAndDeviceInfo();
    NAPALM_OPENCL_EXPORT void destroyPlatformAndDeviceInfo(napalm::PlatformAndDeviceInfo* dev_info);
}
namespace napalm {
    namespace cl{
        NAPALM_OPENCL_EXPORT PlatformAndDeviceInfo * getPlatformAndDeviceInfoOpenCL();
        NAPALM_OPENCL_EXPORT napalm::Context * createContextOpenCL(int32_t platform_id, int32_t device_id, int32_t stream_count);
  }
}



