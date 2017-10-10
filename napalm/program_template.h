#pragma once
#include <napalm/napalm.h>
#include <map>

namespace napalm
{
    namespace gen
    {
        class ProgramTemplate
        {
        public:
            ProgramTemplate(ProgramStore & store);
            ~ProgramTemplate();
            Kernel & getKernel(const std::string & kernel_name);
            Kernel & operator()(const std::string & kernel_name);
        private:
            ProgramStore * m_store = nullptr;
            Program * m_program = nullptr;
        };
    }
}