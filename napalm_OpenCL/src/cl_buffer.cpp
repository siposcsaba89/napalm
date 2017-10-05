#include <napalm/opencl/cl_buffer.h>
#include "../include/napalm/opencl/cl_buffer.h"
#include "cl_utils.h"


namespace napalm
{
    namespace cl
    {
        CLBuffer::CLBuffer(OpenCLContext * ctx, size_t size, MemFlag flag, void * host_ptr, int32_t * err) :
            m_ctx(ctx)
        {
            buff_size = size;
            cl_int err_;
            m_buffer = clCreateBuffer(m_ctx->getCLContext(), getCLMemFlag(flag), buff_size, host_ptr, &err_);
            handleError(err_, "Buffer creating!");
            if (err != nullptr)
                *err = int32_t(err_);
        }

        void CLBuffer::write(const void * data, int32_t command_queue)
        {
            write(data, 0, buff_size, command_queue);
        }

        void CLBuffer::write(const void * data, size_t offset, size_t size, int32_t command_queue)
        {
            //TODO blocking write parameter, maybe wait list shuld be also considered
            cl_int err = 0;
            err = clEnqueueWriteBuffer(m_ctx->getCQ(command_queue), m_buffer, true, offset, size, data, 0, nullptr, nullptr);
            handleError(err, "OpenCL Enequeue Write buffer!");
        }

        void CLBuffer::read(void * data, int32_t command_queue) const
        {
            read(data, 0, buff_size, command_queue);
        }

        void CLBuffer::read(void * data, size_t offset, size_t size, int32_t command_queue) const
        {
            //TODO blocking write parameter, maybe wait list shuld be also considered
            cl_int err = 0;
            err = clEnqueueReadBuffer(m_ctx->getCQ(command_queue), m_buffer, true, offset, size, data, 0, nullptr, nullptr);
            handleError(err, "OpenCL Enequeue Read buffer!");
        }

        void * CLBuffer::map(MapMode mode, int32_t command_queue)
        {
            return map(mode, 0, buff_size, command_queue);
        }

        void * CLBuffer::map(MapMode mode, size_t offset, size_t size, int32_t command_queue)
        {
            //TODO blocking write parameter, maybe wait list shuld be also considered
            cl_int err = 0;
            m_map_address = clEnqueueMapBuffer(m_ctx->getCQ(command_queue), m_buffer, true, 
                getCLMapFlag(mode), offset, size,0, nullptr, nullptr, &err);
            handleError(err, "OpenCL Enequeue Map buffer!");
            return m_map_address;
        }

        void CLBuffer::unmap(int32_t command_queue)
        {
            //TODO blocking write parameter, maybe wait list shuld be also considered
            cl_int err = 0;
            err = clEnqueueUnmapMemObject(m_ctx->getCQ(command_queue), m_buffer, m_map_address, 0, nullptr, nullptr);
            handleError(err, "OpenCL Unmap buffer!");
        }

        CLBuffer::~CLBuffer()
        {
        }

    }
}
