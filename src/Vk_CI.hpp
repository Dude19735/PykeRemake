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
            VkSemaphoreTypeCreateInfo data;
            VkSemaphoreTypeCreateInfo_W(VkSemaphoreType type)
            :
            data({
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
                .pNext = NULL,
                .semaphoreType = type,
                .initialValue = 0
            })
            {}
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
            VkCommandPoolCreateInfo_W(uint32_t familyIndex)
            :
            data({
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = familyIndex
            })
            {}
        };

        struct VkCommandBufferAllocateInfo_W {
            VkCommandPool vkCommandPool;
            VkCommandBufferAllocateInfo data;
            VkCommandBufferAllocateInfo_W(uint32_t n, VkCommandPool commandPool)
            :
            vkCommandPool(commandPool),
            data({
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = vkCommandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = n
            })
            {}
        };

        struct VkTimelineSemaphoreSubmitInfo_W {
            uint64_t waitValue;
            uint64_t signalValue;
            VkTimelineSemaphoreSubmitInfo data;
            VkTimelineSemaphoreSubmitInfo_W(uint64_t waitVal, uint64_t signalVal)
            : 
            waitValue(waitVal), 
            signalValue(signalVal),
            data({
                .sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
                .pNext = NULL,
                .waitSemaphoreValueCount = 1,
                .pWaitSemaphoreValues = &waitValue,
                .signalSemaphoreValueCount = 1,
                .pSignalSemaphoreValues = &signalValue
            })
            {}
        };

        struct VkSubmitInfo_W {
            std::vector<VkPipelineStageFlags> vkWaitStages;
            VkTimelineSemaphoreSubmitInfo vkTimelineInfo;
            VkCommandBuffer vkCommandBuffer;
            VkSemaphore vkWaitSemaphore;
            VkSemaphore vkSignalSemaphore;
            VkSubmitInfo data;
            VkSubmitInfo_W(VkPipelineStageFlags waitStage, VkTimelineSemaphoreSubmitInfo timelineInfo, VkCommandBuffer commandBuffer, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore)
            :
            vkWaitStages({waitStage}),
            vkTimelineInfo(timelineInfo),
            vkCommandBuffer(commandBuffer),
            data({
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .pNext = &vkTimelineInfo,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &vkWaitSemaphore,
                .pWaitDstStageMask = vkWaitStages.data(),
                .commandBufferCount = 1,
                .pCommandBuffers = &vkCommandBuffer,
                .signalSemaphoreCount  = 1,
                .pSignalSemaphores = &vkSignalSemaphore
            })
            {}
        };

        struct VkCommandBufferBeginInfo_W {
            VkCommandBufferBeginInfo data;
            VkCommandBufferBeginInfo_W(VkCommandBufferUsageFlags flag)
            : 
            data({
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .pNext = nullptr,
                .flags = flag,
                .pInheritanceInfo = nullptr
            })
            {}
        };

        /**
         * TODO:Vk_GpuTargetOp - remove implicit switch to CONCURRENT
         */
        struct VkBufferCreateInfo_W {
            std::vector<uint32_t> vkQueueFamilyIndices;
            VkBufferCreateInfo data;
            VkBufferCreateInfo_W(VkBufferUsageFlags usageFlags, VkDeviceSize size, const std::vector<uint32_t>& queueFamilyIndices)
            :
            vkQueueFamilyIndices(queueFamilyIndices),
            data({
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0, /* TODO: if we enable sparse binding and other stuff, this one needs revision */
                .size = size,
                .usage = usageFlags,
                .sharingMode = vkQueueFamilyIndices.size() == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
                .queueFamilyIndexCount = static_cast<uint32_t>(vkQueueFamilyIndices.size()),
                .pQueueFamilyIndices = vkQueueFamilyIndices.data()
            })
            {}
        };

        struct VkMemoryAllocateInfo_W {
            VkMemoryAllocateInfo data;
            VkMemoryAllocateInfo_W(VkDeviceSize allocationSize, uint32_t memoryTypeIndex)
            :
            data({
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .pNext = nullptr,
                .allocationSize = allocationSize,
                .memoryTypeIndex = memoryTypeIndex
            })
            {}
        };
    };
}