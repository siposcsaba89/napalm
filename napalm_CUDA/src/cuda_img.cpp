#include <napalm/cuda/cuda_buffer.h>
#include "cuda_utils.h"
#include "../include/napalm/cuda/cuda_img.h"
#include <cassert>
#if GL_SHARING_ENABLED
#if HAVE_GLEW
#include <GL/glew.h>
#else
#include <GLES2/gl2.h>
#endif
#include <cudaGL.h>
#endif

namespace napalm
{
    namespace cuda
    {
        CUDAImg::CUDAImg(const CUDAContext * ctx,
            ImgFormat format,
            ImgRegion size, 
            MemFlag mem_flag,
            void * host_ptr,
            int32_t * error): m_ctx(ctx)
        {
            img_size = size;
            this->mem_flag = mem_flag;
            CUresult res = CUDA_SUCCESS;

            unsigned int desc_flags = 0;
            unsigned int desc_num_channels = 0;
            int byte_per_channels = 0;

            auto desc_format = getCUDAImageForamt(format, 
                desc_num_channels,
                desc_flags,
                byte_per_channels);
            m_channel_byte_count = desc_num_channels * byte_per_channels;

            if (mem_flag & MEM_FLAG_CREATE_GL_SHARED)
            {

                GLuint texture;
                glGenTextures(1, &texture);
                //binnding the texture
                glBindTexture(GL_TEXTURE_2D, texture);
                //regular sampler params
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                //need to set GL_NEAREST
                //(not GL_NEAREST_MIPMAP_* which would cause CL_INVALID_GL_OBJECT later)
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                //specify texture dimensions, format etc
                GLint gl_internal_format = GL_RGBA;
                GLenum gl_format = GL_RGBA;


                switch (format.img_channel_format)
                {
                case napalm::IMG_CHANNEL_FORMAT_INTENSITY:
                    gl_internal_format = GL_LUMINANCE;
                    gl_format = gl_internal_format;
                    break;
                case napalm::IMG_CHANNEL_FORMAT_R:
                    gl_internal_format = GL_LUMINANCE;
                    gl_format = gl_internal_format;
                    break;
                case napalm::IMG_CHANNEL_FORMAT_A:
                    gl_internal_format = GL_LUMINANCE;
                    gl_format = gl_internal_format;
                    break;
                case napalm::IMG_CHANNEL_FORMAT_RGBA:
                    gl_internal_format = GL_RGBA;
                    gl_format = gl_internal_format;
                    break;
                default:
                    assert(false && "cannot create gl shared image format");
                    std::runtime_error("cannot create gl shared image format");
                    break;
                }

                GLenum type = GL_FLOAT;

                switch (format.data_type)
                {
                case napalm::DATA_TYPE_FLOAT:
                    type = GL_FLOAT;
                    break;
                case napalm::DATA_TYPE_UNORM_INT8:
                    type = GL_UNSIGNED_BYTE;
                    break;
                case napalm::DATA_TYPE_UNORM_INT16:
                    type = GL_UNSIGNED_SHORT;
                    break;
                case napalm::DATA_TYPE_UNSIGNED_INT8:
                    type = GL_UNSIGNED_BYTE;
                    break;
                case napalm::DATA_TYPE_UNSIGNED_INT16:
                    type = GL_UNSIGNED_SHORT;
                    break;
                default:
                    assert(false && "cannot create gl shared image type");
                    std::runtime_error("cannot create gl shared image type");
                    break;
                }

                glTexImage2D(GL_TEXTURE_2D,
                    0,
                    gl_internal_format, 
                    img_size.x, 
                    img_size.y, 
                    0, 
                    gl_format,
                    type, 
                    0);
                glBindTexture(GL_TEXTURE_2D, 0);
                m_gl_texture_id = int32_t(texture);
                glFinish();
                res = cuGraphicsGLRegisterImage(&m_cu_graphics_resource, 
                    texture, GL_TEXTURE_2D, 
                    CU_GRAPHICS_REGISTER_FLAGS_SURFACE_LDST | 
                    CU_GRAPHICS_REGISTER_FLAGS_TEXTURE_GATHER);
                handleError(res, "Cuda error gl register image");
                
                res = cuGraphicsMapResources(1, 
                    &m_cu_graphics_resource, 
                    m_ctx->getCQ(0));
                handleError(res, "Cuda error gl map resource");
                res = cuGraphicsSubResourceGetMappedArray(&m_buffer,
                    m_cu_graphics_resource, 
                    0, 0);
                handleError(res, "Cuda error get mapped array");
                res = cuGraphicsUnmapResources(1, &m_cu_graphics_resource,
                    m_ctx->getCQ(0));
                handleError(res, "Cuda error unmap resource");
            }
            else
            {
                if (size.z > 1)
                {
                    CUDA_ARRAY3D_DESCRIPTOR desc;
                    memset(&desc, 0, sizeof(CUDA_ARRAY3D_DESCRIPTOR));
                    desc.Width = size.x;
                    desc.Height = size.y;
                    desc.Depth = size.z;
                    desc.Format = desc_format;
                    desc.NumChannels = desc_num_channels;
                    desc.Flags = desc_flags;
                    res = cuArray3DCreate(&m_buffer, &desc);
                }
                else
                {
                    CUDA_ARRAY_DESCRIPTOR desc;
                    memset(&desc, 0, sizeof(CUDA_ARRAY_DESCRIPTOR));
                    desc.Format = desc_format;
                    desc.Width = size.x;
                    desc.Height = size.y;
                    desc.NumChannels = desc_num_channels;
                    res = cuArrayCreate(&m_buffer, &desc);
                }
                handleError(res, "Cuda array create");
                if (error != nullptr)
                    *error = int(res);
            }

            CUDA_RESOURCE_DESC r_desc;
            memset(&r_desc, 0, sizeof(CUDA_RESOURCE_DESC));
            r_desc.resType = CU_RESOURCE_TYPE_ARRAY;
            r_desc.res.array.hArray = m_buffer;
            r_desc.flags = 0;
            CUDA_TEXTURE_DESC t_desc;
            memset(&t_desc, 0, sizeof(CUDA_TEXTURE_DESC));
            t_desc.addressMode[0] = CU_TR_ADDRESS_MODE_CLAMP; /// TODO;
            t_desc.addressMode[1] = CU_TR_ADDRESS_MODE_CLAMP; /// TODO;
            t_desc.addressMode[2] = CU_TR_ADDRESS_MODE_CLAMP; /// TODO;
            t_desc.filterMode = CU_TR_FILTER_MODE_LINEAR; /// TODO set from outside

            res = cuTexObjectCreate(&m_texture, &r_desc, &t_desc, nullptr);
            handleError(res, "Cuda Texture create!");
            res = cuSurfObjectCreate(&m_surface, &r_desc);
            handleError(res, "Cuda Surface create!");

            if (host_ptr && (mem_flag & MEM_FLAG_COPY_HOST_PTR))
            {
                mapGLImage(0);
                write(host_ptr, SYNC_MODE_BLOCKING, 0);
                unmapGLImage();
            }
            handleError(res, "CUDA Create image!");

        }

