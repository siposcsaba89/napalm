#include "napalm.h"
#include <string>
#include <napalm/opencl/create.h>


NAPALM_EXPORT napalm::PlatformAndDeviceInfo napalm::getPlatformAndDeviceInfo(const char * api_type)
{
    if (std::strcmp(api_type, "OpenCL") == 0)
        return napalm::cl::getPlatformAndDeviceInfo();
    return napalm::PlatformAndDeviceInfo();
}

napalm::Context* napalm::createContext(const char * kind, int32_t platform_id, int32_t device_id, int32_t stream_count)
{
    if (std::strcmp(kind, "OpenCL") == 0)
        return napalm::cl::createContext(platform_id, device_id, stream_count);

    return 0;
}

//napalm::PlatformAndDeviceInfo::PlatformAndDeviceInfo()
//{
//}
//
//napalm::PlatformAndDeviceInfo::PlatformAndDeviceInfo(const PlatformAndDeviceInfo & other)
//{
//    *this = other;
//}

//napalm::PlatformAndDeviceInfo & napalm::PlatformAndDeviceInfo::operator=(const PlatformAndDeviceInfo & other)
//{
//    for (int i = 0; i < num_platforms; ++i)
//    {
//        for (int j = 0; j < num_devices[i]; ++j)
//        {
//            delete[] device_names[i][j];
//        }
//        delete[] platforms[i];
//        delete[] device_names[i];
//    }
//    delete[] num_devices;
//    delete[] platforms;
//    delete[] device_names;
//    num_devices = nullptr;
//    platforms = nullptr;
//    device_names = nullptr;
//    num_platforms = other.num_platforms;
//    platforms = new char*[num_platforms];
//    device_names = new char**[num_platforms];
//    num_devices = new int32_t[num_platforms];
//    memcpy(num_devices, other.num_devices, num_platforms * sizeof(int32_t));
//    for (int i = 0; i < num_platforms; ++i)
//    {
//        device_names[i] = new char *[num_devices[i]];
//        platforms[i] = new char[strlen(other.platforms[i])];
//        strcpy(platforms[i], other.platforms[i]);
//        for (int j = 0; j < num_devices[i]; ++j)
//        {
//            device_names[i][j] = new char[strlen(other.device_names[i][j])];
//            strcpy(device_names[i][j], other.device_names[i][j]);
//        }
//    }
//    return *this;
//}

//napalm::PlatformAndDeviceInfo::~PlatformAndDeviceInfo()
//{
//    for (int i = 0; i < num_platforms; ++i)
//    {
//        for (int j = 0; j < num_devices[i]; ++j)
//        {
//            delete [] device_names[i][j];
//        }
//        delete[] platforms[i];
//        delete[] device_names[i];
//    }
//    delete[] num_devices;
//    delete[] platforms;
//    delete[] device_names;
//    num_devices = nullptr;
//    platforms = nullptr;
//    device_names = nullptr;
//    num_platforms = 0;
//}
