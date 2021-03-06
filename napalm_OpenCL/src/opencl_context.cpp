#include "../include/napalm/opencl/opencl_context.h"
#include "../include/napalm/opencl/cl_buffer.h"
#include "../include/napalm/opencl/cl_img.h"
#include "../include/napalm/opencl/cl_program.h"
#include "cl_utils.h"
#include <memory>
#include <assert.h>
#include <vector>
#if GL_SHARING_ENABLED
#include <CL/cl_gl.h>
#endif
#include <sstream>
#if defined (__APPLE__) || defined(MACOSX)
static const std::string CL_GL_SHARING_EXT = "cl_APPLE_gl_sharing";
#else
static const std::string CL_GL_SHARING_EXT = "cl_khr_gl_sharing";
#endif


namespace napalm {
namespace cl
{
    OpenCLContext::OpenCLContext(int32_t platform_id, int32_t device_id, int32_t stream_count, napalm::GLSharedInfo * window_data) : Context()
    {
        bool  gl_sharing_supported_by_device = false;
        cl_platform_id cl_platform;
        cl_int err = 0;
        if (window_data != nullptr)
        {

            cl_uint num_platform;
            err = clGetPlatformIDs(0, NULL, &num_platform);
            handleError(err, "Context creating");
            std::vector<cl_platform_id> platform_ids(num_platform);
            if (num_platform > 0)
            {
                err = clGetPlatformIDs(num_platform, platform_ids.data(), nullptr);
                handleError(err, "Context creating");
            }
            clGetGLContextInfoKHR_fn clGetGLContextInfoKHRF = (clGetGLContextInfoKHR_fn)
                clGetExtensionFunctionAddress("clGetGLContextInfoKHR");

            if (clGetGLContextInfoKHRF == nullptr)
            {
                assert(false && "Cannot get clGetGLContextInfoKHR function pointer!");
                std::runtime_error("Cannot get clGetGLContextInfoKHR function pointer!");
            }

            for (size_t pid = 0; pid < platform_ids.size() && !gl_sharing_supported_by_device; ++pid)
            {
#ifdef WIN32
                cl_context_properties prop[] = {
                    CL_GL_CONTEXT_KHR, (cl_context_properties)window_data->gl_context,
                    CL_WGL_HDC_KHR, (cl_context_properties)window_data->native_window,
                    CL_CONTEXT_PLATFORM, (cl_context_properties)platform_ids[pid],
                    0
                };
#else
                cl_context_properties prop[] = {
                    CL_GL_CONTEXT_KHR, (cl_context_properties)window_data->gl_context,
                    CL_GLX_DISPLAY_KHR, (cl_context_properties)window_data->native_window,
                    CL_CONTEXT_PLATFORM, (cl_context_properties)platform_ids[pid],
                    0
                };

#endif
                size_t bytes = 0;
                // Notice that extension functions are accessed via pointers 
                // initialized with clGetExtensionFunctionAddressForPlatform.
                //clGetExtensionFunctionAddressForPlatform
                // queuring how much bytes we need to read
                err = (*clGetGLContextInfoKHRF)(prop, CL_DEVICES_FOR_GL_CONTEXT_KHR, 0, NULL, &bytes);
                if (err == CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR)
                    continue;
                handleError(err, "Context creating");
                // allocating the mem
                size_t devNum = bytes / sizeof(cl_device_id);
                if (devNum > 0)
                {

                    std::vector<cl_device_id> devs(devNum);
                    //reading the info
                    err = (*clGetGLContextInfoKHRF)(prop, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR, bytes, &devs[0], NULL);
                    handleError(err, "Context creating");
                    //looping over all devices
                    for (size_t i = 0; i < devNum && !gl_sharing_supported_by_device; i++)
                    {
                        // find extensions required
                        size_t ret_size = 0;
                        err = clGetDeviceInfo(devs[i], CL_DEVICE_EXTENSIONS, 0, nullptr, &ret_size);
                        handleError(err, "Context creating");
                        std::string exts;
                        exts.resize(ret_size);
                        err = clGetDeviceInfo(devs[i], CL_DEVICE_EXTENSIONS, ret_size, &exts[0], &ret_size);
                        handleError(err, "Context creating");
                        std::stringstream ss(exts);
                        std::string item;
                        while (std::getline(ss, item, ' ') && !gl_sharing_supported_by_device) 
                        {
                            if (item == CL_GL_SHARING_EXT) {
                                gl_sharing_supported_by_device = true;
                                cl_platform = platform_ids[pid];
                                m_cl_device_id = devs[i];
                                break;
                            }
                        }
                    }
                }
            }
            if (gl_sharing_supported_by_device)
            {
#ifdef WIN32
                cl_context_properties prop[] = {
                    CL_GL_CONTEXT_KHR, (cl_context_properties)window_data->gl_context,
                    CL_WGL_HDC_KHR, (cl_context_properties)window_data->native_window,
                    CL_CONTEXT_PLATFORM, (cl_context_properties)cl_platform,
                    0
                };
#else
                cl_context_properties prop[] = {
                    CL_GL_CONTEXT_KHR, (cl_context_properties)window_data->gl_context,
                    CL_GLX_DISPLAY_KHR, (cl_context_properties)window_data->native_window,
                    CL_CONTEXT_PLATFORM, (cl_context_properties)cl_platform,
                    0
                };

#endif
                m_cl_context = clCreateContext(prop, 1, &m_cl_device_id, nullptr, nullptr, &err);
                handleError(err, "Context creating");
            }
        }

        if (!gl_sharing_supported_by_device)
        {
            cl_uint num_platform;
            clGetPlatformIDs(0, NULL, &num_platform);
            std::vector<cl_platform_id> platform_ids(num_platform);
            if (num_platform > 0)
            {
                clGetPlatformIDs(num_platform, platform_ids.data(), nullptr);
                if (platform_id < 0 || platform_id >= int(num_platform))
                {
                    assert(false && "incorrect platfom id");
                    printf("Incorrect platform id %d! \n", platform_id);
                    exit(EXIT_FAILURE);
                }
                {
                    cl_platform = platform_ids[platform_id];
                    //printf("Selected platform vendor: %s %s \n",
                    //    getPlatformInfo(platform_ids, platform_id, CL_PLATFORM_VENDOR).c_str(),
                    //    getPlatformInfo(platform_ids, platform_id, CL_PLATFORM_VERSION).c_str());
                }
            }
            else
            {
                assert(false && "Not found any platform");
                printf("Not found any platform! \n");
                exit(EXIT_FAILURE);
            }
            cl_uint num_devices;
            clGetDeviceIDs(platform_ids[platform_id], CL_DEVICE_TYPE_ALL, 0, nullptr, &num_devices);
            if (num_devices == 0)
            {
                assert(false && "Not found any devices");
                printf("Not found any devices! \n");
                exit(EXIT_FAILURE);
            }
            if (device_id < 0 || device_id >= int(num_devices))
            {
                assert(false && "incorrect device id");
                printf("Incorrect device id %d! \n", device_id);
                exit(EXIT_FAILURE);
            }
            std::vector<cl_device_id> devices(num_devices);
            clGetDeviceIDs(platform_ids[platform_id], CL_DEVICE_TYPE_ALL, num_devices, devices.data(), nullptr);
            m_cl_device_id = devices[device_id];

            cl_context_properties prop[] =
            { CL_CONTEXT_PLATFORM, (cl_context_properties)platform_ids[platform_id],
                0 };
            m_cl_context = clCreateContext(prop, 1, &m_cl_device_id, nullptr, nullptr, &err);
            handleError(err, "Context creating");
        }
        printf("OpenCL context created! \n");

        m_command_queues.resize(stream_count);
        for (int32_t i = 0; i < stream_count; ++i)
        {
            m_command_queues[i] = clCreateCommandQueue(m_cl_context, m_cl_device_id, 0, &err);
            handleError(err, "ClCommandQueue creating");
        }


        printf("Selected platform vendor: %s %s \n",
            getPlatformInfo(cl_platform, CL_PLATFORM_VENDOR).c_str(),
            getPlatformInfo(cl_platform, CL_PLATFORM_VERSION).c_str());

        printf("Selected device name: %s \n",
            getDevInfo(m_cl_device_id, CL_DEVICE_NAME).c_str());

        printf("Selected device OpenCL device version: %s \n",
               getDevInfo(m_cl_device_id, CL_DEVICE_VERSION).c_str());
        printf("Selected device OpenCL C device version: %s \n",
               getDevInfo(m_cl_device_id, CL_DEVICE_OPENCL_C_VERSION).c_str());



    }

