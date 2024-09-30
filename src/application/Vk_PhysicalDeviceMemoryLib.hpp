#pragma once

#include <vector>
#include <map>
#include <unordered_map>

#include "../Defines.h"
#include "../external/tabulate/single_include/tabulate/tabulate.hpp"

namespace VK5 {
    typedef uint32_t THeapIndex;

    struct Vk_GpuMemoryHeapStr {
        THeapIndex heapIndex;
        std::string str;
        std::set<VkMemoryPropertyFlagBits> flags;
        VkMemoryPropertyFlags coalescedFlags;
    };

    struct Vk_HeapSize {
        std::uint64_t size;
        double Kb;
        double Mb;
        double Gb;

        static Vk_HeapSize get(std::uint64_t total){
            double s = static_cast<double>(total);
            return Vk_HeapSize{
                .size=total,
                .Kb=Vk_Lib::round(s / 1.0e3, 2),
                .Mb=Vk_Lib::round(s / 1.0e6, 2),
                .Gb=Vk_Lib::round(s / 1.0e9, 2)
            };
        }
    };

    struct Vk_GpuMemoryHeapState {
        Vk_HeapSize size;
        Vk_HeapSize budget;
        Vk_HeapSize usage;
        THeapIndex heapIndex;
        std::vector<Vk_GpuMemoryHeapStr> heapFlags;
    };

    typedef Vk_GpuMemoryHeapState TGpuMemoryHeapState;
    typedef std::vector<TGpuMemoryHeapState> TGpuMemoryHeapsState;

    class Vk_PhysicalDeviceMemoryLib {
        /**
         * NOTE: this is to not have to iterate over all possible values of VkMemoryPropertyFlagBits
         * which is VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
         */
        static inline const  std::set<VkMemoryPropertyFlagBits> _allMemoryPropertyBits {
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
            VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
            VK_MEMORY_PROPERTY_PROTECTED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD,
            VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD,
            VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV
        };

    public:
        static Vk_HeapSize queryGpuMemoryHeapSize(const TGpuMemoryHeapsState& memoryHeapsState, VkMemoryPropertyFlags memoryPropertyFlags) {
            for(const auto& hs : memoryHeapsState){
                for(const auto& hsf : hs.heapFlags)
                if(hsf.coalescedFlags == memoryPropertyFlags) return hs.size;
            }
            UT::Ut_Logger::RuntimeError(typeid(NoneObj), "Unavailable memoryPropertyFlags [{0}] requested! Use [Vk_Device].physicalDevicesMemoryToStream(std::cout) to check all available memory configurations.", Vk_Lib::Vk_VkMemoryPropertyFlagsSet2Str(_memoryPropertiesSplitter(memoryPropertyFlags)));
            return {};
        }

        static THeapIndex queryGpuMemoryHeapIndex(const TGpuMemoryHeapsState& memoryHeapsState, VkMemoryPropertyFlags memoryPropertyFlags) {
            for(const auto& hs : memoryHeapsState){
                for(const auto& hsf : hs.heapFlags)
                if(hsf.coalescedFlags == memoryPropertyFlags) return hs.heapIndex;
            }
            UT::Ut_Logger::RuntimeError(typeid(NoneObj), "Unavailable memoryPropertyFlags [{0}] requested! Use [Vk_Device].physicalDevicesMemoryToStream(std::cout) to check all available memory configurations.", Vk_Lib::Vk_VkMemoryPropertyFlagsSet2Str(_memoryPropertiesSplitter(memoryPropertyFlags)));
            return 0;
        }

        static Vk_HeapSize queryGpuMemoryHeapBudget(const TGpuMemoryHeapsState& memoryHeapsState, VkMemoryPropertyFlags memoryPropertyFlags) {
            for(const auto& hs : memoryHeapsState){
                for(const auto& hsf : hs.heapFlags)
                if(hsf.coalescedFlags == memoryPropertyFlags) return hs.budget;
            }
            UT::Ut_Logger::RuntimeError(typeid(NoneObj), "Unavailable memoryPropertyFlags [{0}] requested! Use [Vk_Device].physicalDevicesMemoryToStream(std::cout) to check all available memory configurations.", Vk_Lib::Vk_VkMemoryPropertyFlagsSet2Str(_memoryPropertiesSplitter(memoryPropertyFlags)));
            return {};
        }

        static TGpuMemoryHeapsState initGpuMemoryHeapsState(VkPhysicalDevice physicalDevice) {
            TGpuMemoryHeapsState gpuMemoryHeap;
            VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

            std::unordered_map<uint32_t, std::vector<Vk_GpuMemoryHeapStr>> memoryHeapStr;
            for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; ++i) {
                auto h = deviceMemoryProperties.memoryTypes[i];
                if(!memoryHeapStr.contains(h.heapIndex)) memoryHeapStr.insert({h.heapIndex, std::vector<Vk_GpuMemoryHeapStr>()});
                auto flagSet = _memoryPropertiesSplitter(h.propertyFlags);
                memoryHeapStr.at(h.heapIndex).push_back({
                    .heapIndex = h.heapIndex,
                    .str = Vk_Lib::Vk_VkMemoryPropertyFlagsSet2Str(flagSet),
                    .flags = flagSet,
                    .coalescedFlags = h.propertyFlags
                });
            }

            for (uint32_t i = 0; i < deviceMemoryProperties.memoryHeapCount; ++i) {
                auto total = static_cast<uint64_t>(deviceMemoryProperties.memoryHeaps[i].size);
                gpuMemoryHeap.push_back(Vk_GpuMemoryHeapState {
                    .size=Vk_HeapSize::get(total),
                    .budget=Vk_HeapSize::get(0),
                    .usage=Vk_HeapSize::get(0),
                    .heapIndex = static_cast<uint32_t>(i),
                    .heapFlags = memoryHeapStr.at(i)
                });
            }
            return gpuMemoryHeap;
		}

        static void updateGpuHeapsUsageStats(VkPhysicalDevice physicalDevice, TGpuMemoryHeapsState& gpuMemoryHeapState) {

			VkPhysicalDeviceMemoryProperties2 props;
			props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;

			VkPhysicalDeviceMemoryBudgetPropertiesEXT next;
			next.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;
			next.pNext = VK_NULL_HANDLE;
			props.pNext = &next;

            vkGetPhysicalDeviceMemoryProperties2(physicalDevice, &props);

            for (uint32_t i = 0; i < props.memoryProperties.memoryHeapCount; ++i) {
                gpuMemoryHeapState.at(i).budget = Vk_HeapSize::get(static_cast<uint64_t>(next.heapBudget[i]));
                gpuMemoryHeapState.at(i).usage = Vk_HeapSize::get(static_cast<uint64_t>(next.heapUsage[i]));
            }
		}

    private:
        static std::set<VkMemoryPropertyFlagBits> _memoryPropertiesSplitter(VkMemoryPropertyFlags properties) {
            std::set<VkMemoryPropertyFlagBits> propertyBits;

            if(properties == VK_MEMORY_PROPERTY_HOST_LOCAL_BIT) propertyBits.insert(static_cast<VkMemoryPropertyFlagBits>(VK_MEMORY_PROPERTY_HOST_LOCAL_BIT));

            for(const auto& p : _allMemoryPropertyBits){
                if(properties & p) propertyBits.insert(p);
            }
            
            return propertyBits;
        }
    };
}