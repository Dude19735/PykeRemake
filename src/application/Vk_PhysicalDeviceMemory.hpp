#pragma once

#include "../Defines.h"
#include "Vk_PhysicalDeviceMemoryLib.hpp"

namespace VK5 {
    class Vk_PhysicalDeviceMemory {
    private:
        VkPhysicalDevice _physicalDevice;
        TGpuMemoryHeapsState _gpuMemoryHeapsState;
    public:
        Vk_PhysicalDeviceMemory(VkPhysicalDevice physicalDevice)
        :
        _physicalDevice(physicalDevice),
        _gpuMemoryHeapsState(Vk_PhysicalDeviceMemoryLib::initGpuMemoryHeapsState(_physicalDevice))
        {}

        Vk_PhysicalDeviceMemory(const Vk_PhysicalDeviceMemory& other) = delete;
        Vk_PhysicalDeviceMemory(Vk_PhysicalDeviceMemory&& other)
        :
        _physicalDevice(other._physicalDevice),
        _gpuMemoryHeapsState(std::move(other._gpuMemoryHeapsState))
        {
            other._physicalDevice = nullptr;
        }
        Vk_PhysicalDeviceMemory& operator=(const Vk_PhysicalDeviceMemory& other) = delete;
        Vk_PhysicalDeviceMemory& operator=(Vk_PhysicalDeviceMemory&& other) noexcept {
            if(this == &other) return *this;
            _physicalDevice = other._physicalDevice;
            _gpuMemoryHeapsState = std::move(other._gpuMemoryHeapsState);
            other._physicalDevice = nullptr;
            return *this;
        }

        // non-const modifiers
        void stateUpdate() { Vk_PhysicalDeviceMemoryLib::updateGpuHeapsUsageStats(_physicalDevice, _gpuMemoryHeapsState); }

        // const getters
        const TGpuMemoryHeapsState& state() const { return _gpuMemoryHeapsState; }
        const Vk_HeapSize queryMemoryHeapSize(VkMemoryPropertyFlags memoryPropertyFlags) const { return Vk_PhysicalDeviceMemoryLib::queryGpuMemoryHeapSize(_gpuMemoryHeapsState, memoryPropertyFlags); }
        const THeapIndex queryGpuMemoryHeapIndex(VkMemoryPropertyFlags memoryPropertyFlags) const { return Vk_PhysicalDeviceMemoryLib::queryGpuMemoryHeapIndex(_gpuMemoryHeapsState, memoryPropertyFlags); }
        const Vk_HeapSize queryGpuMemoryHeapBudget(VkMemoryPropertyFlags memoryPropertyFlags) const { return Vk_PhysicalDeviceMemoryLib::queryGpuMemoryHeapBudget(_gpuMemoryHeapsState, memoryPropertyFlags); }
    };
}