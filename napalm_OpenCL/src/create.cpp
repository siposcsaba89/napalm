#include <napalm/opencl/create.h>
#include <napalm/opencl/opencl_context.h>

NAPALM_OPENCL_EXPORT napalm::Context * napalm::cl::createContext(int32_t platform_id, int32_t device_id, int32_t stream_count)
{
    return new napalm::cl::OpenCLContext(platform_id, device_id, stream_count);
}
