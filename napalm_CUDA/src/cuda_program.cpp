#include "napalm/cuda/cuda_program.h"
#include "cuda_utils.h"
#include <assert.h>

#ifdef HAVE_NVRTC
#include <nvrtc.h>
#define NVRTC_SAFE_CALL(x) \
    do { \
        nvrtcResult result = x; \
        if (result != NVRTC_SUCCESS) \
        {\
            std::cerr << "\nerror: " #x " failed with error " << nvrtcGetErrorString(result) << '\n';\
            exit(1);\
        }\
    } while(0)
#endif


namespace napalm
{
    namespace cuda
    {
        CUDAProgram::CUDAProgram(const CUDAContext * ctx, const ProgramData & data, const char * compiler_options):
            m_ctx(ctx)
        {
            napalm::ProgramData::DataType dt = data.data_type;
            switch (dt)
            {
            case napalm::ProgramData::DATA_TYPE_BINARY_DATA:
                m_program_status = createProgramWithBinaryData(data);
                break;
            case napalm::ProgramData::DATA_TYPE_SOURCE_DATA:
                m_program_status = createProgramWithSourceData(data);
                break;
            case napalm::ProgramData::DATA_TYPE_BINARY_FILE_NAME:
                m_program_status = createProgramWithBinaryFile(data);
                break;
            case napalm::ProgramData::DATA_TYPE_SOURCE_FILE_NAME:
                m_program_status = createProgramWithSourceFile(data);
                break;
            default:
                break;
            }
            //CUresult res = CUDA_SUCCESS;
            //if (m_program_status)
            //{
            //    cuModuleLoadData(&m_program, )
            //    cl_device_id dev_id = m_ctx->getCLDevice();
            //    err = clBuildProgram(m_program,
            //        1,
            //        &dev_id,
            //        compiler_options,
            //        nullptr,
            //        nullptr
            //    );
            //}
            //if (err != CL_SUCCESS)
            //    m_program_status = false;
            //
            //if (err != CL_SUCCESS && (dt == napalm::ProgramData::DATA_TYPE_SOURCE_FILE_NAME || 
            //    dt == napalm::ProgramData::DATA_TYPE_SOURCE_DATA))
            //{
            //    std::string m_build_log;
            //    size_t log_size = 0;
            //    clGetProgramBuildInfo(m_program, m_ctx->getCLDevice(), CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
            //    m_build_log.resize(log_size);
            //    clGetProgramBuildInfo(m_program, m_ctx->getCLDevice(), CL_PROGRAM_BUILD_LOG, log_size, &m_build_log[0], nullptr);
            //    std::cout << m_build_log << std::endl;
            //    handleError(err, "Program build failed");
            //}
        }
        napalm::Kernel & napalm::cuda::CUDAProgram::getKernel(const char * kernel_name)
        {
            CUDAKernel *ret = nullptr;
            auto it = m_kernels.find(kernel_name);
            if (it != m_kernels.end())
            {
                ret = it->second;
            }
            else
            {
                ret = new CUDAKernel(m_ctx, m_program, kernel_name);
                m_kernels[kernel_name] = ret;
            }
            assert(ret != nullptr && "Kernel has been deleted or failed to create!");
            return *ret;
        }
        bool CUDAProgram::getStatus() const
        {
            return m_program_status;
        }
        ProgramBinary CUDAProgram::getBinary()
        {
            char * data = &m_program_binary[0];
            return ProgramBinary{data, m_program_binary.size()};
        }
        CUDAProgram::~CUDAProgram()
        {
            for (auto & k : m_kernels)
            {
                delete k.second;
                k.second = nullptr;
            }
            CUresult err = cuModuleUnload(m_program);
            handleError(err, "CL Release program!");
        }
        bool CUDAProgram::createProgramWithSourceData(const ProgramData & data)
        {
#ifdef HAVE_NVRTC
            compileRuntimeCudaKernel(data.data);
            CUresult err = CUDA_SUCCESS;
            err = cuModuleLoadData(&m_program, m_program_binary.c_str());
            bool ret = true;
            if (err != CUDA_SUCCESS)
            {
                ret = false;
            }
            handleError(err, "CUDA load Program With Binary Data!");
            return ret;
#else
            return false;
#endif
        }
        bool CUDAProgram::createProgramWithSourceFile(const ProgramData & data)
        {
#ifdef HAVE_NVRTC
            std::string file_data = loadFile(data.data, false);
            if (file_data.empty())
                return false;
            compileRuntimeCudaKernel(file_data.c_str());
            CUresult err = CUDA_SUCCESS;
            err = cuModuleLoadData(&m_program, m_program_binary.c_str());
            bool ret = true;
            if (err != CUDA_SUCCESS)
            {
                ret = false;
            }
            handleError(err, "CUDA load Program With Binary Data!");
            return ret;
#else
            return false;
#endif
        }
        bool CUDAProgram::createProgramWithBinaryData(const ProgramData & data)
        {
            bool ret = true;
            CUresult err = CUDA_SUCCESS;
            err = cuModuleLoadData(&m_program, data.data);
            if (err != CUDA_SUCCESS)
            {
                ret = false;
            }
            handleError(err, "CUDA load Program With Binary Data!");
            m_program_binary.resize(data.data_size);
            memcpy(&m_program_binary[0], data.data, data.data_size);
            return ret;

        }
        bool CUDAProgram::createProgramWithBinaryFile(const ProgramData & data)
        {
            bool ret = true;
            CUresult err = CUDA_SUCCESS;
            err = cuModuleLoad(&m_program, data.data);
            if (err != CUDA_SUCCESS)
            {
                ret = false;
            }
            handleError(err, "CUDA load Program With Binary File!");
            std::string file_data = loadFile(data.data, false);
            if (file_data.empty())
                return false;

            m_program_binary.resize(file_data.size());
            memcpy(&m_program_binary[0], file_data.data(), file_data.size());

            return ret;
        }
#ifdef HAVE_NVRTC
        void CUDAProgram::compileRuntimeCudaKernel(const char * source)
        {
            nvrtcProgram prog;
            NVRTC_SAFE_CALL(
                nvrtcCreateProgram(&prog,         // prog
                    source,         // buffer
                    "napalm cuda runtime compile",    // name
                    0,             // numHeaders
                    NULL,          // headers
                    NULL));        // includeNames
                                   // Compile the program for compute_30 with fmad disabled.
                                   //TODO
            const char *opts[] = { "--gpu-architecture=compute_50" }; //TODO from outside, opencl to and --include-path needs to be set
            nvrtcResult compileResult = nvrtcCompileProgram(prog,  // prog
                1,     // numOptions
                opts); // options
                       // Obtain compilation log from the program.
            size_t logSize;
            NVRTC_SAFE_CALL(nvrtcGetProgramLogSize(prog, &logSize));
            char *log = new char[logSize];
            NVRTC_SAFE_CALL(nvrtcGetProgramLog(prog, log));
            std::cout << log << '\n';
            delete[] log;
            if (compileResult != NVRTC_SUCCESS) {
                exit(1);
            }
            // Obtain PTX from the program.
            size_t ptxSize;
            NVRTC_SAFE_CALL(nvrtcGetPTXSize(prog, &ptxSize));
            m_program_binary.resize(ptxSize);
            NVRTC_SAFE_CALL(nvrtcGetPTX(prog, &m_program_binary[0]));
            // Destroy the program.
            NVRTC_SAFE_CALL(nvrtcDestroyProgram(&prog));
        }
#endif
        CUDAKernel::CUDAKernel(const CUDAContext * ctx, CUmodule program, const char * kernel_name) :
            m_ctx(ctx)
        {
            CUresult err = cuModuleGetFunction(&m_kernel, program, kernel_name);
            handleError(err, "Cuda Create Kernel: " + std::string(kernel_name));
        }
        void CUDAKernel::setArg(int32_t idx, void * val, size_t sizeof_arg)
        {
            m_arg_sizes[idx] = sizeof_arg;
            m_arg_ptrs[idx] = val;
        }
        void CUDAKernel::setArgs(int32_t num_args, void ** argument, size_t * argument_sizes)
        {
            for (int32_t i = 0; i < num_args; ++i)
                setArg(i, argument[i], argument_sizes[i]);
        }
        void CUDAKernel::execute(int32_t command_queue, ImgRegion num_blocks, ImgRegion block_size)
        {
            size_t global_w_offset[3] = { 0,0,0 };
            size_t global_w_size[3] = { 
                size_t(num_blocks.x * block_size.x),
                size_t(num_blocks.y * block_size.y),
                size_t(num_blocks.z * block_size.z)};
            size_t local_w_size[3] = {
                size_t(block_size.x),
                size_t(block_size.y),
                size_t(block_size.z)};

            ///TODO shared memory size
            CUresult res = cuLaunchKernel(m_kernel, num_blocks.x, num_blocks.y, num_blocks.z,
                block_size.x, block_size.y, block_size.z, 0, m_ctx->getCQ(command_queue), m_arg_ptrs, nullptr);
            handleError(res, "CU launch kernel");
        }
        CUDAKernel::~CUDAKernel()
        {
        }
    }
}
