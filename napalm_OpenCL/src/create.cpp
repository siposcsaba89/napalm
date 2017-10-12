#include <napalm/opencl/create.h>
#include <napalm/opencl/opencl_context.h>
#include "cl_utils.h"

NAPALM_OPENCL_EXPORT napalm::Context * createContext(int32_t platform_id, int32_t device_id, int32_t stream_count)
{
    return new napalm::cl::OpenCLContext(platform_id, device_id, stream_count);
}

NAPALM_OPENCL_EXPORT napalm::PlatformAndDeviceInfo * getPlatformAndDeviceInfo()
{
    napalm::PlatformAndDeviceInfo * ret2 = new napalm::PlatformAndDeviceInfo();
    napalm::PlatformAndDeviceInfo & ret = *ret2;
    cl_uint num_platform;
    clGetPlatformIDs(0, NULL, &num_platform);
    if (num_platform > 0)
    {
        ret.num_platforms = int32_t(num_platform);
        ret.platforms = new char*[num_platform];
        ret.num_devices = new int32_t[num_platform];
        ret.device_names = new char**[num_platform];

        std::vector<cl_platform_id> platform_ids(num_platform);
        cl_int err = clGetPlatformIDs(num_platform, platform_ids.data(), nullptr);
        napalm::cl::handleError(err, "CL Get Platform list");
        for (size_t i = 0; i < platform_ids.size(); ++i)
        {
            std::string platform_name =
                napalm::cl::getPlatformInfo(platform_ids, int(i), CL_PLATFORM_VENDOR).c_str() + std::string(" Version:") +
                napalm::cl::getPlatformInfo(platform_ids, int(i), CL_PLATFORM_VERSION);
            ret.platforms[i] = new char[platform_name.size()];
            strcpy_s(ret.platforms[i], platform_name.size(), platform_name.c_str());
            cl_uint num_devices;
            clGetDeviceIDs(platform_ids[i], CL_DEVICE_TYPE_ALL, 0, nullptr, &num_devices);
            ret.num_devices[i] = int32_t(num_devices);
            if (num_devices > 0)
            {
                std::vector<cl_device_id> devices(num_devices);
                clGetDeviceIDs(platform_ids[i], CL_DEVICE_TYPE_ALL, num_devices, devices.data(), nullptr);
                ret.device_names[i] = new char*[num_devices];
                for (size_t j = 0; j < devices.size(); ++j)
                {
                    std::string dev_name = napalm::cl::getDevInfo(devices, int(j), CL_DEVICE_NAME);
                    ret.device_names[i][j] = new char[dev_name.size()];
                    strcpy_s(ret.device_names[i][j], dev_name.size(), dev_name.c_str());
                }
            }
        }
        
    }
    return ret2;
}

NAPALM_OPENCL_EXPORT void destroyPlatformAndDeviceInfo(napalm::PlatformAndDeviceInfo * dev_info)
{
    return delete dev_info;
}
