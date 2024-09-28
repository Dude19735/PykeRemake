#include "Vk_Device.h"

namespace VK5{
    void Vk_Device::physicalDevicesToStream(/*out*/std::ostream& outStream){
        _physicalDevicesToStream(outStream);
    }

    void Vk_Device::logicalDevicesQueuesToStream(/*out*/std::ostream& outStream){
        _logicalDeviceQueuesToStream(outStream);
    }

    void Vk_Device::physicalDevicesMemoryToStream(/*out*/std::ostream& outStream){
        _physicalDevicesMemoryToStram(outStream);
    }
}