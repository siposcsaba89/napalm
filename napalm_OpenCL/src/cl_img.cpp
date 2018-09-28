#include <napalm/opencl/cl_buffer.h>
#include "cl_utils.h"
#include "../include/napalm/opencl/cl_img.h"
#include <assert.h>
#if GL_SHARING_ENABLED
#if HAVE_GLEW
#include <GL/glew.h>
#else
#include <GLES2/gl2.h>
#endif
#include <CL/cl_gl.h>
#endif


void getInternalFormat(const napalm::ImgFormat & cl_format,
    GLint &internal_format, GLenum & format, GLenum & type)
{


    switch (cl_format.data_type)
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

    switch (cl_format.img_channel_format)
    {
    case napalm::IMG_CHANNEL_FORMAT_INTENSITY:
    case napalm::IMG_CHANNEL_FORMAT_R:
        if (type == GL_FLOAT)
            internal_format = GL_R32F;
        else if (type == GL_UNSIGNED_BYTE)
            internal_format = GL_R8;
        else if (type == GL_UNSIGNED_SHORT)
            internal_format = GL_R16;
        format = GL_RED;
        break;
    case napalm::IMG_CHANNEL_FORMAT_RGBA:
        if (type == GL_FLOAT)
            internal_format = GL_RGBA32F;
        else if (type == GL_UNSIGNED_BYTE)
            internal_format = GL_RGBA;
        else if (type == GL_UNSIGNED_SHORT)
            internal_format = GL_RGBA16;
        format = GL_RGBA;
        break;
    default:
        assert(false && "cannot create gl shared image format");
        std::runtime_error("cannot create gl shared image format");
        break;
    }
}

namespace napalm
{
    namespace cl
    {
        CLImg::CLImg(const OpenCLContext * ctx, 
            ImgFormat format, 
            ImgRegion size, 
            MemFlag mem_flag, 
            void * host_ptr, 
            int32_t * error): m_ctx(ctx)
        {
            this->format = format;
            this->mem_flag = mem_flag;
            img_size = size;
            cl_int err = 0;
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
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                //specify texture dimensions, format etc
                GLint gl_internal_format = GL_RGBA;
                GLenum gl_format = GL_RGBA;
                GLenum type = GL_FLOAT;
                getInternalFormat(format, gl_internal_format, gl_format, type);

                glTexImage2D(GL_TEXTURE_2D, 0, gl_internal_format, img_size.x, img_size.y, 0, gl_format, type, 0);
                m_buffer = clCreateFromGLTexture(ctx->getCLContext(), getCLMemFlag(mem_flag), GL_TEXTURE_2D, 0, texture, &err);
                handleError(err, "OpenCL Create gl image!");
                glBindTexture(GL_TEXTURE_2D, 0);
                m_gl_texture_id = int32_t(texture);

            }
            else
            {
                cl_image_format cl_img_f;
                cl_image_desc cl_img_d;
                getCLImageForamt(format, size, cl_img_f, cl_img_d);
                m_buffer = clCreateImage(ctx->getCLContext(), getCLMemFlag(mem_flag),
                    &cl_img_f, &cl_img_d, nullptr, &err);
                if (error != nullptr)
                    *error = int(err);
                handleError(err, "OpenCL Create image!");
            }
            if (host_ptr)
            {
                mapGLImage(0);
                write(host_ptr, SYNC_MODE_BLOCKING, 0);
                unmapGLImage();
            }

        }

        void CLImg::write(const void * data, SyncMode block_queue, int32_t command_queue)
        {
            write(data, ImgRegion(0, 0, 0), img_size, block_queue, command_queue);
        }

