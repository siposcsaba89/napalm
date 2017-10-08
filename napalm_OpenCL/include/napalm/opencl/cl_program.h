#pragma once

#include <napalm/napalm.h>
#include <napalm/opencl/opencl_context.h>

namespace napalm
{
namespace cl
{
    struct CLProgram : public Program
    {
        CLProgram(OpenCLContext * ctx, const ProgramData & data);
        virtual Kernel * getKernel(const char *  kernel_name);
        virtual ~CLProgram();
    private:
        OpenCLContext * m_ctx = nullptr;
        cl_program m_program;
    };
}
}

