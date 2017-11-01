#pragma once
#include <napalm/napalm.h>
#include <cuda.h>
#include <vector>

namespace napalm
{
    namespace cuda
    {
        class CUDAContext : public napalm::Context
        {
        public:
            CUDAContext(int32_t platform_id, int32_t device_id, int32_t stream_count);
            virtual napalm::Buffer * createBuffer(size_t size, 
                MemFlag flag = MEM_FLAG_READ_WRITE,
                void * host_ptr = nullptr, int32_t * error = 0) const;
            virtual Img * createImg(ImgFormat format, ImgRegion size, MemFlag mem_flag = MEM_FLAG_READ_WRITE,
                void * host_ptr = nullptr, int32_t * error = nullptr) const;
            virtual Program * createProgram(const ProgramData & data) const;
            virtual const char * getContextKind() const;
            virtual void finish(int32_t command_queue) const;
            virtual void registerContext() const;
            virtual ~CUDAContext();
            CUcontext getCLContext() const;
            CUstream getCQ(int32_t id) const;
            CUdevice getCUDADevice() const;
        private:
            CUcontext m_cuda_context;
            std::vector<CUstream> m_command_queues;
            CUdevice m_cuda_device_id;
        };

    }
}
