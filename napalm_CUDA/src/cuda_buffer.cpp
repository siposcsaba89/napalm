#include <napalm/cuda/cuda_buffer.h>
#include "../include/napalm/cuda/cuda_buffer.h"
#include "cuda_utils.h"


namespace napalm
{
    namespace cuda
    {
        CUDABuffer::CUDABuffer(const CUDAContext * ctx, size_t size, MemFlag flag, void * host_ptr, int32_t * err) :
            m_ctx(ctx)
        {
            buff_size = size;
            cl_int err_;
            m_buffer = clCreateBuffer(m_ctx->getCLContext(), getCLMemFlag(flag), buff_size, host_ptr, &err_);
            handleError(err_, "Buffer creating!");
            if (err != nullptr)
                *err = int32_t(err_);
        }

        void CUDABuffer::write(const void * data, bool block_queue, int32_t command_queue)
        {
            write(data, 0, buff_size, block_queue, command_queue);
        }

        void CUDABuffer::write(const void * data, size_t offset, size_t size, bool block_queue, int32_t command_queue)
        {
            //TODO blocking write parameter, maybe wait list shuld be also considered
            cl_int err = 0;
            err = clEnqueueWriteBuffer(m_ctx->getCQ(command_queue), m_buffer, block_queue, offset, size, data, 0, nullptr, nullptr);
            handleError(err, "OpenCL Enequeue Write buffer!");
        }

        void CUDABuffer::read(void * data, bool block_queue, int32_t command_queue) const
        {
            read(data, 0, buff_size, block_queue, command_queue);
        }

        void CUDABuffer::read(void * data, size_t offset, size_t size, bool block_queue, int32_t command_queue) const
        {
            //TODO blocking write parameter, maybe wait list shuld be also considered
            cl_int err = 0;
            err = clEnqueueReadBuffer(m_ctx->getCQ(command_queue), m_buffer, block_queue, offset, size, data, 0, nullptr, nullptr);
            handleError(err, "OpenCL Enequeue Read buffer!");
        }

        void * CUDABuffer::map(MapMode mode, bool block_queue, int32_t command_queue)
        {
            return map(mode, 0, buff_size, block_queue, command_queue);
        }

        void * CUDABuffer::map(MapMode mode, size_t offset, size_t size, bool block_queue, int32_t command_queue)
        {
            //TODO blocking write parameter, maybe wait list shuld be also considered
            cl_int err = 0;
            m_map_address = clEnqueueMapBuffer(m_ctx->getCQ(command_queue), m_buffer, block_queue, 
                getCLMapFlag(mode), offset, size,0, nullptr, nullptr, &err);
            handleError(err, "OpenCL Enequeue Map buffer!");
            return m_map_address;
        }

        void CUDABuffer::unmap(int32_t command_queue)
        {
            //TODO blocking write parameter, maybe wait list shuld be also considered
            cl_int err = 0;
            err = clEnqueueUnmapMemObject(m_ctx->getCQ(command_queue), m_buffer, m_map_address, 0, nullptr, nullptr);
            handleError(err, "OpenCL Unmap buffer!");
        }

		ArgumentPropereties CUDABuffer::getARgumentPropereties()
		{
			return ArgumentPropereties(&m_buffer, sizeof(m_buffer));
		}

        CUDABuffer::~CUDABuffer()
        {
        }

    }
}
