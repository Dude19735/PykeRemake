#pragma once

#include "../Defines.h"
#include "Vk_LogicalDeviceLib.hpp"
#include "Vk_PhysicalDeviceQueue.hpp"

namespace VK5{
    class Vk_LogicalDevice{
    private:
        VkDevice _vkDevice;
    public:
        Vk_LogicalDevice(VkPhysicalDevice physicalDevice, const Vk_PhysicalDeviceLib::PhysicalDevicePR& pr, const Vk_PhysicalDeviceQueue& physicalDeviceQueues)
        :
        _vkDevice(_createLogicalDevice(physicalDevice, pr, physicalDeviceQueues))
        {}

        Vk_LogicalDevice(Vk_LogicalDevice& other) = delete;
        Vk_LogicalDevice(Vk_LogicalDevice&& other) noexcept
        :
        _vkDevice(other._vkDevice)
        {
            other._vkDevice = nullptr;
        }

        Vk_LogicalDevice& operator=(const Vk_LogicalDevice& other) = delete;
        Vk_LogicalDevice& operator=(Vk_LogicalDevice&& other) noexcept {
            if(this == &other) return *this;
            _vkDevice = other._vkDevice;
            other._vkDevice = nullptr;
        }

        ~Vk_LogicalDevice(){
            if(_vkDevice != nullptr) vkDestroyDevice(_vkDevice, nullptr);
        }

        VkDevice vk_device() const { return _vkDevice; }

        template<class TStructureType>
        void copyCpuToGpu (const TStructureType* offsetCpuMemoryPtr, VkDeviceMemory gpuMemoryPtr, std::uint64_t copyByteSize, std::uint64_t srcByteOffset, std::uint64_t dstByteOffset){
            Vk_LogicalDeviceLib::copyCpuToGpu(_vkDevice, offsetCpuMemoryPtr, gpuMemoryPtr, copyByteSize, srcByteOffset, dstByteOffset);
        }

        void createAndAllocBuffer (
            VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
			VkBuffer& buffer, VkDeviceMemory& memory, VkDeviceSize size,
            const std::vector<TQueueFamilyIndex>& queueFamilies, THeapIndex heapIndex
		) {
            Vk_LogicalDeviceLib::createAndAllocBuffer(_vkDevice, usageFlags, memoryPropertyFlags, buffer, memory, size, queueFamilies, heapIndex);
		}

        void destroyBuffers(/*out*/std::vector<VkBuffer>&& buffers, /*out*/std::vector<VkDeviceMemory>&& memories) const{
            Vk_LogicalDeviceLib::destroyBuffers(_vkDevice, std::move(buffers), std::move(memories));
        }

        void destroyBuffer(/*out*/VkBuffer& buffers, /*out*/VkDeviceMemory& memories) const{
            Vk_LogicalDeviceLib::destroyBuffer(_vkDevice, buffers, memories);
        }

    private:
        VkDevice _createLogicalDevice(VkPhysicalDevice physicalDevice, const Vk_PhysicalDeviceLib::PhysicalDevicePR& pr, const Vk_PhysicalDeviceQueue& physicalDeviceQueues){
			// add all available device physicalDeviceQueues to the create info
            auto queueCreateInfos = Vk_LogicalDeviceLib::getDeviceQueueCreateInfo(physicalDeviceQueues.queueFamilyMap(), physicalDeviceQueues.largestQueueFamilySize());

            // create the device create info with required extensions etc
            auto deviceCreateInfo = Vk_LogicalDeviceLib::getDeviceCreateInfo(queueCreateInfos.data, pr);            

            VkDevice device;
			Vk_CheckVkResult(typeid(NoneObj), vkCreateDevice(physicalDevice, &deviceCreateInfo.data, nullptr, &device), "Failed to create logical device");

            return device;
        }
    };
}