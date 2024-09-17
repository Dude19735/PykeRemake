#pragma once

#include <unordered_map>
#include <vector>
#include <sstream>
#include <set>
#include <algorithm>

#include "../Defines.h"
#include "../external/tabulate/single_include/tabulate/tabulate.hpp"
#include "Vk_Device.hpp"
#include "Vk_Surface.hpp"

namespace VK5 {
    class Vk_DeviceQueue {
    public:

        Vk_DeviceQueue(Vk_Device* device)
        :
        _device(device),
        _deviceProps({}),
        _availableQueuesProps({})
        {
            queryAvailableQueueTypes();
            queryAvailableQueueTypesFlags();
            assignQueues();
        }

        void vk_printAvailableQueueTypes(std::ostream& stream = std::cout){
            queueFamilies2Stream(stream);
            stream << std::endl;
        }

        void vk_determineQueueFamiliesPresentCapability(const Vk_SurfaceConfig& surfaceConfig){
            determineQueueFamiliesPresentCapability(surfaceConfig);
        }

    private:
        struct QueueFamily {
            uint32_t queueFamilyCount;
            std::vector<VkQueueFamilyProperties> queueFamilyProperties;        
        };

        struct QueueFamilyProps {
            int count;
            std::set<VkQueueFlagBits> flags;
            VkExtent3D minImageTransferGranularity;
            bool presentCapable;
        };

        struct QueueIdentifier {
            int deviceIndex;
            int queuFamilyIndex;
            int queueIndex;
        };

        Vk_Device* _device;

        // {PhysicalDeviceIndex : {QueueIndex : {QueueProps_1 ... QueueProps_N}}}
        std::unordered_map<int, QueueFamily> _deviceProps;

        // {PhysicalDeviceIndex : {QueueFamilyIndex : QueueFamilyProps}}
        std::unordered_map<int, std::unordered_map<int, QueueFamilyProps>> _availableQueuesProps;

        void determineQueueFamiliesPresentCapability(const Vk_SurfaceConfig& surfaceConfig) {
            VkSurfaceKHR surf = surfaceConfig.surface->vk_surface(surfaceConfig.viewportId);
            const auto& physicalDevices = _device->vk_allPhysicalDevices();
            VkBool32 presentSupport = false;
            for(auto& qfp : _availableQueuesProps){
                VkPhysicalDevice physDev = physicalDevices.at(qfp.first).physicalDevice;
                for(auto& qf : qfp.second){
                    int queueFamilyIndex = qf.first;
                    vkGetPhysicalDeviceSurfaceSupportKHR(physDev, queueFamilyIndex, surf, &presentSupport);
                    if(presentSupport){
                        qf.second.presentCapable = true;
                    }
                }
            }
        }

        std::string mergeIndices(int deviceIndex, int queueFamilyIndex, int queueIndex) {
            return std::to_string(deviceIndex) + std::to_string(queueFamilyIndex) + std::to_string(queueIndex);
        }

        void assignSingleFunctionQueues(const std::unordered_map<int, QueueFamilyProps>& deviceQueues){

        }

        void assignQueues() {
            // Priorities: Graphics -> Transfer -> Compute (the last one is nice to have if we have enough of the other ones)
            std::vector<QueueIdentifier> compute;
            std::vector<QueueIdentifier> transfer;
            std::vector<QueueIdentifier> graphics;
            std::set<std::string> used;
            // 1. assign queues that can do only one of graphics, compute or transfer
            for(auto& qfp : _availableQueuesProps){
                int deviceIndex = qfp.first;
                for(auto& qf : qfp.second){
                    int queueFamilyIndex = qf.first;
                    for(int queueIndex=0; queueIndex<qf.second.count; ++queueIndex){
                        const auto& x = qf.second.flags;
                        if(x.contains(VK_QUEUE_TRANSFER_BIT) && !x.contains(VK_QUEUE_COMPUTE_BIT) && !x.contains(VK_QUEUE_GRAPHICS_BIT)){
                            transfer.push_back(QueueIdentifier{.deviceIndex=deviceIndex, .queuFamilyIndex=queueFamilyIndex, .queueIndex=queueIndex});
                            used.insert(mergeIndices(deviceIndex, queueFamilyIndex, queueIndex));
                        }
                        else if(!x.contains(VK_QUEUE_TRANSFER_BIT) && x.contains(VK_QUEUE_COMPUTE_BIT) && !x.contains(VK_QUEUE_GRAPHICS_BIT)){
                            compute.push_back(QueueIdentifier{.deviceIndex=deviceIndex, .queuFamilyIndex=queueFamilyIndex, .queueIndex=queueIndex});
                            used.insert(mergeIndices(deviceIndex, queueFamilyIndex, queueIndex));
                        }
                        else if(!x.contains(VK_QUEUE_TRANSFER_BIT) && !x.contains(VK_QUEUE_COMPUTE_BIT) && x.contains(VK_QUEUE_GRAPHICS_BIT)){
                            graphics.push_back(QueueIdentifier{.deviceIndex=deviceIndex, .queuFamilyIndex=queueFamilyIndex, .queueIndex=queueIndex});
                            used.insert(mergeIndices(deviceIndex, queueFamilyIndex, queueIndex));
                        }
                    }
                }
            }

        }

