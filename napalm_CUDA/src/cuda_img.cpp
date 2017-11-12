#include <napalm/cuda/cuda_buffer.h>
#include "cuda_utils.h"
#include "../include/napalm/cuda/cuda_img.h"
#include <cassert>

namespace napalm
{
    namespace cuda
    {
        CUDAImg::CUDAImg(const CUDAContext * ctx, ImgFormat format, ImgRegion size, 
            MemFlag mem_flag, void * host_ptr, int32_t * error): m_ctx(ctx)
        {
            this->mem_flag = mem_flag;
            CUresult res = CUDA_SUCCESS;
            int num_channels = 0;
            int byte_per_channels = 0;
            if (size.z > 1)
            {
                CUDA_ARRAY3D_DESCRIPTOR desc;
                memset(&desc, 0, sizeof(CUDA_ARRAY3D_DESCRIPTOR));
                desc.Width = size.x;
                desc.Height = size.y;
                desc.Depth = size.z;
                desc.Format = getCUDAImageForamt(format, desc.NumChannels, desc.Flags, byte_per_channels);
                res = cuArray3DCreate(&m_buffer, &desc);
                num_channels = int(desc.NumChannels);

            }
            else
            {
                CUDA_ARRAY_DESCRIPTOR desc;
                memset(&desc, 0, sizeof(CUDA_ARRAY_DESCRIPTOR));
                unsigned int flags = 0;
                desc.Format = getCUDAImageForamt(format, desc.NumChannels, flags, byte_per_channels);
                desc.Width = size.x;
                desc.Height = size.y;
                res = cuArrayCreate(&m_buffer, &desc);
                num_channels = int(desc.NumChannels);
            }
            m_channel_byte_count = num_channels * byte_per_channels;
            handleError(res, "Cuda array create");
            img_size = size;
            if (error != nullptr)
                *error = int(res);

            CUDA_RESOURCE_DESC m_r_desc;
            memset(&m_r_desc, 0, sizeof(CUDA_RESOURCE_DESC));
            m_r_desc.resType = CU_RESOURCE_TYPE_ARRAY;
            m_r_desc.res.array.hArray = m_buffer;
            m_r_desc.flags = 0;
            CUDA_TEXTURE_DESC t_desc;
            memset(&t_desc, 0, sizeof(CUDA_TEXTURE_DESC));
            t_desc.addressMode[0] = CU_TR_ADDRESS_MODE_CLAMP; /// TODO;
            t_desc.addressMode[1] = CU_TR_ADDRESS_MODE_CLAMP; /// TODO;
            t_desc.addressMode[2] = CU_TR_ADDRESS_MODE_CLAMP; /// TODO;
            t_desc.filterMode = CU_TR_FILTER_MODE_LINEAR; /// TODO set from outside

            res = cuTexObjectCreate(&m_texture, &m_r_desc, &t_desc, nullptr);
            handleError(res, "Cuda Texture create!");
            res = cuSurfObjectCreate(&m_surface, &m_r_desc);
            handleError(res, "Cuda Surface create!");

            if (host_ptr && (mem_flag & MEM_FLAG_COPY_HOST_PTR))
                write(host_ptr, true, 0);
            handleError(res, "CUDA Create image!");
        }

        void CUDAImg::write(const void * data, bool block_queue, int32_t command_queue)
        {
            write(data, ImgRegion(0, 0, 0), img_size, block_queue, command_queue);
        }

