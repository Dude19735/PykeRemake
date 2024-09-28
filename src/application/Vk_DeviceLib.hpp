#pragma once

#include "../external/tabulate/single_include/tabulate/tabulate.hpp"
#include "Vk_PhysicalDeviceQueueLib.hpp"

namespace VK5 {
    enum class Vk_DevicePreference {
        USE_DISCRETE_GPU=0,
        USE_INTEGRATED_GPU=1,
        USE_ANY_GPU=2
    };

    class Vk_DeviceLib{
    public:
    };
}