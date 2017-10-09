#pragma once
#include <string>
#include <map>
#include "napalm.h"

namespace napalm
{
    class ProgramManager
    {
    public:
        ProgramManager(Context * dev_ctx);
        ~ProgramManager();
        Program * getProgram(const std::string & name);
        void addProgram(const std::string & name, const ProgramData & data);
        void enableProgramCache(bool cache_enabled = true);
    private:
        Context * m_dev_ctx = nullptr;
        std::map<std::string, Program*> m_program_map;
        bool m_program_cache_enabled = true;
    };
}
