#pragma once

#include "../Defines.h"
#include "../external/tabulate/single_include/tabulate/tabulate.hpp"
#include "../Vk_CI.hpp"
#include "Vk_PhysicalDeviceQueueLib.hpp"

namespace VK5 {
    struct Queue {
        VkQueue vkQueue;
        VkSemaphore vkTimelineSemaphore;
        uint32_t familyIndex = 42;
        uint32_t queueIndex = 42;

        static std::string toString(const Queue& q){
            std::stringstream ss;
            // ss << std::setw(2) << std::setfill('0') << q.familyIndex << "|" << std::setw(2) << std::setfill('0') << q.queueIndex;
            ss << q.familyIndex << "|" << q.queueIndex;
            return ss.str();
        }
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

        static TLogicalQueuesOpMap createLogicalQueuesOpMap(const TDeviceQueueFamilyMap& queueFamilyMap, const TQueueFamilies& queueFamilies, TLogicalQueues* logicalQueues, TLogicalQueuesSize logicalQueuesSize) {
            // check if we have a situation, for example with Compute > Graphics > Transfer. We may get
            // that we have X queues that can do Compute and Graphics and Transfer, Y queues that can do Compute and Transfer
            // and Z queues that can only do Transfer. Assume, prioMap looks as follows:
            // Compute | Graphics | Transfer
            //    X          X         X
            //    Y                    Y
            //                         Z
            // We would like to order this list such that Compute has Y as highest priority and Transfer has Z so that we still
            // fully benefit from X being the only queue family that can do Graphics.
            // Generally, all queue families can to Transfer, most can do Compute and only one family can do Graphics
            // Sorting by the number of supported Vk_GpuOp will approximate this result (at least on my RTX 3080) and as long
            // as we only support Graphics, Compute and Transfer.
            // Future-TODO: if we add stuff like sparse binding etc...
            std::map<int, std::vector<TQueueFamilyIndex>> sortMap;
            for(const auto& family : queueFamilyMap) {
                auto familyIndex = family.first;
                const auto& ops = queueFamilies.at(familyIndex).opPriorities;
                if(!sortMap.contains(ops.size())) sortMap.insert({ops.size(), {}});
                sortMap.at(ops.size()).push_back(familyIndex);
            }
            
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
            for(const auto& familyVec : sortMap) {
                const auto& vec = familyVec.second;
                for(const auto& familyIndex : vec) {
                    const auto& ops = queueFamilies.at(familyIndex).opPriorities;
                    int prio = 0;
                    for(const auto& op : ops) {
                        if(!prioMap.contains(op)) prioMap.insert({op, {}});
                        if(!prioMap.at(op).contains(prio)) prioMap.at(op).insert({prio, {}});
                        prioMap.at(op).at(prio).push_back(familyIndex);
                        prio++;
                    }
                }
            }

            // map family index to logicalQueues index (logicalQueues is a simple array where the indexes don't necessarily correspond
            // to the family indexes)
            std::unordered_map<TQueueIndex, TQueueIndex> lqMap;
            for(TQueueSize s=0; s<logicalQueuesSize; ++s){
                const auto& q = *(logicalQueues[s].begin());
                lqMap.insert({q.familyIndex, s});
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
                        // we have to take one indirection here, using lqMap:
                        //  => logicalQueues only contains the queue families that can actually be used
                        //     For example: if only queue families 0 and 2 are used (in that order), then 
                        //     logicalQueues[queueFamilyIndex=0] == correct, but 
                        //     logicalQueues[queueFamilyIndex=2] == out of bounds == garbage memory
                        // Using lqMap, we get {{0,0},{2,1}}, so lqMap.at(queueFamilyIndex) is always the correct index into
                        // logicalQueues.
                        // This is a bit complicated, but since, outside of this, we only use queuesOpMap, which is safe and everything,
                        // it's fine.
                        vec.push_back(&logicalQueues[lqMap.at(queueFamilyIndex)]);
                    }
                }
            }

            return queuesOpMap;
        }
    };
}