        void CLImg::write(const void * data,
            const ImgRegion & origin,
            const ImgRegion & region,
            SyncMode block_queue, 
            int32_t command_queue)
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
            cl_int err = clEnqueueWriteImage(m_ctx->getCQ(command_queue), 
                m_buffer, 
                block_queue == SYNC_MODE_BLOCKING ? CL_TRUE : CL_FALSE,
                img_origin, img_region, 0, 0, data, 0, nullptr, nullptr);
            handleError(err, "OpenCL write image");
        }

        void CLImg::read(void * data, 
            SyncMode block_queue, 
            int32_t command_queue) const
        {
            read(data, ImgRegion(0, 0, 0), img_size, block_queue, command_queue);
        }

        void CLImg::read(void * data,
            const ImgRegion & origin, 
            const ImgRegion & region,
            SyncMode block_queue, 
            int32_t command_queue) const
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
            cl_int err = clEnqueueReadImage(m_ctx->getCQ(command_queue), 
                m_buffer, 
                block_queue == SYNC_MODE_BLOCKING ? CL_TRUE : CL_FALSE,
                img_origin,
                img_region,
                0, 0, data, 0, nullptr, nullptr);
            handleError(err, "OpenCL img create");
        }

        void * CLImg::map(MapMode mode, 
            SyncMode block_queue, 
            int32_t command_queue)
        {
            return map(mode,
                ImgRegion(0, 0, 0),
                img_size,
                block_queue,
                command_queue);
        }

        void * CLImg::map(MapMode mode, 
            const ImgRegion & origin,
            const ImgRegion & region,
            SyncMode block_queue,
            int32_t command_queue)
        {
            if ((mem_flag & MEM_FLAG_ALLOC_HOST_PTR) == 0)
            {
                handleError(-111, "Buffer was created without MEM_FLAG_ALLOC_HOST_PTR");
            }
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
            m_map_address = clEnqueueMapImage(m_ctx->getCQ(command_queue),
                m_buffer, 
                block_queue == SYNC_MODE_BLOCKING ? CL_TRUE : CL_FALSE,
                getCLMapFlag(mode), 
                img_origin, 
                img_region, 
                &image_row_pitch,
                &image_slice_pitch,
                0, nullptr, nullptr, &err);
            handleError(err, "OpenCL map image");
            return m_map_address;
        }

        void CLImg::unmap(int32_t command_queue)
        {
            if ((mem_flag & MEM_FLAG_ALLOC_HOST_PTR) == 0)
            {
                handleError(-111, "Buffer was created without MEM_FLAG_ALLOC_HOST_PTR");
            }
            cl_int err = clEnqueueUnmapMemObject(m_ctx->getCQ(command_queue), 
                m_buffer, 
                m_map_address, 
                0, 0, 0);
            handleError(err, "OpenCL unmap Image");
        }

        ArgumentPropereties CLImg::getARgumentPropereties() const
        {
            return ArgumentPropereties((void*)&m_buffer, sizeof(m_buffer));
        }
        
        ArgumentPropereties CLImg::getARgumentProperetiesWritable()
        {
            return getARgumentPropereties();
        }

        int32_t CLImg::getGLTextureID() const
        {
            return m_gl_texture_id;
        }

        void CLImg::mapGLImage(int32_t command_queue)
        {
            if (mem_flag & MEM_FLAG_CREATE_GL_SHARED)
            {
                m_gl_queue_acquired = command_queue;
                glFinish();
                cl_int err = clEnqueueAcquireGLObjects(m_ctx->getCQ(command_queue), 1, &m_buffer, 0, nullptr, nullptr);
                handleError(err, "Mapping gl image!");
            }
        }

        void CLImg::unmapGLImage()
        {
            if (mem_flag & MEM_FLAG_CREATE_GL_SHARED)
            {
                //m_ctx->finish(m_gl_queue_acquired);
                cl_int err = clEnqueueReleaseGLObjects(m_ctx->getCQ(m_gl_queue_acquired), 1, &m_buffer, 0, nullptr, nullptr);
                handleError(err, "Unmapping gl image!");
            }
        }

        CLImg::~CLImg()
        {
            clReleaseMemObject(m_buffer);
        }
    }
}