        void CUDAImg::write(const void * data, const ImgRegion & origin, const ImgRegion & region, bool block_queue, int32_t command_queue)
        {
            CUresult res = CUDA_SUCCESS;
            if (img_size.z > 1)
            {
                CUDA_MEMCPY3D copy_desc;
                memset(&copy_desc, 0, sizeof(copy_desc));
                copy_desc.Depth = size_t(region.z);
                copy_desc.Height = size_t(region.z);
                copy_desc.srcPitch = size_t(region.x * m_channel_byte_count);
                copy_desc.WidthInBytes = copy_desc.srcPitch;
                copy_desc.dstMemoryType = CU_MEMORYTYPE_ARRAY;
                copy_desc.dstArray = m_buffer;
                copy_desc.srcMemoryType = CU_MEMORYTYPE_HOST;
                copy_desc.srcHost = data;
                if (block_queue)
                    res = cuMemcpy3D(&copy_desc);
                else
                    res = cuMemcpy3DAsync(&copy_desc, m_ctx->getCQ(command_queue));

            }
            else
            {
                CUDA_MEMCPY2D copy_desc;
                memset(&copy_desc, 0, sizeof(copy_desc));
                copy_desc.dstMemoryType = CU_MEMORYTYPE_ARRAY;
                copy_desc.dstArray = m_buffer;
                copy_desc.srcMemoryType = CU_MEMORYTYPE_HOST;
                copy_desc.srcHost = data;
                copy_desc.srcPitch = region.x * m_channel_byte_count;
                copy_desc.WidthInBytes = copy_desc.srcPitch;
                copy_desc.Height = region.y;
                if (block_queue)
                    res = cuMemcpy2D(&copy_desc);
                else
                    res = cuMemcpy2DAsync(&copy_desc, m_ctx->getCQ(command_queue));
            }
            handleError(res, "CUDA img write");
        }

        void CUDAImg::read(void * data, bool block_queue, int32_t command_queue) const
        {
            read(data, ImgRegion(0, 0, 0), img_size, block_queue, command_queue);
        }

        void CUDAImg::read(void * data, const ImgRegion & origin, const ImgRegion & region, bool block_queue, int32_t command_queue) const
        {
            CUresult res = CUDA_SUCCESS;
            if (img_size.z > 1)
            {
                CUDA_MEMCPY3D copy_desc;
                memset(&copy_desc, 0, sizeof(copy_desc));
                copy_desc.srcPitch = size_t(region.x) * m_channel_byte_count;
                copy_desc.WidthInBytes = copy_desc.srcPitch;
                copy_desc.Height = size_t(region.y);
                copy_desc.Depth = size_t(region.z);
                copy_desc.dstMemoryType = CU_MEMORYTYPE_HOST;
                copy_desc.srcMemoryType = CU_MEMORYTYPE_ARRAY;
                copy_desc.dstHost = data;
                copy_desc.srcArray = m_buffer;
                if (block_queue)
                    res = cuMemcpy3D(&copy_desc);
                else
                    res = cuMemcpy3DAsync(&copy_desc, m_ctx->getCQ(command_queue));

            }
            else
            {
                CUDA_MEMCPY2D copy_desc;
                memset(&copy_desc, 0, sizeof(copy_desc));
                copy_desc.srcPitch = size_t(region.x) * m_channel_byte_count;
                copy_desc.WidthInBytes = copy_desc.srcPitch;
                copy_desc.Height = size_t(region.y);
                copy_desc.dstMemoryType = CU_MEMORYTYPE_HOST;
                copy_desc.srcMemoryType = CU_MEMORYTYPE_ARRAY;
                copy_desc.dstHost = data;
                copy_desc.srcArray = m_buffer;

                if (block_queue)
                    res = cuMemcpy2D(&copy_desc);
                else
                    res = cuMemcpy2DAsync(&copy_desc, m_ctx->getCQ(command_queue));
            }
            handleError(res, "CUDA img write");
         
        }

        void * CUDAImg::map(MapMode mode, bool block_queue, int32_t command_queue)
        {
            return map(mode, ImgRegion(0, 0, 0), img_size, block_queue, command_queue);
        }

        void * CUDAImg::map(MapMode mode, const ImgRegion & origin, const ImgRegion & region, bool block_queue, int32_t command_queue)
        {
            assert(false && "Napalm cuda not support image mapping!");
            return nullptr;
        }

        void CUDAImg::unmap(int32_t command_queue)
        {
            assert(false && "Napalm cuda not support image mapping!");
        }

        ArgumentPropereties CUDAImg::getARgumentPropereties() const
        {
            return ArgumentPropereties((void*)&m_texture, sizeof(m_texture));
        }

        ArgumentPropereties CUDAImg::getARgumentProperetiesWritable()
        {
            return ArgumentPropereties(&m_surface, sizeof(m_surface));
        }

        CUDAImg::~CUDAImg()
        {
            cuTexObjectDestroy(m_texture);
            cuSurfObjectDestroy(m_surface);
            cuArrayDestroy(m_buffer);
        }
    }
}