        void queryAvailableQueueTypes(){
            const auto& physicalDevices = _device->vk_allPhysicalDevices();

            for(const auto& pd : physicalDevices){
                _deviceProps.insert({pd.physicalDeviceIndex, {}});
            }

            for(const auto& pd : physicalDevices){
                // query device queue indices for graphics and present family
                uint32_t queueFamilyCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(
                    pd.physicalDevice,
                    &queueFamilyCount,
                    nullptr
                );

                _deviceProps.insert({
                    pd.physicalDeviceIndex, 
                    QueueFamily{
                        .queueFamilyCount=queueFamilyCount,
                        .queueFamilyProperties=std::vector<VkQueueFamilyProperties>(queueFamilyCount)
                    }
                });

                vkGetPhysicalDeviceQueueFamilyProperties(
                    pd.physicalDevice,
                    &queueFamilyCount,
                    _deviceProps.at(pd.physicalDeviceIndex).queueFamilyProperties.data()
                );
            }
        }

        void queryAvailableQueueTypesFlags() {
            const auto& physicalDevices = _device->vk_allPhysicalDevices();
            for(const auto& pd : physicalDevices){
                _availableQueuesProps.insert({pd.physicalDeviceIndex, {}});
                int queueIndex = 0;
                for(const auto& prop : pd.queueFamilyProperties){
                    auto flagBits = queueFamilyPropertiesSplitter(prop);
                    _availableQueuesProps.at(pd.physicalDeviceIndex).insert({queueIndex, QueueFamilyProps{
                        .count=static_cast<int>(prop.queueCount),
                        .flags=flagBits,
                        .minImageTransferGranularity=prop.minImageTransferGranularity,
                        .presentCapable=false // default to false, check later when surface is available
                    }});
                    queueIndex++;
                }
            }
        }

        void queueFamilies2Stream(std::ostream& stream){
            const auto& physicalDevices = _device->vk_allPhysicalDevices();

            tabulate::Table table;
            table.add_row({"GPU", "DeviceIndex", "QueueIndex", "VkQueueFamilyProperties", "#Queues", "Min Transf [w,h,d]", "Present Capable"});
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

            for(const auto& pd : _availableQueuesProps){
                for(const auto& prop : pd.second){
                    table.add_row({
                        std::string(physicalDevices.at(pd.first).deviceProperties.deviceName),
                        std::to_string(pd.first),
                        std::to_string(prop.first),
                        queueFamilyPropertiesVec2Str(prop.second.flags),
                        std::to_string(prop.second.count),
                        std::string("[") + std::to_string(prop.second.minImageTransferGranularity.width) + ","  + std::to_string(prop.second.minImageTransferGranularity.height) + "," + std::to_string(prop.second.minImageTransferGranularity.depth) + "]",
                        prop.second.presentCapable ? "Yes" : "No"
                    });
                }
            }

            table.column(0).format().font_color(tabulate::Color::red);
			table.column(1).format().font_color(tabulate::Color::blue);
            table.column(2).format().font_color(tabulate::Color::blue);
			table.column(3).format().font_color(tabulate::Color::cyan);
            table.column(4).format().font_color(tabulate::Color::cyan);
            table.column(5).format().font_color(tabulate::Color::cyan);
            table.column(6).format().font_color(tabulate::Color::cyan);

            table[0][0].format().font_background_color(tabulate::Color::red).font_color(tabulate::Color::white);
			table[0][1].format().font_background_color(tabulate::Color::blue).font_color(tabulate::Color::white);
            table[0][2].format().font_background_color(tabulate::Color::blue).font_color(tabulate::Color::white);
			table[0][3].format().font_background_color(tabulate::Color::cyan).font_color(tabulate::Color::blue);
            table[0][4].format().font_background_color(tabulate::Color::cyan).font_color(tabulate::Color::blue);
            table[0][5].format().font_background_color(tabulate::Color::cyan).font_color(tabulate::Color::blue);
            table[0][6].format().font_background_color(tabulate::Color::cyan).font_color(tabulate::Color::blue);

            stream << table;
        }

        std::set<VkQueueFlagBits> queueFamilyPropertiesSplitter(VkQueueFamilyProperties prop) {
            std::set<VkQueueFlagBits> flags;
            std::vector<VkQueueFlagBits> allProps = {
                VK_QUEUE_GRAPHICS_BIT,
                VK_QUEUE_COMPUTE_BIT,
                VK_QUEUE_TRANSFER_BIT,
                VK_QUEUE_SPARSE_BINDING_BIT,
                VK_QUEUE_PROTECTED_BIT,
                VK_QUEUE_VIDEO_DECODE_BIT_KHR,
                VK_QUEUE_VIDEO_ENCODE_BIT_KHR,
                VK_QUEUE_OPTICAL_FLOW_BIT_NV
                // VK_QUEUE_FLAG_BITS_MAX_ENUM => last one marker => ignore
            };

            for(const auto& p : allProps){
                if(prop.queueFlags & p){
                    flags.insert(p);
                }
            }

            return flags;
        }

        std::string cropQueueFamilyPropertyStr(const std::string& propStr){
            std::vector<int> inds;
            int i=0;
            for(const auto& c : propStr) {
                if(c == '_') inds.push_back(i);
                i++;
            }

            return std::string(propStr.begin()+inds.at(1)+1, propStr.begin()+inds.back());
        }

        std::string queueFamilyPropertiesVec2Str(const std::set<VkQueueFlagBits>& propBits){
            std::stringstream res;
            size_t s = propBits.size();
            int i=0; 
            for(const auto& p : propBits){
                res << cropQueueFamilyPropertyStr(Vk_VkQueueFlagBits2String(p));
                if(i < s-1) res << " | ";
                i++;
            }
            return res.str();
        }
    };
}