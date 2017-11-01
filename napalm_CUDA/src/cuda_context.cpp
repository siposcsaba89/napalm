#include "../include/napalm/cuda/cuda_context.h"
#include "../include/napalm/cuda/cuda_buffer.h"
#include "../include/napalm/cuda/cuda_img.h"
#include "../include/napalm/cuda/cuda_program.h"
#include "cuda_utils.h"
#include <memory>
#include <assert.h>
#include <vector>


namespace napalm {
    namespace cuda
    {
        CUDAContext::CUDAContext(int32_t platform_id, int32_t device_id, int32_t stream_count) : Context()
        {
            CUresult res = cuInit(0);
            napalm::cuda::handleError(res, "Cu init");
            {
                if (platform_id < 0 || platform_id >= int(2))
                {
                    assert(false && "incorrect platfom id");
                    printf("Incorrect platform id %d! \n", platform_id);
                    exit(EXIT_FAILURE);
                }
                {
                    int version = 0;
                    CUresult err = cuDriverGetVersion(&version);
                    handleError(err, "Cuda get driver version");
                    std::string platform_name =
                        "NVidia" + std::string(" Version:") +
                        std::to_string(version);
                    printf("Selected platform vendor: %s %s \n", 
                        "NVidia Version: ",
                        std::to_string(version).c_str());
                }
            }
            int num_devices;
            res =  cuDeviceGetCount(&num_devices);
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
            res = cuDeviceGet(&m_cuda_device_id, device_id);
            handleError(res, "Cuda get device");

            std::string dev_name;
            dev_name.resize(1024);
            res = cuDeviceGetName(&dev_name[0], int(dev_name.size()), m_cuda_device_id);
            handleError(res, "Cuda get device name");
            printf("Selected device name: %s \n",
                dev_name.c_str());

            res = cuCtxCreate(&m_cuda_context, CU_CTX_MAP_HOST | CU_CTX_SCHED_BLOCKING_SYNC, m_cuda_device_id);
            handleError(res, "Cuda ctx create");


            printf("CUDA context created! \n");

            registerContext();
            m_command_queues.resize(stream_count);
            for (int32_t i = 0; i < stream_count; ++i)
            {
                res = cuStreamCreate(&m_command_queues[i], CU_STREAM_NON_BLOCKING);
                handleError(res, "CUDACommandQueue creating");
            }
        }

        napalm::Buffer * CUDAContext::createBuffer(size_t size, MemFlag mem_flag, void * host_ptr, int32_t * error) const
        {
            return new CUDABuffer(this, size, mem_flag, host_ptr, error);
        }

        Img * CUDAContext::createImg(ImgFormat format, ImgRegion size, MemFlag mem_flag, void * host_ptr, int32_t * error) const
        {
            return new CUDAImg(this, format, size, mem_flag, host_ptr, error);
        }

        Program * CUDAContext::createProgram(const ProgramData & data) const
        {
            return new CUDAProgram(this, data);
        }

        const char * CUDAContext::getContextKind() const
        {
            return "CUDA";
        }
        
         void CUDAContext::finish(int32_t command_queue) const
         {
            CUresult res = cuStreamSynchronize(m_command_queues[command_queue]);
            handleError(res, "CUDACommandQueue finish");
         }

         void CUDAContext::registerContext() const
         {
             CUresult res = cuCtxSetCurrent(m_cuda_context);
             handleError(res, "CUDA register context");
         }
        
        CUDAContext::~CUDAContext()
        {
            for (auto & cq : m_command_queues)
                cuStreamDestroy(cq);

            cuCtxDestroy(m_cuda_context);
        }
        
        CUcontext CUDAContext::getCLContext() const
        {
            return m_cuda_context;
        }
        
        CUstream CUDAContext::getCQ(int32_t id) const
        {
            assert(id < int(m_command_queues.size()) && id >= 0 && "Wrong OpenCL command queue id");
            return m_command_queues[id];
        }
        
        CUdevice CUDAContext::getCUDADevice() const
        {
            return m_cuda_device_id;
        }
    }
}