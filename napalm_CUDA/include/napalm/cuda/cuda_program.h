#pragma once
#include <napalm/napalm.h>
#include <napalm/cuda/cuda_context.h>
#include <map>

namespace napalm
{
namespace cuda
{
    struct CUDAKernel : public Kernel
    {
        CUDAKernel(const CUDAContext * ctx, cl_program program, const char * kernel_name);
        virtual void setArg(int32_t idx, void * val, size_t sizeof_arg);
        virtual void setArgs(int32_t num_args, void ** argument, size_t * argument_sizes);
        virtual void execute(int32_t command_queue, ImgRegion num_blocks, ImgRegion block_sizes);
        virtual ~CUDAKernel();
    private:
        cl_kernel m_kernel;
        const CUDAContext * m_ctx = nullptr;
    };

    struct CUDAProgram : public Program
    {
        CUDAProgram(const CUDAContext * ctx, const ProgramData & data, const char * compiler_options);
        virtual Kernel & getKernel(const char *  kernel_name);
        virtual bool getStatus() const;
        virtual ProgramBinary getBinary();
        virtual ~CUDAProgram();
    private:
        bool createProgramWithSourceData(const ProgramData & data);
        bool createProgramWithSourceFile(const ProgramData & data);
        bool createProgramWithBinaryData(const ProgramData & data);
        bool createProgramWithBinaryFile(const ProgramData & data);
    private:
        const CUDAContext * m_ctx = nullptr;
        cl_program m_program;
        bool m_program_status = false;
        std::map<std::string, CUDAKernel*> m_kernels;
        std::string m_program_binary;
    };
}
}

