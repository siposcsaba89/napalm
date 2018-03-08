#pragma once
#include <CL/cl.h>
#include <string>
#include <vector>
#include <napalm/napalm.h>

namespace napalm
{
    namespace cl
    {
        std::string getPlatformInfo(const std::vector<cl_platform_id> & platform_ids,
            int platform_id, int info_name);

        std::string getPlatformInfo(cl_platform_id platform_id, int info_name);

        std::string getDevInfo(const std::vector<cl_device_id> & dev_ids,
            int dev_id, int info_name);

        std::string getDevInfo(cl_device_id dev_id, int info_name);


        void handleError(cl_int err, const std::string & msg);

        int32_t getCLMemFlag(MemFlag mem_flag);

        int32_t getCLMapFlag(MapMode map_mode);

        cl_channel_type getCLDataType(DataType data_type);
        cl_channel_order getCLChannelOrder(ImgChannelFormat channel_format);

        void getCLImageForamt(ImgFormat format, ImgRegion size,
            cl_image_format &  cl_img_f, cl_image_desc & cl_img_d);

        std::string loadFile(const char * fname, bool binary);
    }
}