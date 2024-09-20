#pragma once

#include <string>

#include "../external/tabulate/single_include/tabulate/tabulate.hpp"

#include "Vk_DeviceLib.hpp"
#include "Vk_PhysicalDevice.hpp"
#include "Vk_Instance.hpp"

namespace VK5 {
	class Vk_Device {
	private:
		std::string _deviceName;
		Vk_DevicePreference _devicePreference;
		std::unique_ptr<Vk_Instance> _instance;
	public:
		TPhysicalDevices PhysicalDevices;

		Vk_Device(std::string deviceName, Vk_DevicePreference devicePreference = Vk_DevicePreference::USE_ANY_GPU, const std::vector<Vk_GpuOp>& opPriorities={Vk_GpuOp::Graphics, Vk_GpuOp::Transfer, Vk_GpuOp::Compute})
		:
		_deviceName(deviceName),
		_devicePreference(devicePreference),
		_instance(std::make_unique<Vk_Instance>(deviceName)),
		PhysicalDevices(_enumeratePhysicalDevices(opPriorities))
		{

		}

		~Vk_Device(){}

		void tableStream(/*out*/std::ostream& out){
			std::unordered_map<TPhysicalDeviceIndex, Vk_PhysicalDeviceLib::PhysicalDevicePR> pr;
			std::unordered_map<TPhysicalDeviceIndex, TQueueFamilies> qf;
			for(const auto& pd : PhysicalDevices){
				pr.insert({pd.first, pd.second.physicalDevicePR()});
				qf.insert({pd.first, pd.second.Queues.queueFamilies()});
			}
			Vk_PhysicalDeviceLib::tableStream(pr, qf, out);
		}

	private:
		/**
         * Enumerate all physical devices and return an unordered_map of shape
         *   std::unordered_map<TPhysicalDeviceIndex, Vk_PhysicalDevice>
         */
        TPhysicalDevices _enumeratePhysicalDevices(const std::vector<Vk_GpuOp>& opPriorities){
            TPhysicalDevices physicalDevices;
            auto vkPhysicalDevices =  Vk_PhysicalDeviceLib::enumeratePhysicalDevices(*_instance.get());

            TPhysicalDeviceIndex index = 0;
            for(VkPhysicalDevice vkPhysicalDevice : vkPhysicalDevices){
                auto capabilities = Vk_PhysicalDeviceLib::queryPhysicalDeviceCapabilities(vkPhysicalDevice);
                auto extensions = Vk_PhysicalDeviceLib::queryPhysicalDeviceExtensionsSupport(capabilities);

                physicalDevices.insert({index, Vk_PhysicalDevice(index, vkPhysicalDevice, capabilities, extensions, opPriorities)});
                index++;
            }

            return physicalDevices;
        }
	};
}
