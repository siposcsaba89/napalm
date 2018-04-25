#pragma once
#include <napalm/napalm.h>
#include <napalm/cuda/cuda_context.h>
namespace napalm {
    namespace cuda {
        struct CUDAImg : public Img
        {
            CUDAImg(const CUDAContext * ctx, 
                ImgFormat format,
                ImgRegion size, 
                MemFlag mem_flag, 
                void * host_ptr, 
                int32_t * error);
            virtual void write(const void * data,
                SyncMode block_queue,
                int32_t command_queue) override;
            virtual void write(const void * data,
                const ImgRegion & origin,
                const ImgRegion & region,
                SyncMode block_queue,
                int32_t command_queue) override;
            virtual void read(void * data,
                SyncMode block_queue,
                int32_t command_queue) const override;
            virtual void read(void * data,
                const ImgRegion & origin,
                const ImgRegion & region,
                SyncMode block_queue,
                int32_t command_queue) const override;
            virtual void * map(MapMode mode,
                SyncMode block_queue,
                int32_t command_queue) override;
            virtual void * map(MapMode mode,
                const ImgRegion & origin,
                const ImgRegion & region, 
                SyncMode block_queue,
                int32_t command_queue) override;
            virtual void unmap(int32_t command_queue);
            virtual ArgumentPropereties getARgumentPropereties() const;
            virtual ArgumentPropereties getARgumentProperetiesWritable();

            virtual int32_t getGLTextureID() const;
            virtual void mapGLImage(int32_t command_queue = 0);
            virtual void unmapGLImage();

            virtual ~CUDAImg();
        private:
            CUarray m_buffer;
            CUtexObject m_texture;
            CUsurfObject m_surface;
            const CUDAContext * m_ctx;
            void * m_map_address = nullptr;
            int32_t m_channel_byte_count = 0;
            int32_t m_gl_texture_id = -1;
            CUgraphicsResource m_cu_graphics_resource;
            int32_t m_gl_queue_acquired = -1;
        };
    }
}