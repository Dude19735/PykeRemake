#pragma once

#include "Defines.h"

namespace VK5 {
    class Vk_CI {
    public:
        struct VkDeviceQueueCreateInfo_W {
            std::vector<VkDeviceQueueCreateInfo> data;
            std::vector<float> queuePriority;
        };

        struct VkDeviceCreateInfo_W {
            VkDeviceCreateInfo data;
            std::vector<const char*> deviceExtensions;
        };
    };
}