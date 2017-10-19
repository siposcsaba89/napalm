#pragma once
#include <napalm/napalm.h>
#include <napalm/opencl/napalm_OpenCL_export.h>
#include <CL/cl.h>
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
			virtual Program * createProgram(const ProgramData & data, const char * compiler_options) const;
            virtual const char * getContextKind() const;
            virtual void finish(int32_t command_queue) const;
            virtual ~CUDAContext();
            cl_context getCLContext() const;
            cl_command_queue getCQ(int32_t id) const;
            cl_device_id getCLDevice() const;
        private:
            cl_context m_cl_context;
            std::vector<cl_command_queue> m_command_queues;
            cl_device_id m_cl_device_id;
            ProgramStore * m_store;
        };

    }
}
