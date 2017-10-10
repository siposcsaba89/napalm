#pragma once
#include <string>
#include <map>

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
        static ProgramStore * create(Context * dev_ctx);
        Program * getProgram(const std::string & name);
        Program * addProgram(const std::string & name, const ProgramData & data);
        void enableProgramCache(bool cache_enabled = true);
        const Context * getContext() const;
    private:
        ~ProgramStore();
        friend void destroyContext(Context * ctx);
        Context * m_dev_ctx = nullptr;
        std::map<std::string, Program*> m_program_map;
        bool m_program_cache_enabled = true;
    };
}
