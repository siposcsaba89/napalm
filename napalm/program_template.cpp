#include "program_template.h"
#include <cassert>

namespace napalm
{
    namespace gen
    {
        ProgramTemplate::ProgramTemplate(ProgramStore & store):
            m_store(&store)
        {
            std::map<std::string, ProgramData> program_data =
            { 
                {"OpenCL", { 
                    napalm::ProgramData::DATA_TYPE_SOURCE_DATA,
                    "kernel void test_kernel(global unsigned char * src, global unsigned char * dst, int multiplier)"
                    "{dst[get_global_id(0)] = src[get_global_id(0)] * multiplier;}",
                    0,
                    "1234"}
                },
                { "CUDA",{
                    napalm::ProgramData::DATA_TYPE_SOURCE_DATA,
                    "kernel void test_kernel(global unsigned char * src, global unsigned char * dst, int multiplier)"
                    "{dst[get_global_id(0)] = src[get_global_id(0)] * multiplier;}",
                    0,
                    "12345"}
                }
            };

            if (strcmp(m_store->getContext()->getContextKind(), "OpenCL") == 0)
            {
                auto it = program_data.find("OpenCL");
                assert(it != program_data.end() && "Program not available for OpenCL Context!");
                m_program = m_store->addProgram(it->first, it->second);
            }
        }
        ProgramTemplate::~ProgramTemplate()
        {
        }
        Kernel & ProgramTemplate::getKernel(const std::string & kernel_name)
        {
            assert(m_program != nullptr && "Invalid Program");
            return m_program->getKernel(kernel_name.c_str());
        }

        Kernel & ProgramTemplate::operator()(const std::string & kernel_name)
        {
            return getKernel(kernel_name);
        }
    }
}
