#include "../include/napalm/opencl/opencl_context.h"
#include "../include/napalm/opencl/cl_buffer.h"
#include "../include/napalm/opencl/cl_img.h"
#include "cl_utils.h"
#include <memory>
#include <assert.h>
#include <vector>


namespace napalm {
    namespace cl
    {
        OpenCLContext::OpenCLContext(int32_t platform_id, int32_t device_id, int32_t stream_count)
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
                    printf("Selected platform vendor: %s %s \n", 
                        getPlatformInfo(platform_ids, platform_id, CL_PLATFORM_VENDOR).c_str(),
                        getPlatformInfo(platform_ids, platform_id, CL_PLATFORM_VERSION).c_str());
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


            printf("Selected device name: %s \n",
                getDevInfo(devices, device_id, CL_DEVICE_NAME).c_str());

            cl_context_properties prop[] =
            { CL_CONTEXT_PLATFORM, (cl_context_properties)platform_ids[platform_id],
                0 };
            cl_int err = 0;
            m_cl_context =  clCreateContext(prop, 1, &m_cl_device_id, nullptr, nullptr, &err);
            
            handleError(err, "Context creating");
            printf("OpenCL context created! \n");

            m_command_queues.resize(stream_count);
            for (int32_t i = 0; i < stream_count; ++i)
            {
                m_command_queues[i] = clCreateCommandQueue(m_cl_context, m_cl_device_id, 0, &err);
                handleError(err, "ClCommandQueue creating");
            }
        }

        napalm::Buffer * OpenCLContext::createBuffer(size_t size, MemFlag mem_flag, void * host_ptr, int32_t * error)
        {
            return new CLBuffer(this, size, mem_flag, host_ptr, error);
        }

        Img * OpenCLContext::createImg(ImgFormat format, ImgRegion size, MemFlag mem_flag, void * host_ptr, int32_t * error)
        {
            return new CLImg(this, format, size, mem_flag, host_ptr, error);
        }

		Program * OpenCLContext::createProgram(const ProgramData & data)
		{
			return nullptr;
		}

        const char * OpenCLContext::getContextKind()
        {
            return "OpenCL";
        }
        
         void OpenCLContext::finish(int32_t command_queue)
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
        cl_context OpenCLContext::getCLContext()
        {
            return m_cl_context;
        }
        cl_command_queue OpenCLContext::getCQ(int32_t id)
        {
            assert(id < int(m_command_queues.size()) && id >= 0 && "Wrong OpenCL command queue id");
            return m_command_queues[id];
        }
    }
}