        void CUDAImg::write(const void * data,
            SyncMode block_queue,
            int32_t command_queue)
        {
            write(data, ImgRegion(0, 0, 0), img_size, block_queue, command_queue);
        }

        void CUDAImg::write(const void * data,
            const ImgRegion & origin,
            const ImgRegion & region,
            SyncMode block_queue,
            int32_t command_queue)
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
                if (block_queue == SYNC_MODE_BLOCKING)
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
                if (block_queue == SYNC_MODE_BLOCKING)
                    res = cuMemcpy2D(&copy_desc);
                else
                    res = cuMemcpy2DAsync(&copy_desc, m_ctx->getCQ(command_queue));
            }
            handleError(res, "CUDA img write");
        }

        void CUDAImg::read(void * data,
            SyncMode block_queue, 
            int32_t command_queue) const
        {
            read(data, ImgRegion(0, 0, 0), img_size, block_queue, command_queue);
        }

        void CUDAImg::read(void * data,
            const ImgRegion & origin, 
            const ImgRegion & region, 
            SyncMode block_queue,
            int32_t command_queue) const
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
                if (block_queue == SYNC_MODE_BLOCKING)
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

                if (block_queue == SYNC_MODE_BLOCKING)
                    res = cuMemcpy2D(&copy_desc);
                else
                    res = cuMemcpy2DAsync(&copy_desc, m_ctx->getCQ(command_queue));
            }
            handleError(res, "CUDA img write");
         
        }

        void * CUDAImg::map(MapMode mode, 
            SyncMode block_queue, 
            int32_t command_queue)
        {
            return map(mode, ImgRegion(0, 0, 0), img_size, block_queue, command_queue);
        }

        void * CUDAImg::map(MapMode mode,
            const ImgRegion & origin,
            const ImgRegion & region,
            SyncMode block_queue,
            int32_t command_queue)
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

        int32_t CUDAImg::getGLTextureID() const
        {
            return m_gl_texture_id;
        }

        void CUDAImg::mapGLImage(int32_t command_queue)
        {
            if (mem_flag & MEM_FLAG_CREATE_GL_SHARED)
            {
                m_gl_queue_acquired = command_queue;
                glFinish();
                auto res = cuGraphicsMapResources(1,
                    &m_cu_graphics_resource, 
                    m_ctx->getCQ(m_gl_queue_acquired));
                handleError(res , "cuda error graphics map resource!");
            }
        }

        void CUDAImg::unmapGLImage()
        {
            if (mem_flag & MEM_FLAG_CREATE_GL_SHARED)
            {
                m_ctx->finish(m_gl_queue_acquired);
                auto res = cuGraphicsUnmapResources(1, 
                    &m_cu_graphics_resource, 
                    m_ctx->getCQ(m_gl_queue_acquired));
                handleError(res, "cuda error graphics unmap resource!");
            }
        }

        CUDAImg::~CUDAImg()
        {
            cuTexObjectDestroy(m_texture);
            cuSurfObjectDestroy(m_surface);
            cuArrayDestroy(m_buffer);
        }
    }
}
