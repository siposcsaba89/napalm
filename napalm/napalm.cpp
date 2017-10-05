#include "napalm.h"
#include <string>
#include <napalm/opencl/create.h>


napalm::Context* napalm::createContext(const char * kind, int32_t platform_id, int32_t device_id, int32_t stream_count)
{
    if (std::strcmp(kind, "OpenCL") == 0)
        return napalm::cl::createContext(platform_id, device_id, stream_count);

    return 0;
}
