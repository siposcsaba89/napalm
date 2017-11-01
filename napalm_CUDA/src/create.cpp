#include <napalm/cuda/create.h>
#include <napalm/cuda/cuda_context.h>
#include "cuda_utils.h"
#include <cuda.h>
#include <cstring>
NAPALM_CUDA_EXPORT napalm::Context * createContextCUDA(int32_t platform_id, int32_t device_id, int32_t stream_count)
{
    return new napalm::cuda::CUDAContext(platform_id, device_id, stream_count);
}

NAPALM_CUDA_EXPORT napalm::PlatformAndDeviceInfo * getPlatformAndDeviceInfoCUDA()
{
    CUresult res = cuInit(0);
    napalm::cuda::handleError(res, "Cu init");
    napalm::PlatformAndDeviceInfo * ret2 = new napalm::PlatformAndDeviceInfo();
    napalm::PlatformAndDeviceInfo & ret = *ret2;
    int32_t num_platform = 1;
    {
        ret.num_platforms = num_platform;
        ret.platforms = new char*[num_platform];
        ret.num_devices = new int32_t[num_platform];
        ret.device_names = new char**[num_platform];

        {
            int version = 0;
            CUresult err = cuDriverGetVersion(&version);
            napalm::cuda::handleError(err, "Cuda get driver version");
            std::string platform_name =
                "NVidia" + std::string(" Version:") +
                std::to_string(version);
            ret.platforms[0] = new char[platform_name.size()];
            memcpy(ret.platforms[0], platform_name.c_str(), platform_name.size());

            int num_devices = 0;
            cuDeviceGetCount(&num_devices);
            ret.num_devices[0] = int32_t(num_devices);
            if (num_devices > 0)
            {
                ret.device_names[0] = new char*[num_devices];
                for (int j = 0; j < num_devices; ++j)
                {
                    std::string dev_name;
                    dev_name.resize(1024);
                    CUdevice dev;
                    CUresult res = cuDeviceGet(&dev, j);
                    napalm::cuda::handleError(res, "Get device");
                    res = cuDeviceGetName(&dev_name[0], int(dev_name.size()), dev);
                    ret.device_names[0][j] = new char[dev_name.size()];
#ifdef WIN32
                    strcpy_s(ret.device_names[0][j], dev_name.size(), dev_name.c_str());
#else
                    std::strcpy(ret.device_names[0][j], dev_name.c_str());
#endif
                }
            }
        }
        
    }
    return ret2;
}

NAPALM_CUDA_EXPORT void destroyPlatformAndDeviceInfoCUDA(napalm::PlatformAndDeviceInfo * dev_info)
{
    return delete dev_info;
}
