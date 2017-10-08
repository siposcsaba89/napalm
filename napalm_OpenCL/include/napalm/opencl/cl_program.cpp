#include "cl_program.h"

namespace napalm
{
    namespace cl
    {
        CLProgram::CLProgram(OpenCLContext * ctx, const ProgramData & data)
        {
            switch (data.data_type)
            {
            default:
                break;
            }

            //clCreateProgramWithSource

        }
        napalm::Kernel * napalm::cl::CLProgram::getKernel(const char * kernel_name)
        {
            return nullptr;
        }
        CLProgram::~CLProgram()
        {
        }
    }
}
