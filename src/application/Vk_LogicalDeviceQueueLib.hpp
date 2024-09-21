#pragma once

#include "../Defines.h"
#include "../Vk_CI.hpp"
#include "Vk_PhysicalDeviceQueueLib.hpp"

namespace VK5 {
    struct Queue {
        VkQueue vkQueue;
        VkSemaphore vkTimelineSemaphore;
        uint32_t familyIndex;
        uint32_t queueIndex;
    };
    typedef TQueueSize TLogicalQueuesSize;
    typedef std::vector<Queue> TLogicalQueues;
    typedef std::unordered_map<Vk_GpuOp, std::vector<TLogicalQueues*>> TLogicalQueuesOpMap;

    class Vk_LogicalDeviceQueueLib {
    public:

        static std::unique_ptr<TLogicalQueues[]> createLogicalQueues(VkDevice device, const TDeviceQueueFamilyMap& queueFamilyMap, /*out*/TLogicalQueuesSize& logicalQueuesSize){
            // get the queues for all queue families and organize them in a stack
            std::map<TQueueFamilyIndex, TLogicalQueues> map;
            for(const auto& family : queueFamilyMap) {
                for(const auto& i : family.second) {
                    auto familyIndex = family.first;

                    Queue q;
                    q.familyIndex = familyIndex;
                    q.queueIndex = i;

                    // get the queue from the device
                    vkGetDeviceQueue(device, familyIndex, i, &q.vkQueue);

                    // create a timeline semaphore for it
                    VkSemaphoreTypeCreateInfo timelineCreateInfo = Vk_CI::VkSemaphoreTypeCreateInfo_W().data;
                    VkSemaphoreCreateInfo createInfo = Vk_CI::VkSemaphoreCreateInfo_W(timelineCreateInfo).data;
                    Vk_CheckVkResult(typeid(NoneObj), vkCreateSemaphore(device, &createInfo, NULL, &q.vkTimelineSemaphore), "Failed to create timeline semaphore!");

                    if(!map.contains(familyIndex)) map.insert({familyIndex, {}});
                    map.at(familyIndex).push_back(std::move(q));
                }
            };

            // move all stacks into an array, ordered by the queue family index
            logicalQueuesSize = map.size();
            std::unique_ptr<TLogicalQueues[]> logicalQueues = std::make_unique<TLogicalQueues[]>(logicalQueuesSize);
            size_t s=0;
            for(const auto& family : map){
                logicalQueues[s++] = std::move(family.second);
            }

            return logicalQueues;
        }

        static TLogicalQueuesOpMap createLogicalQueuesOpMap(const TQueueFamilies& queueFamilies, TLogicalQueues* logicalQueues, TLogicalQueuesSize logicalQueuesSize) {
            // order all queue stacks by operation (Graphics, Transfer, Compute) and their respective
            // priority for that operation. The result is a map that has
            // {Vk_Op1: 
            //     {0: queue family index with Vk_Op1 as priority 1},
            //      ...
            //     {N: queue family index with Vk_Op1 as priority M}
            //  Vk_Op2:
            //     {0: queue family index with Vk_Op2 as priority 1},
            //      ...
            //     {N: queue family index with Vk_Op2 as priority M}
            // }
            // All queue stacks can be assigned to multiple Vk_Ops but at most once for each priority
            std::unordered_map<Vk_GpuOp, std::map<int, std::vector<TQueueFamilyIndex>>> prioMap;
            for(const auto& family : queueFamilies) {
                auto familyIndex = family.first;
                const auto& ops = family.second.opPriorities;
                int prio = 0;
                for(const auto& op : ops) {
                    if(!prioMap.contains(op)) prioMap.insert({op, {}});
                    if(!prioMap.at(op).contains(prio)) prioMap.at(op).insert({prio, {}});
                    prioMap.at(op).at(prio).push_back(familyIndex);
                    prio++;
                }
            }

            // collect all family stacks and insert them ordered by their respective Vk_Op priority
            // in a vector and assign the vector to the Vk_Op in a map
            // {Vk_Op1: vec{[&stack of queue family with Vk_Op1 as priority 1], ..., [&stack of queue family with Vk_Op1 as priority M]}
            //   ...
            //  Vk_OpN: vec{[&stack of queue family with Vk_OpN as priority 1], ..., [&stack of queue family with Vk_OpN as priority M]}
            // }
            TLogicalQueuesOpMap queuesOpMap;
            for(const auto& pm : prioMap){
                auto& op = pm.first;
                auto& map = pm.second;
                if(!queuesOpMap.contains(op)) queuesOpMap.insert({op, {}});
                for(const auto& m : map){
                    auto& prio = m.first;
                    auto& vec = queuesOpMap.at(op);
                    for(const auto& queueFamilyIndex : m.second){
                        vec.push_back(&logicalQueues[queueFamilyIndex]);
                    }
                }
            }

            return queuesOpMap;
        }

    private:
    };
}