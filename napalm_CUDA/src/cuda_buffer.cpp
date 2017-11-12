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
            CUresult res;
            if (flag & MEM_FLAG_ALLOC_HOST_PTR)
                res = cuMemAllocManaged(&m_buffer, size, CU_MEM_ATTACH_GLOBAL);
            else
                res = cuMemAlloc(&m_buffer, size);
            if (host_ptr != nullptr && (flag & MEM_FLAG_COPY_HOST_PTR))
                write(host_ptr, true, 0);
            handleError(res, "CUDA Buffer creating!");
            m_map_address = (void*)m_buffer;
            if (err != nullptr)
                *err = int32_t(res);
            mem_flag = flag;
        }

        void CUDABuffer::write(const void * data, bool block_queue, int32_t command_queue)
        {
            write(data, 0, buff_size, block_queue, command_queue);
        }

        void CUDABuffer::write(const void * data, size_t offset, size_t size, bool block_queue, int32_t command_queue)
        {
            CUresult res = CUDA_SUCCESS;
            if (block_queue)
                res = cuMemcpyHtoD(m_buffer + offset, data, size);
            else
                res = cuMemcpyHtoDAsync(m_buffer, data, size, m_ctx->getCQ(command_queue));
            handleError(res, "CUDA Write buffer!");
        }

        void CUDABuffer::read(void * data, bool block_queue, int32_t command_queue) const
        {
            read(data, 0, buff_size, block_queue, command_queue);
        }

        void CUDABuffer::read(void * data, size_t offset, size_t size, bool block_queue, int32_t command_queue) const
        {
            CUresult res = CUDA_SUCCESS;
            if (block_queue)
                res = cuMemcpyDtoH(data, m_buffer, size);
            else
                res = cuMemcpyDtoHAsync(data, m_buffer + offset, size, m_ctx->getCQ(command_queue));
            handleError(res, "CUDA Write buffer!");
        }

        void * CUDABuffer::map(MapMode mode, bool block_queue, int32_t command_queue)
        {
            return map(mode, 0, buff_size, block_queue, command_queue);
        }

        void * CUDABuffer::map(MapMode mode, size_t offset, size_t size, bool block_queue, int32_t command_queue)
        {
            if ((mem_flag & MEM_FLAG_ALLOC_HOST_PTR) == 0)
            {
                handleError(CUDA_ERROR_ASSERT, "Buffer was created without MEM_FLAG_ALLOC_HOST_PTR");
            }

            return m_map_address;
        }

        void CUDABuffer::unmap(int32_t command_queue)
        {
            if ((mem_flag & MEM_FLAG_ALLOC_HOST_PTR) == 0)
            {
                handleError(CUDA_ERROR_ASSERT, "Buffer was created without MEM_FLAG_ALLOC_HOST_PTR");
            }
        }

        ArgumentPropereties CUDABuffer::getARgumentPropereties() const
        {
            return ArgumentPropereties(&m_buffer, sizeof(m_buffer));
        }

        CUDABuffer::~CUDABuffer()
        {
            CUresult res = cuMemFree(m_buffer);
            handleError(res, "cuda memory free");
        }

    }
}
