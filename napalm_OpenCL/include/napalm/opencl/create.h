#pragma once
#include <napalm/opencl/napalm_OpenCL_export.h>
#include <napalm/napalm.h>
namespace napalm {
    namespace cl
    {
        NAPALM_OPENCL_EXPORT Context * createContext(int32_t platform_id, int32_t device_id, int32_t stream_count);
        NAPALM_OPENCL_EXPORT PlatformAndDeviceInfo getPlatformAndDeviceInfo();
    }
}



