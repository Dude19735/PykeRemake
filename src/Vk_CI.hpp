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

        struct VkSemaphoreTypeCreateInfo_W {
            VkSemaphoreTypeCreateInfo data {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
                .pNext = NULL,
                .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
                .initialValue = 0
            };
        };

        struct VkSemaphoreCreateInfo_W {
            VkSemaphoreTypeCreateInfo semaphoreTypeCreateInfo;
            VkSemaphoreCreateInfo data;
            VkSemaphoreCreateInfo_W() = delete;
            VkSemaphoreCreateInfo_W(const VkSemaphoreTypeCreateInfo& timelineCreateInfo)
            :
            semaphoreTypeCreateInfo(timelineCreateInfo),
            data({
                .sType=VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext=&semaphoreTypeCreateInfo,
                .flags=0
            })
            {}
        };

        struct VkCommandPoolCreateInfo_W {
            VkCommandPoolCreateInfo data;
            VkCommandPoolCreateInfo_W(uint32_t familyIndex){
                data = VkCommandPoolCreateInfo {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                    .queueFamilyIndex = familyIndex
                };
            }
        };
    };
}