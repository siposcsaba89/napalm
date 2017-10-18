#include "napalm.h"
#include <string>
//#include <napalm/opencl/create.h>
#include <Windows.h>

namespace napalm
{

    class BackendLoader
    {
        typedef PlatformAndDeviceInfo*(__cdecl *get_info_func)();
        typedef Context*(__stdcall *create_context_func)(int32_t platform_id, int32_t device_id, int32_t stream_count);
        BackendLoader(const BackendLoader &) = delete;
        BackendLoader & operator = (const BackendLoader &) = delete;
        BackendLoader(const std::string & backend)
        {
            std::string debug_ext = "";
#if _DEBUG
            debug_ext = "_d";
#endif
            std::string dll_name = (std::string("napalm_") + backend + debug_ext + ".dll");
            hGetProcIDDLL = LoadLibrary(dll_name.c_str());
            if (!hGetProcIDDLL) {
                std::cout << "could not load the dynamic library " << dll_name << std::endl;
                backend_loaded = false;
                //throw std::runtime_error("could not load the dynamic library ");
            }
            // resolve function address here
            get_info = (get_info_func)GetProcAddress(hGetProcIDDLL, "getPlatformAndDeviceInfo");
            if (!get_info) {
                DWORD a = GetLastError();
                backend_loaded = false;
                std::cout << "could not locate the function getPlatformAndDeviceInfo" << std::endl;
                //throw std::runtime_error("could not locate the function getPlatformAndDeviceInfo");
            }

            // resolve function address here
            create_context = (create_context_func)GetProcAddress(hGetProcIDDLL, "createContext");
            if (!create_context) {
                std::cout << "could not locate the function napalm::cl::createContext" << std::endl;
                backend_loaded = false;
            }

        }

        ~BackendLoader() { FreeLibrary(hGetProcIDDLL); }
    public:
        PlatformAndDeviceInfo* getPlatformAndDeviceInfo() const
        {
            return get_info();
        }

        Context* createContext(int32_t platform_id, int32_t device_id, int32_t stream_count) const
        {
            return create_context(platform_id, device_id, stream_count);
        }

        bool isLoaded() const { return backend_loaded; }
    private:
        friend class BackendManager;
        friend bool isBackendAvailable(const char *);
        get_info_func get_info = nullptr;
        create_context_func create_context = nullptr;
        bool backend_loaded = true;
        HINSTANCE hGetProcIDDLL;
    };


    class BackendManager
    {
    public:
        static BackendManager & getManager()
        {
            static BackendManager s_instance;
            return s_instance;
        }
    private:
        BackendManager() {}
        ~BackendManager() {
            for (auto & be : m_backend_loaders)
            {
                delete be.second;
                be.second = nullptr;
            }
        }
    public:
        const BackendLoader * getLoader(const std::string & backend)
        {
            auto it = m_backend_loaders.find(backend);
            if (it != m_backend_loaders.end())
                return it->second;
            else
            {
                BackendLoader * backend_loader = new BackendLoader(backend);
                if (backend_loader->isLoaded())
                    m_backend_loaders[backend] = backend_loader;
                else
                {
                    delete backend_loader;
                    backend_loader = nullptr;
                }
                return backend_loader;
            }
        }
    private:
        std::map<std::string, BackendLoader*> m_backend_loaders;
    };

    NAPALM_EXPORT bool isBackendAvailable(const char * api_type)
    {

        BackendLoader loader(api_type);
        return loader.isLoaded();
    }

    NAPALM_EXPORT PlatformAndDeviceInfo * napalm::getPlatformAndDeviceInfo(const char * api_type)
    {
        auto backend = BackendManager::getManager().getLoader(api_type);
        if (backend)
            return backend->getPlatformAndDeviceInfo();
        else
            return nullptr;
    }

    Context* napalm::createContext(const char * api_type, int32_t platform_id, int32_t device_id, int32_t stream_count)
    {
        auto backend = BackendManager::getManager().getLoader(api_type);
        if (backend)
            return backend->createContext(platform_id, device_id, stream_count);
        else
            return nullptr;
    }

    NAPALM_EXPORT void napalm::destroyContext(Context * ctx)
    {
        if (ctx->getProgramStore())
            delete ctx->getProgramStore();
        ctx->setProgramStore(nullptr);
        delete ctx;
        ctx = nullptr;
    }
}
//napalm::PlatformAndDeviceInfo::PlatformAndDeviceInfo()
//{
//}
//
//napalm::PlatformAndDeviceInfo::PlatformAndDeviceInfo(const PlatformAndDeviceInfo & other)
//{
//    *this = other;
//}

//napalm::PlatformAndDeviceInfo & napalm::PlatformAndDeviceInfo::operator=(const PlatformAndDeviceInfo & other)
//{
//    for (int i = 0; i < num_platforms; ++i)
//    {
//        for (int j = 0; j < num_devices[i]; ++j)
//        {
//            delete[] device_names[i][j];
//        }
//        delete[] platforms[i];
//        delete[] device_names[i];
//    }
//    delete[] num_devices;
//    delete[] platforms;
//    delete[] device_names;
//    num_devices = nullptr;
//    platforms = nullptr;
//    device_names = nullptr;
//    num_platforms = other.num_platforms;
//    platforms = new char*[num_platforms];
//    device_names = new char**[num_platforms];
//    num_devices = new int32_t[num_platforms];
//    memcpy(num_devices, other.num_devices, num_platforms * sizeof(int32_t));
//    for (int i = 0; i < num_platforms; ++i)
//    {
//        device_names[i] = new char *[num_devices[i]];
//        platforms[i] = new char[strlen(other.platforms[i])];
//        strcpy(platforms[i], other.platforms[i]);
//        for (int j = 0; j < num_devices[i]; ++j)
//        {
//            device_names[i][j] = new char[strlen(other.device_names[i][j])];
//            strcpy(device_names[i][j], other.device_names[i][j]);
//        }
//    }
//    return *this;
//}

//napalm::PlatformAndDeviceInfo::~PlatformAndDeviceInfo()
//{
//    for (int i = 0; i < num_platforms; ++i)
//    {
//        for (int j = 0; j < num_devices[i]; ++j)
//        {
//            delete [] device_names[i][j];
//        }
//        delete[] platforms[i];
//        delete[] device_names[i];
//    }
//    delete[] num_devices;
//    delete[] platforms;
//    delete[] device_names;
//    num_devices = nullptr;
//    platforms = nullptr;
//    device_names = nullptr;
//    num_platforms = 0;
//}
