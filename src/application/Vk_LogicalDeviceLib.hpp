#pragma once

#include "../Defines.h"
#include "../Vk_CI.hpp"
#include "Vk_PhysicalDeviceLib.hpp"

namespace VK5{
    class Vk_LogicalDeviceLib{
    public:
        static Vk_CI::VkDeviceQueueCreateInfo_W getDeviceQueueCreateInfo(const TDeviceQueueFamilyMap& familyMap, int largestFamily){
            Vk_CI::VkDeviceQueueCreateInfo_W w;
            w.queuePriority = std::vector<float>(largestFamily, 1.0f);
			for (const auto& uqf : familyMap) {
				VkDeviceQueueCreateInfo queueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
				queueCreateInfo.queueFamilyIndex = uqf.first;
				queueCreateInfo.queueCount = static_cast<uint32_t>(uqf.second.size());
				queueCreateInfo.pQueuePriorities = w.queuePriority.data();
				w.data.push_back(queueCreateInfo);
            }

            return w;
        }

        static Vk_CI::VkDeviceCreateInfo_W getDeviceCreateInfo(const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos, const Vk_PhysicalDeviceLib::PhysicalDevicePR& pr){
            // NOTE: this one actually **allocates** the queues too => need to get all of them with vkGetDeviceQueue
            Vk_CI::VkDeviceCreateInfo_W w = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
			w.data.pQueueCreateInfos = queueCreateInfos.data();
			w.data.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			w.data.pEnabledFeatures = &pr.v10features;
            w.data.pNext = &pr.v12features;

            if(pr.extensionSupport.memoryBudget) w.deviceExtensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
            if(pr.extensionSupport.swapchain) w.deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
            if(pr.extensionSupport.timelineSemaphore) w.deviceExtensions.push_back(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

			w.data.enabledExtensionCount = static_cast<uint32_t>(w.deviceExtensions.size());
			w.data.ppEnabledExtensionNames = w.deviceExtensions.data();

            return w;
        }

        template<class T_StructureType>
        static void copyCpuToGpu(
            VkDevice vkDevice,
            const T_StructureType* offsetCpuMemoryPtr,
            VkDeviceMemory gpuMemoryPtr, 
            std::uint64_t copyByteSize, std::uint64_t srcByteOffset, std::uint64_t dstByteOffset
        ){
			void* data;
			vkMapMemory(vkDevice, gpuMemoryPtr, static_cast<VkDeviceSize>(dstByteOffset), static_cast<VkDeviceSize>(copyByteSize), 0, &data);
			memcpy(data, static_cast<const void*>(offsetCpuMemoryPtr), static_cast<size_t>(copyByteSize));
			vkUnmapMemory(vkDevice, gpuMemoryPtr);
        }

        static VkBuffer createBuffer(VkDevice vkDevice, VkBufferUsageFlags usageFlags, VkDeviceSize size, const std::vector<TQueueFamilyIndex>& queueFamilyIndices){
            VkBuffer buffer;
            auto bufferCreateInfo = Vk_CI::VkBufferCreateInfo_W(usageFlags, size, queueFamilyIndices).data;
			Vk_CheckVkResult(typeid(NoneObj), vkCreateBuffer(vkDevice, &bufferCreateInfo, nullptr, &buffer), "Unable to create buffer");
            return buffer;
        }

        static void destroyBuffers(VkDevice vkDevice, std::vector<VkBuffer>&& buffers, std::vector<VkDeviceMemory>&& buffersMemory){
            for(int i=0; i<buffers.size(); ++i) destroyBuffer(vkDevice, buffers.at(i), buffersMemory.at(i));
			buffers.clear(); buffersMemory.clear();
        }

        static void destroyBuffer(VkDevice vkDevice, VkBuffer buffer, VkDeviceMemory memory){
            if(buffer != nullptr) vkDestroyBuffer(vkDevice, buffer, nullptr);
			if(memory != nullptr) vkFreeMemory(vkDevice, memory, nullptr);
        }

        static VkDeviceMemory allocBuffer(VkDevice vkDevice, VkDeviceSize size, THeapIndex heapIndex){
            VkDeviceMemory memory;
            // Create the memory backing up the buffer handle
			auto memAllocInfo = Vk_CI::VkMemoryAllocateInfo_W(size, heapIndex).data;
			VkResult res = vkAllocateMemory(vkDevice, &memAllocInfo, nullptr, &memory);
			if (res != VK_SUCCESS) {
				if (res == VK_ERROR_OUT_OF_DEVICE_MEMORY) throw OutOfDeviceMemoryException();
				else UT::Ut_Logger::RuntimeError(typeid(NoneObj), "Failed to allocate buffer memory!");
			}
            return memory;
        }

        static void createAndAllocBuffer(
            VkDevice vkDevice,
            VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
			VkBuffer& buffer, VkDeviceMemory& memory, VkDeviceSize size,
            const std::vector<TQueueFamilyIndex>& queueFamilies, THeapIndex heapIndex
		) {
            buffer = createBuffer(vkDevice, usageFlags, size, queueFamilies);
            memory = allocBuffer(vkDevice, size, heapIndex);
            /**
             * TODO: check what this one does => it's probably only a mechanism to decouple buffer creation and allocation
             */
            VkMemoryRequirements memReqs;
            vkGetBufferMemoryRequirements(vkDevice, buffer, &memReqs);
    		Vk_CheckVkResult(typeid(NoneObj), vkBindBufferMemory(vkDevice, buffer, memory, 0), "Unable to bind buffer memory to device");
		}
    };
}