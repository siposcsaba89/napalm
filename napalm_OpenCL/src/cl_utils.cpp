#include "cl_utils.h"
#include <assert.h>
#include <string>
#include <fstream>
#include <streambuf>

std::string napalm::cl::getPlatformInfo(const std::vector<cl_platform_id>& platform_ids, int platform_id, int info_name)
{
    size_t info_size = 0;
    clGetPlatformInfo(platform_ids[platform_id], info_name, 0, nullptr, &info_size);
    std::string str;
    str.resize(info_size);
    clGetPlatformInfo(platform_ids[platform_id], info_name, info_size, &str[0], nullptr);
    return std::move(str);
}

std::string napalm::cl::getDevInfo(const std::vector<cl_device_id>& dev_ids, int dev_id, int info_name)
{
    size_t info_size = 0;
    clGetDeviceInfo(dev_ids[dev_id], info_name, 0, nullptr, &info_size);
    std::string str;
    str.resize(info_size);
    clGetDeviceInfo(dev_ids[dev_id], info_name, info_size, &str[0], nullptr);
    return std::move(str);
}

void napalm::cl::handleError(cl_int err, const std::string & msg)
{
    if (err != CL_SUCCESS)
    {
        printf("[OpenCL Error] %s %d \n", msg.c_str(), err);
        assert(false && "[OpenCL Error]");
        exit(EXIT_FAILURE);
    }

}

int32_t napalm::cl::getCLMemFlag(MemFlag mem_flag)
{
    int32_t ret = -1;
    switch (mem_flag)
    {
    case napalm::MEM_FLAG_READ_WRITE:
        ret = CL_MEM_READ_WRITE;
        break;
    case napalm::MEM_FLAG_WITE_ONLY:
        ret = CL_MEM_WRITE_ONLY;
        break;
    case napalm::MEM_FLAG_READ_ONLY:
        ret = CL_MEM_READ_ONLY;
        break;
    case napalm::MEM_FLAG_USE_HOST_PTR:
        ret = CL_MEM_USE_HOST_PTR;
        break;
    case napalm::MEM_FLAG_ALLOC_HOST_PTR:
        ret = CL_MEM_ALLOC_HOST_PTR;
        break;
    case napalm::MEM_FLAG_COPY_HOST_PTR:
        ret = CL_MEM_COPY_HOST_PTR;
        break;
    default:
        assert(false && "Unknown OpenCL MemFlag!");
        break;
    }
    return ret;
}

int32_t napalm::cl::getCLMapFlag(MapMode map_mode)
{
    int32_t ret = -1;
    switch (map_mode)
    {
    case napalm::MAP_MODE_READ:
        ret = CL_MAP_READ;
        break;
    case napalm::MAP_MODE_WRITE:
        ret = CL_MAP_WRITE;
        break;
    case napalm::MAP_MODE_WRITE_INVALIDATE_REGION:
        ret = CL_MAP_WRITE_INVALIDATE_REGION;
        break;
    default:
        assert(false && "Invalid Map flag!!!");
        break;
    }
    return int32_t();
}

cl_channel_type napalm::cl::getCLDataType(DataType data_type)
{
    cl_channel_type ret;

    switch (data_type)
    {
    case napalm::DATA_TYPE_FLOAT:
        ret = CL_FLOAT;
        break;
    case napalm::DATA_TYPE_HALF_FLOAT:
        ret = CL_HALF_FLOAT;
        break;
    case napalm::DATA_TYPE_SNORM_INT8:
        ret = CL_SNORM_INT8;
        break;
    case napalm::DATA_TYPE_SNORM_INT16:
        ret = CL_SNORM_INT16;
        break;
    case napalm::DATA_TYPE_UNORM_INT8:
        ret = CL_UNORM_INT8;
        break;
    case napalm::DATA_TYPE_UNORM_INT16:
        ret = CL_UNORM_INT16;
        break;
    case napalm::DATA_TYPE_UNORM_INT24:
        ret = CL_UNORM_INT24;
        break;
    case napalm::DATA_TYPE_SIGNED_INT8:
        ret = CL_SIGNED_INT8;
        break;
    case napalm::DATA_TYPE_SIGNED_INT16:
        ret = CL_SIGNED_INT16;
        break;
    case napalm::DATA_TYPE_SIGNED_INT32:
        ret = CL_SIGNED_INT32;
        break;
    case napalm::DATA_TYPE_UNSIGNED_INT8:
        ret = CL_UNSIGNED_INT8;
        break;
    case napalm::DATA_TYPE_UNSIGNED_INT16:
        ret = CL_UNSIGNED_INT16;
        break;
    case napalm::DATA_TYPE_UNSIGNED_INT32:
        ret = CL_UNSIGNED_INT32;
        break;
    default:
        assert(false && "Unknown cl Data Type");
        break;
    }

    return ret;
}

cl_channel_order napalm::cl::getCLChannelOrder(ImgChannelFormat channel_format)
{
    cl_channel_order ret;

    switch (channel_format)
    {
    case napalm::IMG_CHANNEL_FORMAT_INTENSITY:
        ret = CL_INTENSITY;
        break;
    case napalm::IMG_CHANNEL_FORMAT_R:
        ret = CL_R;
        break;
    case napalm::IMG_CHANNEL_FORMAT_A:
        ret = CL_A;
        break;
    case napalm::IMG_CHANNEL_FORMAT_RG:
        ret = CL_RG;
        break;
    case napalm::IMG_CHANNEL_FORMAT_RA:
        ret = CL_RA;
        break;
    case napalm::IMG_CHANNEL_FORMAT_RGBA:
        ret = CL_RGBA;
        break;
    default:
        assert(false && "Unknown image channel format!");
        break;
    }

    return ret;
}


void napalm::cl::getCLImageForamt(ImgFormat format, ImgRegion size, cl_image_format & cl_img_f, cl_image_desc & cl_img_d)
{
    assert(size.x > 0 && size.y > 0 && size.z > 0 && "Invalid image size");
    cl_img_f.image_channel_data_type = getCLDataType(format.data_type);
    cl_img_f.image_channel_order = getCLChannelOrder(format.img_channel_format);
    memset(&cl_img_d, 0, sizeof(cl_image_desc));
    cl_img_d.image_width = size.x;
    cl_img_d.image_height = size.y;
    cl_img_d.image_depth = size.z;
    if (size.z == 1) cl_img_d.image_type = CL_MEM_OBJECT_IMAGE2D;
    else if (size.z > 1) cl_img_d.image_type = CL_MEM_OBJECT_IMAGE3D;
    else assert(false && "Invalid image size");
    cl_img_d.image_array_size = 0;
    cl_img_d.image_slice_pitch = 0;
    cl_img_d.num_mip_levels = 0;
    cl_img_d.num_samples = 0;
}

std::string napalm::cl::loadFile(const char * fname, bool binary)
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
