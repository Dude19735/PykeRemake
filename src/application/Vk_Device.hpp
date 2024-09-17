#pragma once

#include <string>

#include "Vk_DeviceLib.hpp"

namespace VK5 {
	class Vk_Device {
	public:
		Vk_Device(std::string deviceName, Vk_DevicePreference devicePreference = Vk_DevicePreference::USE_ANY_GPU){

		}
		~Vk_Device(){}
	};
}
