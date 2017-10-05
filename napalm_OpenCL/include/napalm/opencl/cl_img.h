#pragma once
#include <napalm/napalm.h>
#include <napalm/opencl/opencl_context.h>
namespace napalm {
    namespace cl {
        struct CLImg : public Img
        {
            CLImg(OpenCLContext * ctx, ImgFormat format, ImgRegion size, 
                MemFlag mem_flag, void * host_ptr, int32_t * error);
            virtual void write(const void * data, int32_t command_queue);
            virtual void write(const void * data, const ImgRegion & origin,
                const ImgRegion & region, int32_t command_queue);
            virtual void read(void * data, int32_t command_queue) const;
            virtual void read(void * data, const ImgRegion & origin,
                const ImgRegion & region, int32_t command_queue) const;
            virtual void * map(MapMode mode, int32_t command_queue);
            virtual void * map(MapMode mode, const ImgRegion & origin,
                const ImgRegion & region, int32_t command_queue);
            virtual void unmap(int32_t command_queue);

            virtual ~CLImg();
        private:
            cl_mem m_buffer;
            OpenCLContext * m_ctx;
            void * m_map_address = nullptr;
        };
    }
}