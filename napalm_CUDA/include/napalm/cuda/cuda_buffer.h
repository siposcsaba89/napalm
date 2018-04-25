#pragma once
#include <napalm/napalm.h>
#include <napalm/cuda/cuda_context.h>
namespace napalm {
    namespace cuda {
        struct CUDABuffer : public Buffer
        {
            CUDABuffer(const CUDAContext * ctx, 
                size_t size, 
                MemFlag flag, 
                void * host_ptr, 
                int32_t * err);
            virtual void write(const void * data,
                SyncMode block_queue,
                int32_t command_queue) override;
            virtual void write(const void * data,
                size_t offet, size_t size,
                SyncMode block_queue,
                int32_t command_queue) override;
            virtual void read(void * data,
                SyncMode block_queue,
                int32_t command_queue) const override;
            virtual void read(void * data, 
                size_t offet, size_t size,
                SyncMode block_queue,
                int32_t command_queue) const override;
            virtual void * map(MapMode mode,
                SyncMode block_queue,
                int32_t command_queue) override;
            virtual void * map(MapMode mode,
                size_t offset,
                size_t size,
                SyncMode block_queue,
                int32_t command_queue) override;
            virtual void unmap(int32_t command_queue);
            virtual ArgumentPropereties getARgumentPropereties() const override;
            virtual ~CUDABuffer();
        private:
            mutable CUdeviceptr m_buffer;
            const CUDAContext * m_ctx;
            void * m_map_address = nullptr;
        };
    }
}