    napalm::Buffer * OpenCLContext::createBuffer(size_t size, MemFlag mem_flag, void * host_ptr, int32_t * error) const
    {
        return new CLBuffer(this, size, mem_flag, host_ptr, error);
    }

    Img * OpenCLContext::createImg(ImgFormat format, ImgRegion size, MemFlag mem_flag, void * host_ptr, int32_t * error) const
    {
        return new CLImg(this, format, size, mem_flag, host_ptr, error);
    }

    Program * OpenCLContext::createProgram(const ProgramData & data) const
    {
        return new CLProgram(this, data);
    }

    const char * OpenCLContext::getContextKind() const
    {
        return "OpenCL";
    }

    void OpenCLContext::finish(int32_t command_queue) const
    {
        cl_int err = clFinish(m_command_queues[command_queue]);
        handleError(err, "ClCommandQueue finish");
    }

    OpenCLContext::~OpenCLContext()
    {
        for (auto & cq : m_command_queues)
            clReleaseCommandQueue(cq);

        clReleaseContext(m_cl_context);
    }
    cl_context OpenCLContext::getCLContext() const
    {
        return m_cl_context;
    }
    cl_command_queue OpenCLContext::getCQ(int32_t id) const
    {
        assert(id < int(m_command_queues.size()) && id >= 0 && "Wrong OpenCL command queue id");
        return m_command_queues[id];
    }
    cl_device_id OpenCLContext::getCLDevice() const
    {
        return m_cl_device_id;
    }
}
}