#include <napalm/opencl/cl_buffer.h>
#include "cl_utils.h"
#include "../include/napalm/opencl/cl_img.h"


namespace napalm
{
    namespace cl
    {
        CLImg::CLImg(OpenCLContext * ctx, ImgFormat format, ImgRegion size, 
            MemFlag mem_flag, void * host_ptr, int32_t * error): m_ctx(ctx)
        {
            img_size = size;
            cl_int err = 0;
            cl_image_format cl_img_f;
            cl_image_desc cl_img_d;
            getCLImageForamt(format, size, cl_img_f, cl_img_d);
            m_buffer = clCreateImage(ctx->getCLContext(), getCLMemFlag(mem_flag), 
                &cl_img_f , &cl_img_d, host_ptr, &err);
            if (error != nullptr)
                *error = int(err);
            handleError(err, "OpenCL Create image!");
        }

        void CLImg::write(const void * data, int32_t command_queue)
        {
            write(data, ImgRegion(0, 0, 0), img_size, command_queue);
        }

        void CLImg::write(const void * data, const ImgRegion & origin, const ImgRegion & region, int32_t command_queue)
        {
            size_t img_origin[] =
            {
                size_t(origin.x),
                size_t(origin.y),
                size_t(origin.z),
            };
            size_t img_region[] =
            {
                size_t(region.x),
                size_t(region.y),
                size_t(region.z),
            };

            //TODO blocking write parameter, maybe wait list shuld be also considered
            cl_int err = clEnqueueWriteImage(m_ctx->getCQ(command_queue), m_buffer, true,
                img_origin, img_region, 0, 0, data, 0, nullptr, nullptr);
            handleError(err, "OpenCL img create");
        }

        void CLImg::read(void * data, int32_t command_queue) const
        {
            read(data, ImgRegion(0, 0, 0), img_size, command_queue);
        }

        void CLImg::read(void * data, const ImgRegion & origin, const ImgRegion & region, int32_t command_queue) const
        {
            size_t img_origin[] =
            {
                size_t(origin.x),
                size_t(origin.y),
                size_t(origin.z),
            };
            size_t img_region[] =
            {
                size_t(region.x),
                size_t(region.y),
                size_t(region.z),
            };

            //TODO blocking write parameter, maybe wait list shuld be also considered
            cl_int err = clEnqueueReadImage(m_ctx->getCQ(command_queue), m_buffer, true,
                img_origin, img_region, 0, 0, data, 0, nullptr, nullptr);
            handleError(err, "OpenCL img create");
        }

        void * CLImg::map(MapMode mode, int32_t command_queue)
        {
            return map(mode, ImgRegion(0, 0, 0), img_size, command_queue);
        }

        void * CLImg::map(MapMode mode, const ImgRegion & origin, const ImgRegion & region, int32_t command_queue)
        {
            size_t img_origin[] =
            {
                size_t(origin.x),
                size_t(origin.y),
                size_t(origin.z),
            };
            size_t img_region[] =
            {
                size_t(region.x),
                size_t(region.y),
                size_t(region.z),
            };
            //TODO return this values to the user
            size_t image_row_pitch = 0;
            size_t image_slice_pitch = 0;
            //TODO blocking write parameter, maybe wait list shuld be also considered
            cl_int err = 0;
            m_map_address = clEnqueueMapImage(m_ctx->getCQ(command_queue), m_buffer, true,
                getCLMapFlag(mode), img_origin ,img_region, &image_row_pitch, &image_slice_pitch,0, nullptr, nullptr, &err);
            handleError(err, "OpenCL img create");
            return m_map_address;
        }

        void CLImg::unmap(int32_t command_queue)
        {
            cl_int err = clEnqueueUnmapMemObject(m_ctx->getCQ(command_queue), m_buffer, m_map_address, 0, 0, 0);
            handleError(err, "OpenCL unmap Image");
        }

        CLImg::~CLImg()
        {
        }
    }
}
