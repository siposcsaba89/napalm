#pragma once
#include <napalm/napalm.h>
#include <napalm/cuda/cuda_context.h>
namespace napalm {
    namespace cuda {
        struct CUDAImg : public Img
        {
            CUDAImg(const CUDAContext * ctx, ImgFormat format, ImgRegion size, 
                MemFlag mem_flag, void * host_ptr, int32_t * error);
            virtual void write(const void * data, bool block_queue, int32_t command_queue);
            virtual void write(const void * data, const ImgRegion & origin,
                const ImgRegion & region, bool block_queue, int32_t command_queue);
            virtual void read(void * data, bool block_queue, int32_t command_queue) const;
            virtual void read(void * data, const ImgRegion & origin,
                const ImgRegion & region, bool block_queue, int32_t command_queue) const;
            virtual void * map(MapMode mode, bool block_queue, int32_t command_queue);
            virtual void * map(MapMode mode, const ImgRegion & origin,
                const ImgRegion & region, bool block_queue, int32_t command_queue);
            virtual void unmap(int32_t command_queue);
            virtual ArgumentPropereties getARgumentPropereties() const;
            virtual ArgumentPropereties getARgumentProperetiesWritable();
            virtual ~CUDAImg();
        private:
            CUarray m_buffer;
            CUtexObject m_texture;
            CUsurfObject m_surface;
            const CUDAContext * m_ctx;
            void * m_map_address = nullptr;
            int32_t m_channel_byte_count = 0;
        };
    }
}