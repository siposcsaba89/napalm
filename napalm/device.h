#pragma once

#include <napalm/napalm_export.h>
#include <string>
#include <vector>
#include <memory>

namespace napalm
{

    class IDevice
    {
    public:
        virtual std::string getName() = 0;
        virtual ~IDevice() {}
    };


    class Device
    {
    public:
        static std::vector<Device> getDevices();
    private:
        std::shared_ptr<IDevice*> m_device;
    };

}

