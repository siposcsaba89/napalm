#pragma once
#include <napalm/napalm_export.h>
#include <inttypes.h>
#include <iostream>
#include "program_store.h"

//TODO Context manager, to create and manage context by name like : Context * creteContextIfNotExists("main_context");

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

    struct ArgumentPropereties
    {
        ArgumentPropereties(void * ptr = nullptr, size_t argsize = 0) : arg_ptr(ptr), sizeof_arg(argsize) {}
        void * arg_ptr;
        size_t sizeof_arg;
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
        virtual ArgumentPropereties getARgumentPropereties() = 0;
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
        virtual ArgumentPropereties getARgumentPropereties() = 0;
        virtual ArgumentPropereties getARgumentProperetiesWritable() = 0;
        virtual ~Img() {}
    public:
        ImgRegion img_size;
    };


    struct Kernel
    {
        virtual void setArg(int32_t idx, void * val, size_t sizeof_arg) = 0;
        virtual void setArgs(int32_t num_args, void ** argument, size_t * argument_sizes) = 0;
        virtual void execute(int32_t command_queue, ImgRegion num_blocks, ImgRegion block_sizes) = 0;

        void operator()(int32_t command_queue, ImgRegion num_blocks, ImgRegion block_sizes)
        {
            execute(command_queue, num_blocks, block_sizes);
        }

        template <typename ... Types>
        void operator()(int32_t command_queue, ImgRegion num_blocks, ImgRegion block_sizes, Types&&... args)
        {
            size_t num_args = sizeof...(args);
            void* arguments[sizeof...(args)];
            size_t args_sizeof[sizeof...(args)];
            fillArgVector(0, arguments, args_sizeof, args...);
            setArgs(int32_t(num_args), arguments, args_sizeof);
            execute(command_queue, num_blocks, block_sizes);
        }
        virtual ~Kernel() {}
    private:
        void fillArgVector(int32_t arg_idx, void** arg_address, size_t * arg_sizeof)
        {
            std::cout << "Uress" << std::endl;
        }

        template <typename T, typename ... Types>
        void fillArgVector(int32_t arg_idx, void** arg_address, size_t * arg_sizeof, T && arg, Types&&... Fargs)
        {
            arg_address[arg_idx] = &arg;
            arg_sizeof[arg_idx] = sizeof(arg);
            fillArgVector(++arg_idx, arg_address, arg_sizeof, Fargs...);
        }

        template <typename ... Types>
        void fillArgVector(int32_t arg_idx, void** arg_address, size_t * arg_sizeof, Buffer & arg, Types&&... Fargs)
        {
            auto prop = arg.getARgumentPropereties();
            arg_address[arg_idx] = prop.arg_ptr;
            arg_sizeof[arg_idx] = prop.sizeof_arg;
            fillArgVector(++arg_idx, arg_address, arg_sizeof, Fargs...);
        }

        template <typename ... Types>
        void fillArgVector(int32_t arg_idx, void** arg_address, size_t * arg_sizeof, const Img & arg, Types&&... Fargs)
        {
            auto prop = arg.getARgumentPropereties();
            arg_address[arg_idx] = prop.arg_ptr;
            arg_sizeof[arg_idx] = prop.sizeof_arg; 
            fillArgVector(++arg_idx, arg_address, arg_sizeof, Fargs...);
        }

        template <typename ... Types>
        void fillArgVector(int32_t arg_idx, void** arg_address, size_t * arg_sizeof, Img & arg, Types&&... Fargs)
        {
            auto prop = arg.getARgumentProperetiesWritable();
            arg_address[arg_idx] = prop.arg_ptr;
            arg_sizeof[arg_idx] = prop.sizeof_arg;
            fillArgVector(++arg_idx, arg_address, arg_sizeof, Fargs...);
        }
    };

    struct ProgramBinary
    {
        const char * data;
        size_t binary_size;
    };

    struct Program
    {
        virtual Kernel & getKernel(const char *  kernel_name) = 0;
        virtual bool getStatus() const = 0;
        virtual ProgramBinary getBinary() = 0;
        virtual ~Program() {}
    };

    struct ProgramData
    {
        enum DataType
        {
            DATA_TYPE_BINARY_DATA = 0,
            DATA_TYPE_SOURCE_DATA = 1,
            DATA_TYPE_BINARY_FILE_NAME = 2,
            DATA_TYPE_SOURCE_FILE_NAME = 3
        };
        const char * data = nullptr;
        size_t data_size = 0;
        DataType data_type;
        std::string timestamp = "";
        ProgramData() {}
        ProgramData(DataType type, const char * data, size_t data_size_ = 0, const std::string & timestamp = "") :
            data_type(type), data(data), data_size(data_size_), timestamp(timestamp) {
            if (data_size == 0)
                data_size = strlen(data);
        }
    };

    class Context
    {
    public:
        virtual Buffer * createBuffer(size_t size, MemFlag mem_flag = MEM_FLAG_READ_WRITE, 
            void * host_ptr = nullptr, int32_t * error = nullptr) = 0;
        virtual Img * createImg(ImgFormat format, ImgRegion size, MemFlag mem_flag = MEM_FLAG_READ_WRITE,
            void * host_ptr = nullptr, int32_t * error = nullptr) = 0;
        virtual Program * createProgram(const ProgramData & data, const char * compiler_options = nullptr) = 0;
        virtual const char * getContextKind() const = 0;
        virtual void finish(int32_t command_queue) = 0;
        void setProgramStore(ProgramStore * store) { m_pr_store = store; }
        ProgramStore * getProgramStore() { return m_pr_store; }


    protected:
        virtual ~Context() {}
    private:
        friend void destroyContext(Context * ctx);
        ProgramStore * m_pr_store = nullptr;
    };

    struct PlatformAndDeviceInfo
    {
        int32_t num_platforms = 0;
        char ** platforms = nullptr;
        int32_t * num_devices = nullptr;
        char *** device_names = nullptr;

        PlatformAndDeviceInfo() {}
        PlatformAndDeviceInfo(const PlatformAndDeviceInfo & other) { *this = other; }
        PlatformAndDeviceInfo & operator= (const PlatformAndDeviceInfo & other)
        {
            for (int i = 0; i < num_platforms; ++i)
            {
                for (int j = 0; j < num_devices[i]; ++j)
                {
                    delete[] device_names[i][j];
                }
                delete[] platforms[i];
                delete[] device_names[i];
            }
            delete[] num_devices;
            delete[] platforms;
            delete[] device_names;
            num_devices = nullptr;
            platforms = nullptr;
            device_names = nullptr;
            num_platforms = other.num_platforms;
            platforms = new char*[num_platforms];
            device_names = new char**[num_platforms];
            num_devices = new int32_t[num_platforms];
            memcpy(num_devices, other.num_devices, num_platforms * sizeof(int32_t));
            for (int i = 0; i < num_platforms; ++i)
            {
                device_names[i] = new char *[num_devices[i]];
                size_t platform_name_l = strlen(other.platforms[i]) + 1;
                platforms[i] = new char[platform_name_l];
                strcpy_s(platforms[i], platform_name_l, other.platforms[i]);
                for (int j = 0; j < num_devices[i]; ++j)
                {
                    size_t dev_name_size = strlen(other.device_names[i][j]) + 1;
                    device_names[i][j] = new char[dev_name_size];
                    strcpy_s(device_names[i][j], dev_name_size, other.device_names[i][j]);
                }
            }
            return *this;
        }
        ~PlatformAndDeviceInfo()
        {
            for (int i = 0; i < num_platforms; ++i)
            {
                for (int j = 0; j < num_devices[i]; ++j)
                {
                    delete[] device_names[i][j];
                }
                delete[] platforms[i];
                delete[] device_names[i];
            }
            delete[] num_devices;
            delete[] platforms;
            delete[] device_names;
            num_devices = nullptr;
            platforms = nullptr;
            device_names = nullptr;
            num_platforms = 0;
        }
    };

    NAPALM_EXPORT PlatformAndDeviceInfo getPlatformAndDeviceInfo(const char * api_type);
    NAPALM_EXPORT Context * createContext(const char * kind, int32_t platform_id, int32_t device_id, int32_t stream_count);
    NAPALM_EXPORT void destroyContext(Context * ctx);
}
