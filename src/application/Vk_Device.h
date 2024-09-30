#pragma once

#include <string>

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
		void physicalDevicesToStream(/*out*/std::ostream& outStream);
		void logicalDevicesQueuesToStream(/*out*/std::ostream& outStream);
		void physicalDevicesMemoryToStream(/*out*/std::ostream& outStream);

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

		void _physicalDevicesToStream(/*out*/std::ostream& out){
			std::unordered_map<TPhysicalDeviceIndex, Vk_PhysicalDeviceLib::PhysicalDevicePR> pr;
			std::unordered_map<TPhysicalDeviceIndex, TQueueFamilies> qf;
			for(const auto& pd : PhysicalDevices){
				pr.insert({pd.first, pd.second.physicalDevicePR()});
				qf.insert({pd.first, pd.second.physicalDeviceQueues().queueFamilies()});
			}
			Vk_PhysicalDeviceLib::physicalDevicesToStream(pr, qf, out);
		}

		void _logicalDeviceQueuesToStream(std::ostream& stream){
            tabulate::Table table;
			auto rs = tabulate::RowStream{};
            for(const auto& pd : PhysicalDevices){
				rs << std::string(pd.second.physicalDevicePR().properties.deviceName);
			}
			table.add_row(rs);
			
			auto subRs = tabulate::RowStream{};
			for(const auto& pd : PhysicalDevices){
				const auto& queuesOpMap = pd.second.logicalDeviceQueue().queuesOpMap();
				const auto& queueFamilies = pd.second.logicalDeviceQueue().queueFamilies();
				tabulate::Table opTable;
				auto opRow = tabulate::RowStream{};
				for(const auto& queues : queuesOpMap){
					opRow << Vk_GpuOp2String(queues.first);
				}

				auto opRow2 = tabulate::RowStream{};
				std::vector<int> count;
				for(const auto& queueOpFamilies : queuesOpMap){
					tabulate::Table tt;
					tt.format().hide_border();
					tt.add_row({"FI", "QI"});
					int index = 0;
					auto& op = queueOpFamilies.first;
					for(const TQueueFamilyIndex& queueFamilyIndex : queueOpFamilies.second){
						const auto& queue = queueFamilies.at(queueFamilyIndex);
						for(const auto& q : queue){
							auto qstr = q->toString();
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

			stream << table << std::endl;
        }

		void _physicalDevicesMemoryToStram(std::ostream& stream) {
			tabulate::Table table;
			table.add_row({"GPU", "Heap Size", "Usage", "Heap Config Possibilities"});
			table.format().font_style({ tabulate::FontStyle::bold })
				.border_top(" ").border_bottom(" ")
				.border_left(" ").border_right(" ")
				.corner(" ");
			table[0].format()
				.padding_top(1)
				.padding_bottom(1)
				.font_align(tabulate::FontAlign::center)
				.font_style({ tabulate::FontStyle::underline })
				.font_background_color(tabulate::Color::red);

			for(auto& pd : PhysicalDevices) pd.second.stateUpdate();

			for(const auto& pd : PhysicalDevices){
				const auto& deviceMemory = pd.second.physicalDeviceMemory();
				for (const auto& item : deviceMemory.state()) {
					std::string s_kb = Vk_Lib::rightCrop(item.size.Kb) + "[Kb]";
					std::string s_mb = Vk_Lib::rightCrop(item.size.Mb) + "[MB]";
					std::string s_gb = Vk_Lib::rightCrop(item.size.Gb) + "[GB]";

					std::string usage_kb = Vk_Lib::rightCrop(item.usage.Kb) + " / " + Vk_Lib::rightCrop(item.budget.Kb) + "[Kb]";
					std::string usage_mb = Vk_Lib::rightCrop(item.usage.Mb) + " / " + Vk_Lib::rightCrop(item.budget.Mb) + "[MB]";
					std::string usage_gb = Vk_Lib::rightCrop(item.usage.Gb) + " / " + Vk_Lib::rightCrop(item.budget.Gb) + "[GB]";

					std::stringstream props;
					size_t s = item.heapFlags.size();
					size_t counter = 0;
					for (auto conf : item.heapFlags) {
						props << conf.str;
						if(counter++ < s) props << "\n";
					}
					//std::cout << std::string(item.first->deviceProperties.deviceName) << std::endl;
					table.add_row({
						std::string(pd.second.physicalDevicePR().properties.deviceName),
						s_kb + "\n" + s_mb + "\n" + s_gb,
						usage_kb + "\n" + usage_mb + "\n" + usage_gb,
						props.str() });
				}

				table.column(0).format().font_color(tabulate::Color::red);
				table.column(1).format().font_color(tabulate::Color::blue);
				table.column(2).format().font_color(tabulate::Color::cyan);
				table.column(3).format().font_color(tabulate::Color::yellow);

				table[0][0].format().font_background_color(tabulate::Color::red).font_color(tabulate::Color::white);
				table[0][1].format().font_background_color(tabulate::Color::blue).font_color(tabulate::Color::white);
				table[0][2].format().font_background_color(tabulate::Color::cyan).font_color(tabulate::Color::blue);
				table[0][3].format().font_background_color(tabulate::Color::yellow).font_color(tabulate::Color::blue);
			}

			stream << table << std::endl;
		}
	};
}
