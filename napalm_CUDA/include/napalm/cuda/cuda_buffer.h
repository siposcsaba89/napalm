#pragma once
#include <napalm/napalm.h>
#include <napalm/cuda/cuda_context.h>
namespace napalm {
    namespace cuda {
        struct CUDABuffer : public Buffer
        {
            CUDABuffer(const CUDAContext * ctx, size_t size, MemFlag flag, void * host_ptr, int32_t * err);
            virtual void write(const void * data, bool block_queue, int32_t command_queue);
            virtual void write(const void * data, size_t offet, size_t size, bool block_queue, int32_t command_queue);
            virtual void read(void * data, bool block_queue, int32_t command_queue) const;
            virtual void read(void * data, size_t offet, size_t size, bool block_queue, int32_t command_queue) const;
            virtual void * map(MapMode mode, bool block_queue, int32_t command_queue);
            virtual void * map(MapMode mode, size_t offset, size_t size, bool block_queue, int32_t command_queue);
            virtual void unmap(int32_t command_queue);
            virtual ArgumentPropereties getARgumentPropereties();
            virtual ~CUDABuffer();
        private:
            cl_mem m_buffer;
            const CUDAContext * m_ctx;
            void * m_map_address = nullptr;
        };
    }
}