#pragma once
#include <cuda.h>
#include <string>
#include <vector>
#include <napalm/napalm.h>
#include <sstream>

namespace napalm
{
    namespace cuda
    {
        //std::string getPlatformInfo(const std::vector<cl_platform_id> & platform_ids,
        //    int platform_id, int info_name);
        //
        //std::string getDevInfo(const std::vector<cl_device_id> & dev_ids,
        //    int dev_id, int info_name);

        void handleError(CUresult err, const std::string & msg);

        //int32_t getCLMemFlag(MemFlag mem_flag);
        //
        //int32_t getCLMapFlag(MapMode map_mode);

        //cl_channel_type getCLDataType(DataType data_type);
        //cl_channel_order getCLChannelOrder(ImgChannelFormat channel_format);
        
        CUarray_format getCUDAImageForamt(ImgFormat format, unsigned int & num_channels, unsigned int & flags, int & bytes_per_channel);

        std::string loadFile(const char * fname, bool binary);

        template<typename Out>
        void split(const std::string &s, char delim, Out result) {
            std::stringstream ss(s);
            std::string item;
            while (std::getline(ss, item, delim)) {
                *(result++) = item;
            }
        }

        std::vector<std::string> split(const std::string &s, char delim);
        std::string replace_all(std::string str, const std::string& from, const std::string& to);

    }
}