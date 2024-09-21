#pragma once

#include <unordered_map>
#include <vector>
#include <set>

#include "../Defines.h"

namespace VK5 {
    typedef uint32_t TQueueIndex;
    typedef TQueueIndex TQueueSize;
    typedef uint32_t TQueueFamilyIndex;
    typedef std::unordered_map<TQueueFamilyIndex, std::vector<TQueueIndex>> TDeviceQueueFamilyMap;

    enum class Vk_QueueFamilyPresentCapable {
        Undetermined,
        Yes,
        No
    };

    struct Vk_QueueFamily {
        TQueueFamilyIndex queueFamilyIndex;
        TQueueSize queueCount;
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

        static std::set<Vk_GpuOp> getSupportedOpTypes(const TQueueFamilies& queueFamilies){
            std::set<Vk_GpuOp> res;
            for(const auto& f : queueFamilies){
                const auto& family = f.second;
                for(const auto& op : family.opPriorities){
                    res.insert(op);
                }
            }
            return res;
        }

        static TDeviceQueueFamilyMap getDeviceQueueFamilyMap(const TQueueFamilies& queueFamilies, /*out*/TQueueSize& largestFamily){
            // get map to find all necessary queues (queues with skills that fit some opPriority)
            TDeviceQueueFamilyMap uniqueQueueFamilies;
            largestFamily = 0;
            for(const auto& f : queueFamilies){
                auto& family = f.second;
                if(family.opPriorities.size() == 0) continue;
                if(family.queueCount > largestFamily) largestFamily = family.queueCount;
                uniqueQueueFamilies.insert({family.queueFamilyIndex, UT::Ut_Std::vec_range(0U, family.queueCount)});
            }
            return uniqueQueueFamilies;
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

            for(TQueueFamilyIndex queueFamilyIndex=0; queueFamilyIndex < queueFamilies.queueFamiliesCount; queueFamilyIndex++){
                const VkQueueFamilyProperties& props = queueFamilies.queueFamilyProperties.at(queueFamilyIndex);
                const auto queueFlagBitsSet = _queueFamilyPropertiesSplitter(props.queueFlags);
                queueFamilyMap.insert({queueFamilyIndex, Vk_QueueFamily {
                    .queueFamilyIndex = queueFamilyIndex,
                    .queueCount = static_cast<TQueueSize>(props.queueCount),
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