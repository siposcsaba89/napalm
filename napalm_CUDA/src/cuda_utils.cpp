#include "cuda_utils.h"
#include <assert.h>
#include <string>
#include <fstream>
#include <streambuf>

//std::string napalm::cuda::getPlatformInfo(const std::vector<cl_platform_id>& platform_ids, int platform_id, int info_name)
//{
//    size_t info_size = 0;
//    clGetPlatformInfo(platform_ids[platform_id], info_name, 0, nullptr, &info_size);
//    std::string str;
//    str.resize(info_size);
//    clGetPlatformInfo(platform_ids[platform_id], info_name, info_size, &str[0], nullptr);
//    return std::move(str);
//}
//
//std::string napalm::cuda::getDevInfo(const std::vector<cl_device_id>& dev_ids, int dev_id, int info_name)
//{
//    size_t info_size = 0;
//    clGetDeviceInfo(dev_ids[dev_id], info_name, 0, nullptr, &info_size);
//    std::string str;
//    str.resize(info_size);
//    clGetDeviceInfo(dev_ids[dev_id], info_name, info_size, &str[0], nullptr);
//    return std::move(str);
//}

void napalm::cuda::handleError(CUresult err, const std::string & msg)
{
    if (err != CUDA_SUCCESS)
    {
        const char * err_val = 0;
        cuGetErrorName(err, &err_val);
        printf("[CUDA Error] %s %d = %s \n", msg.c_str(), int(err), err_val);
        assert(false && "[CUDA Error]");
        exit(EXIT_FAILURE);
    }

}

CUarray_format napalm::cuda::getCUDAImageForamt(ImgFormat format, unsigned int & num_channels, unsigned int & flags, int & bytes_per_channel)
{
    flags = 0;
    CUarray_format res;
    auto data_type = format.data_type;
    switch (data_type)
    {
    case napalm::DATA_TYPE_FLOAT:
        res = CU_AD_FORMAT_FLOAT;
        bytes_per_channel = 4;
        break;
    case napalm::DATA_TYPE_HALF_FLOAT:
        res = CU_AD_FORMAT_HALF;
        bytes_per_channel = 2;
        break;
    case napalm::DATA_TYPE_SNORM_INT8:
        res = CU_AD_FORMAT_SIGNED_INT8;
        bytes_per_channel = 1;
        break;
    case napalm::DATA_TYPE_SNORM_INT16:
        res = CU_AD_FORMAT_SIGNED_INT16;
        bytes_per_channel = 2;
        break;
    case napalm::DATA_TYPE_UNORM_INT8:
        res = CU_AD_FORMAT_UNSIGNED_INT8;
        bytes_per_channel = 1;
        break;
    case napalm::DATA_TYPE_UNORM_INT16:
        res = CU_AD_FORMAT_UNSIGNED_INT16;
        bytes_per_channel = 2;
        break;
    case napalm::DATA_TYPE_UNORM_INT24:
        assert(false && "Not supported image data type!");
        throw std::runtime_error("Not supported image data type!");
        break;
    case napalm::DATA_TYPE_SIGNED_INT8:
        res = CU_AD_FORMAT_SIGNED_INT8;
        bytes_per_channel = 1;
        break;
    case napalm::DATA_TYPE_SIGNED_INT16:
        res = CU_AD_FORMAT_SIGNED_INT16;
        bytes_per_channel = 2;
        break;
    case napalm::DATA_TYPE_SIGNED_INT32:
        res = CU_AD_FORMAT_SIGNED_INT32;
        bytes_per_channel = 4;
        break;
    case napalm::DATA_TYPE_UNSIGNED_INT8:
        res = CU_AD_FORMAT_UNSIGNED_INT8;
        bytes_per_channel = 1;
        break;
    case napalm::DATA_TYPE_UNSIGNED_INT16:
        res = CU_AD_FORMAT_UNSIGNED_INT16;
        bytes_per_channel = 2;
        break;
    case napalm::DATA_TYPE_UNSIGNED_INT32:
        res = CU_AD_FORMAT_UNSIGNED_INT32;
        bytes_per_channel = 4;
        break;
    default:
        break;
    }
    auto chf = format.img_channel_format;
    switch (chf)
    {
    case napalm::IMG_CHANNEL_FORMAT_INTENSITY:
        num_channels = 1;
        break;
    case napalm::IMG_CHANNEL_FORMAT_R:
        num_channels = 1;
        break;
    case napalm::IMG_CHANNEL_FORMAT_A:
        num_channels = 1;
        break;
    case napalm::IMG_CHANNEL_FORMAT_RG:
        num_channels = 2;
        break;
    case napalm::IMG_CHANNEL_FORMAT_RA:
        num_channels = 2;
        break;
    case napalm::IMG_CHANNEL_FORMAT_RGBA:
        num_channels = 4;
        break;
    default:
        break;
    }
    return res;
}

std::string napalm::cuda::loadFile(const char * fname, bool binary)
{
    std::ifstream t(fname, binary ? std::ios::binary : std::ios::in);
    if (!t.is_open())
    {
        printf("OpenCL loadFile: Cannot open file %s \n", fname);
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(t)),
        std::istreambuf_iterator<char>());
}

std::vector<std::string> napalm::cuda::split(const std::string & s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

std::string napalm::cuda::replace_all(std::string str, const std::string & from, const std::string & to)
{
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}
