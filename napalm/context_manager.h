#pragma once
#include <string>
#include <map>
#include <napalm/napalm_export.h>

namespace napalm
{
    class Context;
    class NAPALM_EXPORT ContextManager
    {
    public:
        static ContextManager & getContextManager();
    private:
        ContextManager() = default;
    public:
        Context * getDefault(const std::string & kind = "OpenCL", int32_t platform_id = 0, int32_t device_id = 0, int32_t queue_count = 1);
        Context * getContext(const std::string & name);
        void addContext(const std::string & name, Context * ctx);
    private:
        std::map<std::string, Context *> m_ctxs;
    };
}
