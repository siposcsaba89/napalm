#pragma once
#include <napalm/napalm.h>
#include <napalm/opencl/opencl_context.h>
namespace napalm {
    namespace cl {
        struct CLBuffer : public Buffer
        {
            CLBuffer(const OpenCLContext * ctx, 
                size_t size, MemFlag flag, 
                void * host_ptr, 
                int32_t * err);
            virtual void write(const void * data,
                SyncMode block_queue, 
                int32_t command_queue) override;
            virtual void write(const void * data,
                size_t offet,
                size_t size,
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
            virtual void unmap(int32_t command_queue) override;
			virtual ArgumentPropereties getARgumentPropereties() const override;
            virtual ~CLBuffer();
        private:
            mutable cl_mem m_buffer;
            const OpenCLContext * m_ctx;
            void * m_map_address = nullptr;
        };
    }
}