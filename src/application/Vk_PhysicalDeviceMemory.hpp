#pragma once

#include "../Defines.h"
#include "Vk_PhysicalDeviceMemoryLib.hpp"

namespace VK5 {
    class Vk_PhysicalDeviceMemory {
    private:
        VkPhysicalDevice _physicalDevice;
        TGpuMemoryHeapState _gpuMemoryHeapState;
    public:
        Vk_PhysicalDeviceMemory(VkPhysicalDevice physicalDevice)
        :
        _physicalDevice(physicalDevice),
        _gpuMemoryHeapState(Vk_PhysicalDeviceMemoryLib::initGpuMemoryHeapState(_physicalDevice))
        {}

        Vk_PhysicalDeviceMemory(const Vk_PhysicalDeviceMemory& other) = delete;
        Vk_PhysicalDeviceMemory(Vk_PhysicalDeviceMemory&& other)
        :
        _physicalDevice(other._physicalDevice),
        _gpuMemoryHeapState(std::move(other._gpuMemoryHeapState))
        {
            other._physicalDevice = nullptr;
        }
        Vk_PhysicalDeviceMemory& operator=(const Vk_PhysicalDeviceMemory& other) = delete;
        Vk_PhysicalDeviceMemory& operator=(Vk_PhysicalDeviceMemory&& other) noexcept {
            if(this == &other) return *this;
            _physicalDevice = other._physicalDevice;
            _gpuMemoryHeapState = std::move(other._gpuMemoryHeapState);
            other._physicalDevice = nullptr;
            return *this;
        }

        void stateUpdate() {
            Vk_PhysicalDeviceMemoryLib::updateGpuHeapUsageStats(_physicalDevice, _gpuMemoryHeapState);
        }
        const TGpuMemoryHeapState& state() const { return _gpuMemoryHeapState; }
    };
}