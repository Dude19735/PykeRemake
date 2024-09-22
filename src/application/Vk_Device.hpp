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
		{}

		~Vk_Device(){}

		void physicalDevicesToStream(/*out*/std::ostream& out){
			std::unordered_map<TPhysicalDeviceIndex, Vk_PhysicalDeviceLib::PhysicalDevicePR> pr;
			std::unordered_map<TPhysicalDeviceIndex, TQueueFamilies> qf;
			for(const auto& pd : PhysicalDevices){
				pr.insert({pd.first, pd.second.physicalDevicePR()});
				qf.insert({pd.first, pd.second.physicalDeviceQueues().queueFamilies()});
			}
			Vk_PhysicalDeviceLib::physicalDevicesToStream(pr, qf, out);
		}

		void logicalDeviceQueuesToStream(std::ostream& stream){
            tabulate::Table table;
			auto rs = tabulate::RowStream{};
            for(const auto& pd : PhysicalDevices){
				rs << std::string(pd.second.physicalDevicePR().properties.deviceName);
			}
			table.add_row(rs);
			
			auto subRs = tabulate::RowStream{};
			for(const auto& pd : PhysicalDevices){
				const auto& queueOpMap = pd.second.logicalDeviceQueues().queuesOpMap();
				tabulate::Table opTable;
				auto opRow = tabulate::RowStream{};
				for(const auto& queues : queueOpMap){
					opRow << Vk_GpuOp2String(queues.first);
				}

				auto opRow2 = tabulate::RowStream{};
				std::vector<int> count;
				for(const auto& queues : queueOpMap){
					tabulate::Table tt;
					tt.format().hide_border();
					tt.add_row({"FI", "QI"});
					int index = 0;
					for(const auto& queue : queues.second){
						for(const auto& q : *queue){
							auto qstr = Queue::toString(q);
							int ind = qstr.find('|');
							tt.add_row({qstr.substr(0, ind), qstr.substr(ind+1, qstr.size() - ind-1) });
							index++;
						}
					}
					count.push_back(index);
					opRow2 << tt;
				}

				auto opRow3 = tabulate::RowStream{};
				for(const auto& c : count){
					opRow3 << std::to_string(c);
				}

				opTable.add_row(opRow);
				opTable[0].format().font_align(tabulate::FontAlign::center);
				opTable.add_row(opRow2);
				opTable.add_row(opRow3);
				opTable[2].format().font_align(tabulate::FontAlign::center);
				subRs << opTable;
			}
			table.add_row(subRs);

			stream << table << std::endl;;
        }

	private:
		/**
         * Enumerate all physical devices and return an unordered_map of shape
         *   std::unordered_map<TPhysicalDeviceIndex, Vk_PhysicalDevice>
         */
        TPhysicalDevices _enumeratePhysicalDevices(const std::vector<Vk_GpuOp>& opPriorities){
            TPhysicalDevices physicalDevices;
            auto vkPhysicalDevices =  Vk_PhysicalDevice::enumeratePhysicalDevices(_instance->vk_instance());

            TPhysicalDeviceIndex index = 0;
            for(VkPhysicalDevice vkPhysicalDevice : vkPhysicalDevices){
                auto physicalDevicePr = Vk_PhysicalDeviceLib::queryPhysicalDevicePr(vkPhysicalDevice);

                physicalDevices.insert({index, Vk_PhysicalDevice(index, vkPhysicalDevice, physicalDevicePr, opPriorities)});
                index++;
            }

            return physicalDevices;
        }
	};
}
