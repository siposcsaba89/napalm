#pragma once
#include <napalm/napalm.h>
#include <napalm/opencl/opencl_context.h>
namespace napalm {
    namespace cl {
        struct CLImg : public Img
        {
            CLImg(const OpenCLContext * ctx, ImgFormat format, ImgRegion size, 
                MemFlag mem_flag, void * host_ptr, int32_t * error);
            virtual void write(const void * data, 
                SyncMode block_queue, int32_t command_queue) override;
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
            virtual void mapGLImage(int32_t command_queue);
            virtual void unmapGLImage();
            virtual ~CLImg();
        private:
            cl_mem m_buffer;
            const OpenCLContext * m_ctx;
            void * m_map_address = nullptr;
            int32_t m_gl_texture_id = -1;
            int32_t m_gl_queue_acquired = -1;
        };
    }
}