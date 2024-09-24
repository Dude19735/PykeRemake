#pragma once

#include "../Defines.h"
#include "../Vk_CI.hpp"

namespace VK5 {
class Vk_Queue {
    private:
        VkDevice _vkDevice;
        VkQueue _vkQueue;
        VkSemaphore _vkTimelineSemaphore;
        VkCommandPool _vkCommandPool;
        TQueueFamilyIndex _familyIndex;
        TQueueIndex _queueIndex;

    public:
        // regular constructor
        Vk_Queue(VkDevice vkDevice, uint32_t familyIndex, uint32_t queueIndex) 
        : 
        _vkDevice(vkDevice), _vkQueue(nullptr), _vkTimelineSemaphore(nullptr), _vkCommandPool(nullptr), _familyIndex(familyIndex), _queueIndex(queueIndex)
        {
            // get the queue from the device
            vkGetDeviceQueue(_vkDevice, _familyIndex, _queueIndex, &_vkQueue);

            // create a timeline semaphore for it
            VkSemaphoreTypeCreateInfo timelineCreateInfo = Vk_CI::VkSemaphoreTypeCreateInfo_W().data;
            VkSemaphoreCreateInfo createInfo = Vk_CI::VkSemaphoreCreateInfo_W(timelineCreateInfo).data;
            Vk_CheckVkResult(typeid(NoneObj), vkCreateSemaphore(_vkDevice, &createInfo, NULL, &_vkTimelineSemaphore), "Failed to create timeline semaphore!");

            // create command pool for the device graphics queue
            auto cmCreateInfo = Vk_CI::VkCommandPoolCreateInfo_W(familyIndex).data;
            Vk_CheckVkResult(typeid(NoneObj), vkCreateCommandPool(_vkDevice, &cmCreateInfo, nullptr, &_vkCommandPool), std::string("Unable to create command pool for queue ") + Vk_Queue::toString());
        }

        Vk_Queue(const Vk_Queue& other) = delete;
        Vk_Queue(Vk_Queue&& other) = delete;
        Vk_Queue& operator=(const Vk_Queue& other) = delete;
        Vk_Queue& operator=(Vk_Queue&& other) = delete;

        ~Vk_Queue(){
            if(_vkQueue != nullptr) vkQueueWaitIdle(_vkQueue);
            if(_vkTimelineSemaphore != nullptr) vkDestroySemaphore(_vkDevice, _vkTimelineSemaphore, nullptr);
            if(_vkCommandPool != nullptr) vkDestroyCommandPool(_vkDevice, _vkCommandPool, nullptr);
        }

        const TQueueFamilyIndex familyIndex() const { return _familyIndex; }
        const TQueueIndex queueIndex() const { return _queueIndex; }

        std::string toString() const {
            std::stringstream ss;
            ss << _familyIndex << "|" << _queueIndex;
            return ss.str();
        }
    };
}