#pragma once

#include <unordered_map>

#include "../Defines.h"
#include "Vk_PhysicalDeviceLib.hpp"
#include "Vk_PhysicalDeviceMemory.hpp"
#include "Vk_PhysicalDeviceQueue.hpp"
#include "Vk_LogicalDevice.hpp"
#include "Vk_LogicalDeviceQueue.hpp"
#include "./gpu_tasks/Vk_GpuTaskPool.hpp"

namespace VK5 {
    class Vk_PhysicalDevice{
    private:
        TPhysicalDeviceIndex _index;
        VkPhysicalDevice _physicalDevice;
        Vk_PhysicalDeviceLib::PhysicalDevicePR _pr;
        Vk_PhysicalDeviceQueue _physicalDeviceQueues;
        Vk_PhysicalDeviceMemory _physicalDeviceMemory;
        Vk_LogicalDevice _logicalDevice;
        Vk_LogicalDeviceQueue _logicalDeviceQueue;
        Vk_GpuTaskPool _gpuTaskPool;
    public:
        /**
         * Enumerate all available physical devices.
         * Throw a runtime exception if something goes wrong.
         */
        static std::vector<VkPhysicalDevice> enumeratePhysicalDevices(VkInstance vkInstance){
            // see how many vulkan capable devices we can find
            uint32_t deviceCount;
            VkResult res = vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
            Vk_CheckVkResult(typeid(NoneObj), res, "Failed to enumerate physical devices");

            // if we don't find any suitable devices, throw an exception because the program won't work anyways
			if (deviceCount == 0) UT::Ut_Logger::RuntimeError(typeid(NoneObj), "Failed to find devices with Vulkan support!");

            // retrieve the real physical devices into a vector
			std::vector<VkPhysicalDevice> vkPhysicalDevices(deviceCount);
			res = vkEnumeratePhysicalDevices(vkInstance, &deviceCount, vkPhysicalDevices.data());
			if(res == VK_INCOMPLETE) UT::Ut_Logger::RuntimeError(typeid(NoneObj), "Fewer GPUs than actually presend in hardware were detected by the graphics driver. Note that in the newest version of Windows, the GPU is selected per App and all GPUs are visible by default. It can be that the vendor of one of the GPUs hides the respective other GPU. This causes VK_INCOMPLETE to be returned. If you use a dual GPU system, select the GPU that does not cause these problems and select the GPU for the viewer using Vk_DevicePreference.");
			else Vk_CheckVkResult(typeid(NoneObj), res, "Failed to load physical devices");

            return vkPhysicalDevices;
        }

        Vk_PhysicalDevice(
            TPhysicalDeviceIndex index,
            VkPhysicalDevice physicalDevice,
            const Vk_PhysicalDeviceLib::PhysicalDevicePR& pr,
            const std::vector<Vk_GpuOp>& opPriorities
        )
        :
        _index(index),
        _physicalDevice(physicalDevice),
        _pr(pr),
        _physicalDeviceQueues(physicalDevice, opPriorities),
        _physicalDeviceMemory(_physicalDevice),
        _logicalDevice(_physicalDevice, _pr, _physicalDeviceQueues),
        _logicalDeviceQueue(_logicalDevice.vk_device(), _physicalDeviceQueues),
        _gpuTaskPool(_logicalDevice.vk_device())
        {}

        Vk_PhysicalDevice(const Vk_PhysicalDevice& other) = delete;
        Vk_PhysicalDevice(Vk_PhysicalDevice&& other)
        :
        _index(other._index),
        _physicalDevice(other._physicalDevice),
        _pr(std::move(other._pr)),
        _physicalDeviceQueues(std::move(other._physicalDeviceQueues)),
        _physicalDeviceMemory(std::move(other._physicalDeviceMemory)),
        _logicalDevice(std::move(other._logicalDevice)),
        _logicalDeviceQueue(std::move(other._logicalDeviceQueue)),
        _gpuTaskPool(std::move(other._gpuTaskPool))
        {
            other._physicalDevice = nullptr;
        }

