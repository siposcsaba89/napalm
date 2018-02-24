#pragma once
#include <napalm/opencl/napalm_OpenCL_export.h>
#include <napalm/napalm.h>
extern "C" {
    NAPALM_OPENCL_EXPORT napalm::Context * createContextOpenCL(int32_t platform_id, int32_t device_id, int32_t stream_count, napalm::GLSharedInfo * window_data);

    NAPALM_OPENCL_EXPORT napalm::PlatformAndDeviceInfo * getPlatformAndDeviceInfoOpenCL();
    NAPALM_OPENCL_EXPORT void destroyPlatformAndDeviceInfoOpenCL(napalm::PlatformAndDeviceInfo* dev_info);
}
