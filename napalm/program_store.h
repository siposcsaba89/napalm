#pragma once
#include <string>
#include <map>
#include <napalm/napalm_export.h>

namespace napalm
{
    class Context;
    struct  Program;
    struct ProgramData;
    class ProgramStore
    {
    private:
        ProgramStore(Context * dev_ctx);
    public:
        static NAPALM_EXPORT ProgramStore * create(Context * dev_ctx);
        NAPALM_EXPORT Program * getProgram(const std::string & name);
        NAPALM_EXPORT Program * addProgram(const std::string & name, const ProgramData & data);
        NAPALM_EXPORT void enableProgramCache(bool cache_enabled = true);
        NAPALM_EXPORT const Context * getContext() const;
    private:
        ~ProgramStore();
        friend void NAPALM_EXPORT destroyContext(Context * ctx);
        Context * m_dev_ctx = nullptr;
        std::map<std::string, Program*> m_program_map;
        bool m_program_cache_enabled = true;
    };
}
