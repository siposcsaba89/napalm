#pragma once
#include <napalm/napalm_export.h>
#include <inttypes.h>

namespace napalm
{
    enum MapMode
    {
        MAP_MODE_READ = 0,
        MAP_MODE_WRITE = 1,
        MAP_MODE_WRITE_INVALIDATE_REGION = 2 /// The contents of the region being mapped are to be discarded. This is typically the case when the region being mapped is overwritten by the host
    };
    
    enum MemFlag
    {
        MEM_FLAG_READ_WRITE = 0,
        MEM_FLAG_WITE_ONLY = 1,
        MEM_FLAG_READ_ONLY = 2,
        MEM_FLAG_USE_HOST_PTR = 3,
        MEM_FLAG_ALLOC_HOST_PTR = 4,
        MEM_FLAG_COPY_HOST_PTR = 5,
        MEM_FLAG_HOST_WRITE_ONLY = 6,
        MEM_FLAG_HOST_READ_ONLY = 7,
        MEM_FLAG_HOST_NO_ACCESS = 8,

    };

    enum DataType
    {
        DATA_TYPE_FLOAT = 0,
        DATA_TYPE_HALF_FLOAT = 1,
        DATA_TYPE_SNORM_INT8 = 2,
        DATA_TYPE_SNORM_INT16 = 3,
        DATA_TYPE_UNORM_INT8 = 4,
        DATA_TYPE_UNORM_INT16 = 5,
        DATA_TYPE_UNORM_INT24 = 6,
        DATA_TYPE_SIGNED_INT8 = 7,
        DATA_TYPE_SIGNED_INT16 = 8,
        DATA_TYPE_SIGNED_INT32 = 9,
        DATA_TYPE_UNSIGNED_INT8 = 10,
        DATA_TYPE_UNSIGNED_INT16 = 11,
        DATA_TYPE_UNSIGNED_INT32 = 12,
    };

    enum ImgChannelFormat
    {
        IMG_CHANNEL_FORMAT_INTENSITY,
        IMG_CHANNEL_FORMAT_R,
        IMG_CHANNEL_FORMAT_A,
        IMG_CHANNEL_FORMAT_RG,
        IMG_CHANNEL_FORMAT_RA,
        IMG_CHANNEL_FORMAT_RGBA
    };

    struct ImgFormat
    {
        DataType data_type;
        ImgChannelFormat img_channel_format;
        ImgFormat(ImgChannelFormat img_format = IMG_CHANNEL_FORMAT_INTENSITY, 
            DataType img_data_type = DATA_TYPE_UNORM_INT8) : 
            data_type(img_data_type), img_channel_format(img_format){}
    };


    struct Buffer
    {
        virtual void write(const void * data, bool block_queue = true, int32_t command_queue = 0) = 0;
        virtual void write(const void * data, size_t offset, size_t size, bool block_queue = true, int32_t command_queue = 0) = 0;
        virtual void read(void * data, bool block_queue = true, int32_t command_queue = 0) const = 0;
        virtual void read(void * data, size_t offset, size_t size, bool block_queue = true, int32_t command_queue = 0) const = 0;
        virtual void * map(MapMode mode = MAP_MODE_READ, bool block_queue = true, int32_t command_queue = 0) = 0;
        virtual void * map(MapMode mode, size_t offset, size_t size, bool block_queue = true, int32_t command_queue = 0) = 0;
        virtual void unmap(int32_t command_queue = 0) = 0;
        virtual ~Buffer() {}
    public:
        size_t buff_size;
    };

    struct ImgRegion
    {
        ImgRegion(int32_t xx = 0, int32_t yy = 1, int32_t zz = 1) : x(xx), y(yy), z(zz) {}
        int32_t x = 0;
        int32_t y = 0;
        int32_t z = 0;
    };

    struct Img
    {
        virtual void write(const void * data, bool block_queue = true, int32_t command_queue = 0) = 0;
        virtual void write(const void * data, const ImgRegion & origin, 
            const ImgRegion & region, bool block_queue = true, int32_t command_queue = 0) = 0;
        virtual void read(void * data, bool block_queue = true, int32_t command_queue = 0) const = 0;
        virtual void read(void * data, const ImgRegion & origin,
            const ImgRegion & region, bool block_queue = true, int32_t command_queue = 0) const = 0;
        virtual void * map(MapMode mode = MAP_MODE_READ, bool block_queue = true, int32_t command_queue = 0) = 0;
        virtual void * map(MapMode mode, const ImgRegion & origin,
            const ImgRegion & region, bool block_queue = true, int32_t command_queue = 0) = 0;
        virtual void unmap(int32_t command_queue = 0) = 0;
        virtual ~Img() {}
        ImgRegion img_size;
    };


    class Context
    {
    public:
        virtual Buffer * createBuffer(size_t size, MemFlag mem_flag = MEM_FLAG_READ_WRITE, 
            void * host_ptr = nullptr, int32_t * error = nullptr) = 0;
        virtual Img * createImg(ImgFormat format, ImgRegion size, MemFlag mem_flag = MEM_FLAG_READ_WRITE,
            void * host_ptr = nullptr, int32_t * error = nullptr) = 0;
        virtual const char * getContextKind() = 0;
        virtual void finish(int32_t command_queue) = 0;
        virtual ~Context() {}

    protected:
        int32_t m_context_type;
    };

    NAPALM_EXPORT Context * createContext(const char * kind, int32_t platform_id, int32_t device_id, int32_t stream_count);
}
