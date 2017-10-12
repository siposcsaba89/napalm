#pragma once
#include <string>
#include <map>

namespace napalm
{
    class Context;
    class ContextManager
    {
    public:
        static ContextManager & getContextManager();
    private:
        ContextManager();
    public:
        Context * getDefault(const std::string & kind = "OpenCL", int32_t platform_id = 0, int32_t device_id = 0, int32_t queue_count = 1);
        Context * getContext(const std::string & name);
        void addContext(const std::string & name, Context * ctx);
    private:
        std::map<std::string, Context *> m_ctxs;
    };
}