        Vk_PhysicalDevice& operator=(const Vk_PhysicalDevice& other) = delete;
        Vk_PhysicalDevice& operator=(Vk_PhysicalDevice&& other) noexcept {
            if(this == &other) return *this;
            _index = other._index;
            _physicalDevice = std::move(other._physicalDevice);
            _pr = std::move(other._pr);
            _physicalDeviceQueues = std::move(other._physicalDeviceQueues);
            _physicalDeviceMemory = std::move(other._physicalDeviceMemory),
            _logicalDevice = std::move(other._logicalDevice);
            _logicalDeviceQueue = std::move(other._logicalDeviceQueue);
            _gpuTaskPool = std::move(other._gpuTaskPool);

            other._physicalDevice = nullptr;

            return *this;
        }

        ~Vk_PhysicalDevice(){}

        // Const PhysicalDevice getters
        const Vk_PhysicalDeviceLib::PhysicalDevicePR& physicalDevicePR() const { return _pr; }
        const Vk_PhysicalDeviceQueue& physicalDeviceQueues() const { return _physicalDeviceQueues; }
        const Vk_PhysicalDeviceMemory& physicalDeviceMemory() const { return _physicalDeviceMemory; }
        const Vk_LogicalDeviceQueue& logicalDeviceQueue() const { return _logicalDeviceQueue; }
        const Vk_LogicalDevice& logicalDevice() const { return _logicalDevice; }
        const Vk_HeapSize queryPhysicalDeviceHeapSize(VkMemoryPropertyFlags flags) const { return _physicalDeviceMemory.queryMemoryHeapSize(flags); }
        
        // Non const modifiers
        /**
         * Update the current stored state of all GPU heaps. 
         */
        const Vk_HeapSize queryPhysicalDeviceHeapBudget(VkMemoryPropertyFlags flags) { 
            _physicalDeviceMemory.stateUpdate();
            return _physicalDeviceMemory.queryGpuMemoryHeapBudget(flags); 
        }

        void stateUpdate() { _physicalDeviceMemory.stateUpdate(); }

        Vk_GpuTaskRunner* enqueue(std::unique_ptr<Vk_GpuTask> task){
            std::unique_ptr<Vk_Queue> queue = nullptr;
            auto op = task->opType();
            while(!queue) queue = _logicalDeviceQueue.getQueue(op);
            Vk_GpuTaskRunner* res = queue->enqueue(std::move(task));
            _logicalDeviceQueue.addQueue(op, std::move(queue));
            return res;
        }
        
        template<class TStructureType>
        void copyCpuToGpu (const TStructureType* offsetCpuMemoryPtr, VkDeviceMemory gpuMemoryPtr, std::uint64_t copyByteSize, std::uint64_t srcByteOffset, std::uint64_t dstByteOffset){
            _logicalDevice.copyCpuToGpu(offsetCpuMemoryPtr, gpuMemoryPtr, copyByteSize, srcByteOffset, dstByteOffset);
        }

        /**
         * TODO:Vk_GpuTargetOp - gpuTargetOp is currently ignored
         *    - introduce an if:
         *       1. if Vk_GpuTarget::Auto => collect all queue families and pass them all to Vk_LogicalDeviceLib::createBuffer
         *       2. if Vk_GpuTarget::Not-Auto => get the queue family that fits the Vk_GpuTargetOp the best and pass only that one to Vk_LogicalDeviceLib::createBuffer
         */
        void createAndAllocBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
			VkBuffer& buffer, VkDeviceMemory& memory, VkDeviceSize size, Vk_GpuTargetOp gpuTargetOp
		) {
            // create the buffer (NOTE the TODO point for future adjustments)
            std::vector<TQueueFamilyIndex> queueFamilies = UT::Ut_Std::umap_keys_to_vec(_logicalDeviceQueue.queueFamilies());
            THeapIndex heapIndex = _physicalDeviceMemory.queryGpuMemoryHeapIndex(memoryPropertyFlags);
            _logicalDevice.createAndAllocBuffer(usageFlags, memoryPropertyFlags, buffer, memory, size, queueFamilies, heapIndex);
		}


        // Vulkan getters
        VkPhysicalDevice vk_physicalDevice() const { return _physicalDevice; }
        VkDevice vk_logicalDevice() const { return _logicalDevice.vk_device(); }

        /**
         * TODO: testing: put into corresponding braces at some point
         */
        std::unique_ptr<Vk_Queue> getQueue(Vk_GpuOp op) { return std::move(_logicalDeviceQueue.getQueue(op)); }
        void addQueue(Vk_GpuOp op, std::unique_ptr<Vk_Queue> queue){ _logicalDeviceQueue.addQueue(op, std::move(queue)); }
    };
}