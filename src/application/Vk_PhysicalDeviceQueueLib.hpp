#pragma once

#include <unordered_map>
#include <vector>
#include <set>

#include "../Defines.h"

namespace VK5 {
    typedef int TQueueFamilyIndex;

    enum class Vk_QueueFamilyPresentCapable {
        Undetermined,
        Yes,
        No
    };

    struct Vk_QueueFamily {
        TQueueFamilyIndex queueFamilyIndex;
        int queueCount;
        std::set<VkQueueFlagBits> flagBits;
        std::vector<Vk_GpuOp> opPriorities;
        VkExtent3D minImageTransferGranularity;
        Vk_QueueFamilyPresentCapable presentCapable;
    };
    typedef std::unordered_map<TQueueFamilyIndex, Vk_QueueFamily> TQueueFamilies;

    class Vk_PhysicalDeviceQueueLib {
    private:
        static inline const std::set<VkQueueFlagBits> _allFlagBits = {
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

        struct QueueFamilies {
            uint32_t queueFamiliesCount;
            std::vector<VkQueueFamilyProperties> queueFamilyProperties;        
        };

    public:
        struct QueueIdentifier {
            int queuFamilyIndex;
            int queueIndex;
        };

        void assignQueues(TQueueFamilies queueFamilies, std::vector<Vk_GpuOp> priorities) {
            // 0. create QueueIdentifier for each queue
            // 1. assign queues that only support one task
            // 2. sum up how many queues we have for all categories
            // Priorities: Graphics -> Transfer -> Compute (the last one is nice to have if we have enough of the other ones)
            std::vector<QueueIdentifier> compute;
            std::vector<QueueIdentifier> transfer;
            std::vector<QueueIdentifier> graphics;
            std::set<std::string> used;

            // 1. assign queues that can do only one of the priorities
            for(auto& f : queueFamilies){
                auto& family = f.second;
                // int deviceIndex = qfp.first;
                // for(auto& qf : qfp.second){
                //     int queueFamilyIndex = qf.first;
                //     for(int queueIndex=0; queueIndex<qf.second.count; ++queueIndex){
                //         const auto& x = qf.second.flags;
                //         if(x.contains(VK_QUEUE_TRANSFER_BIT) && !x.contains(VK_QUEUE_COMPUTE_BIT) && !x.contains(VK_QUEUE_GRAPHICS_BIT)){
                //             transfer.push_back(Vk_DeviceQueueLib::QueueIdentifier{.deviceIndex=deviceIndex, .queuFamilyIndex=queueFamilyIndex, .queueIndex=queueIndex});
                //             used.insert(mergeIndices(deviceIndex, queueFamilyIndex, queueIndex));
                //         }
                //         else if(!x.contains(VK_QUEUE_TRANSFER_BIT) && x.contains(VK_QUEUE_COMPUTE_BIT) && !x.contains(VK_QUEUE_GRAPHICS_BIT)){
                //             compute.push_back(Vk_DeviceQueueLib::QueueIdentifier{.deviceIndex=deviceIndex, .queuFamilyIndex=queueFamilyIndex, .queueIndex=queueIndex});
                //             used.insert(mergeIndices(deviceIndex, queueFamilyIndex, queueIndex));
                //         }
                //         else if(!x.contains(VK_QUEUE_TRANSFER_BIT) && !x.contains(VK_QUEUE_COMPUTE_BIT) && x.contains(VK_QUEUE_GRAPHICS_BIT)){
                //             graphics.push_back(Vk_DeviceQueueLib::QueueIdentifier{.deviceIndex=deviceIndex, .queuFamilyIndex=queueFamilyIndex, .queueIndex=queueIndex});
                //             used.insert(mergeIndices(deviceIndex, queueFamilyIndex, queueIndex));
                //         }
                //     }
                // }
            }

        }

        static bool queryQueueFamilyPresentCapability(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface) {
            VkBool32 presentSupport;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, &presentSupport);
            return static_cast<bool>(presentSupport);
        }

        static TQueueFamilies queryQueueFamilies(VkPhysicalDevice physicalDevice, const std::vector<Vk_GpuOp>& opPriorities){
            QueueFamilies queueFamilies = _queryAvailableQueueFamilies(physicalDevice);
            return _queryQueueFamiliesCapabilities(physicalDevice, queueFamilies, opPriorities);
        }

        static std::string queueFamilyFlagBitsSet2Str(const std::set<VkQueueFlagBits>& propBits){
            std::stringstream res;
            size_t s = propBits.size();
            int i=0; 
            for(const auto& p : propBits){
                res << _cropQueueFamilyPropertyStr(_queueFlagBits2String(p));
                if(i < s-1) res << " | ";
                i++;
            }
            return res.str();
        }

        static std::string queueFamilyOpPriorityVec2Str(const std::vector<Vk_GpuOp>& opPriorities){
            std::stringstream res;
            size_t s = opPriorities.size();
            if(s == 0) return "Unassigned";
            int i=0; 
            for(const auto& p : opPriorities){
                res << Vk_GpuOp2String(p);
                if(i < s-1) res << " > ";
                i++;
            }
            return res.str();
        }

