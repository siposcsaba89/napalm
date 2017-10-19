#include <napalm/cuda/cuda_buffer.h>
#include "cuda_utils.h"
#include "../include/napalm/cuda/cuda_img.h"


namespace napalm
{
    namespace cuda
    {
        CUDAImg::CUDAImg(const CUDAContext * ctx, ImgFormat format, ImgRegion size, 
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

        void CUDAImg::write(const void * data, bool block_queue, int32_t command_queue)
        {
            write(data, ImgRegion(0, 0, 0), img_size, block_queue, command_queue);
        }

        void CUDAImg::write(const void * data, const ImgRegion & origin, const ImgRegion & region, bool block_queue, int32_t command_queue)
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
            cl_int err = clEnqueueWriteImage(m_ctx->getCQ(command_queue), m_buffer, block_queue,
                img_origin, img_region, 0, 0, data, 0, nullptr, nullptr);
            handleError(err, "OpenCL img create");
        }

        void CUDAImg::read(void * data, bool block_queue, int32_t command_queue) const
        {
            read(data, ImgRegion(0, 0, 0), img_size, block_queue, command_queue);
        }

        void CUDAImg::read(void * data, const ImgRegion & origin, const ImgRegion & region, bool block_queue, int32_t command_queue) const
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
            cl_int err = clEnqueueReadImage(m_ctx->getCQ(command_queue), m_buffer, block_queue,
                img_origin, img_region, 0, 0, data, 0, nullptr, nullptr);
            handleError(err, "OpenCL img create");
        }

        void * CUDAImg::map(MapMode mode, bool block_queue, int32_t command_queue)
        {
            return map(mode, ImgRegion(0, 0, 0), img_size, block_queue, command_queue);
        }

        void * CUDAImg::map(MapMode mode, const ImgRegion & origin, const ImgRegion & region, bool block_queue, int32_t command_queue)
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
            m_map_address = clEnqueueMapImage(m_ctx->getCQ(command_queue), m_buffer, block_queue,
                getCLMapFlag(mode), img_origin ,img_region, &image_row_pitch, &image_slice_pitch,0, nullptr, nullptr, &err);
            handleError(err, "OpenCL img create");
            return m_map_address;
        }

        void CUDAImg::unmap(int32_t command_queue)
        {
            cl_int err = clEnqueueUnmapMemObject(m_ctx->getCQ(command_queue), m_buffer, m_map_address, 0, 0, 0);
            handleError(err, "OpenCL unmap Image");
        }

		ArgumentPropereties CUDAImg::getARgumentPropereties()
		{
			return ArgumentPropereties(&m_buffer, sizeof(m_buffer));
		}

		ArgumentPropereties CUDAImg::getARgumentProperetiesWritable()
		{
			return getARgumentPropereties();
		}

        CUDAImg::~CUDAImg()
        {
        }
    }
}
