#include "context_manager.h"
#include <cassert>
#include "napalm.h"

namespace napalm
{
    ContextManager & ContextManager::getContextManager()
    {
        static ContextManager s_manger;
        return s_manger;
    }
    Context * ContextManager::getContext(const std::string & name)
    {
        auto it = m_ctxs.find(name);
        if (it != m_ctxs.end())
            return it->second;
        else
        {
            printf("Please create %s named context before use!", name.c_str());
            assert(false && "Context does not exists!!!");
        }
    }
    void ContextManager::addContext(const std::string & name, Context * ctx)
    {
        auto it = m_ctxs.find(name);
        if (it == m_ctxs.end())
            m_ctxs[name] = ctx;
        else
        {
            printf("Context named %s arleady exists!", name.c_str());
            //assert(false && "Context does not exists!!!");
        }
    }
    Context * ContextManager::getDefault(const std::string & backend, int32_t platform_id, int32_t device_id, int32_t queue_count)
    {
        auto it = m_ctxs.find("default");
        if (it != m_ctxs.end())
            return it->second;
        else
        {
            Context * ctx = createContext(backend.c_str(), platform_id, device_id, queue_count);
            m_ctxs["default"] = ctx;
            return ctx;
        }
    }
}