        static std::string queueFamilyPresentCapable2Str(Vk_QueueFamilyPresentCapable queueFamilyPresentCapable) {
            switch(queueFamilyPresentCapable) {
                case Vk_QueueFamilyPresentCapable::Yes: return "Yes";
                case Vk_QueueFamilyPresentCapable::No: return "No";
                default: return "Undetermined";
            }
	    }

    private:
        static QueueFamilies _queryAvailableQueueFamilies(VkPhysicalDevice physicalDevice){
            // query device queue indices for graphics and present family
            uint32_t queueFamiliesCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, nullptr);

            QueueFamilies queueFamilies = {
                .queueFamiliesCount=queueFamiliesCount,
                .queueFamilyProperties=std::vector<VkQueueFamilyProperties>(queueFamiliesCount)
            };

            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, queueFamilies.queueFamilyProperties.data());

            return queueFamilies;
        }

        static TQueueFamilies _queryQueueFamiliesCapabilities(VkPhysicalDevice physicalDevice, const QueueFamilies& queueFamilies, const std::vector<Vk_GpuOp>& opPriorities){
            TQueueFamilies queueFamilyMap;

            for(int queueFamilyIndex=0; queueFamilyIndex < queueFamilies.queueFamiliesCount; queueFamilyIndex++){
                const VkQueueFamilyProperties& props = queueFamilies.queueFamilyProperties.at(queueFamilyIndex);
                const auto queueFlagBitsSet = _queueFamilyPropertiesSplitter(props.queueFlags);
                queueFamilyMap.insert({queueFamilyIndex, Vk_QueueFamily {
                    .queueFamilyIndex = queueFamilyIndex,
                    .queueCount = static_cast<int>(props.queueCount),
                    .flagBits = queueFlagBitsSet,
                    .opPriorities = _queueFlagBits2QueueTypePriorities(queueFlagBitsSet, opPriorities),
                    .minImageTransferGranularity = props.minImageTransferGranularity,
                    .presentCapable = Vk_QueueFamilyPresentCapable::Undetermined
                }});
            }

            return queueFamilyMap;
        }

        static std::set<VkQueueFlagBits> _queueFamilyPropertiesSplitter(VkQueueFlags flags) {
            std::set<VkQueueFlagBits> flagBits;

            for(const auto& p : _allFlagBits){
                if(flags & p){
                    flagBits.insert(p);
                }
            }

            return flagBits;
        }

        static std::vector<Vk_GpuOp> _queueFlagBits2QueueTypePriorities(const std::set<VkQueueFlagBits>& flagsSet, const std::vector<Vk_GpuOp>& priorities){
            std::vector<Vk_GpuOp> types;
            for(const auto& prio : priorities){
                Vk_GpuOp cur;
                bool found = false;
                for(const auto& flag : flagsSet){
                    switch(flag){
                        case VK_QUEUE_GRAPHICS_BIT: cur = Vk_GpuOp::Graphics; found=true; break;
                        case VK_QUEUE_COMPUTE_BIT: cur = Vk_GpuOp::Compute; found=true; break;
                        case VK_QUEUE_TRANSFER_BIT: cur = Vk_GpuOp::Transfer; found=true; break;
                    }
                    if(found && cur == prio) {
                        if(std::find(types.begin(), types.end(), cur) == types.end()) types.push_back(cur);
                        break;
                    }
                }
            }
            // to falsify some tests:
            // if(types.size() > 1){
            //     auto temp = types.at(0);
            //     types.at(0) = types.at(1);
            //     types.at(1) = temp;
            // }
            return types;
        }

        static std::string _cropQueueFamilyPropertyStr(const std::string& propStr){
            std::vector<int> inds;
            int i=0;
            for(const auto& c : propStr) {
                if(c == '_') inds.push_back(i);
                i++;
            }

            return std::string(propStr.begin()+inds.at(1)+1, propStr.begin()+inds.back());
        }

        static std::string _queueFlagBits2String(VkQueueFlagBits bits) {
            switch(bits) {
                case VK_QUEUE_GRAPHICS_BIT: return "VK_QUEUE_GRAPHICS_BIT";
                case VK_QUEUE_COMPUTE_BIT: return "VK_QUEUE_COMPUTE_BIT";
                case VK_QUEUE_TRANSFER_BIT: return "VK_QUEUE_TRANSFER_BIT";
                case VK_QUEUE_SPARSE_BINDING_BIT: return "VK_QUEUE_SPARSE_BINDING_BIT";
                case VK_QUEUE_PROTECTED_BIT: return "VK_QUEUE_PROTECTED_BIT";
                case VK_QUEUE_VIDEO_DECODE_BIT_KHR: return "VK_QUEUE_VIDEO_DECODE_BIT_KHR";
                case VK_QUEUE_VIDEO_ENCODE_BIT_KHR: return "VK_QUEUE_VIDEO_ENCODE_BIT_KHR";
                case VK_QUEUE_OPTICAL_FLOW_BIT_NV: return "VK_QUEUE_OPTICAL_FLOW_BIT_NV";
                case VK_QUEUE_FLAG_BITS_MAX_ENUM: return "VK_QUEUE_FLAG_BITS_MAX_ENU";
                default: return "UNKNOWN_VK_QUEUE";
            }
	    }

    };
}