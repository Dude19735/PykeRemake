#pragma once

#include "../Defines.h"
#include "../Vk_CI.hpp"
#include "Vk_LogicalDeviceQueueLib.hpp"
#include "Vk_PhysicalDeviceQueue.hpp"
#include "Vk_Queue.hpp"

namespace VK5 {
    class Vk_LogicalDeviceQueue {
    private:
        VkDevice _vkDevice;
        // this one is an array becuase it has to be allocated at runtime and really has 
        // to stay where it is afterwards (std::vector doesn't necessarily do that)
        // _logicalQueues is NOT indexed using the queue family index. It only contains the queue families
        // that can actually be used, given the Vk_GpuOp priorities. For example, if queue families 0 and 2 are used
        // then family 0 is at index 0 but family 2 is at index 1. If _logicalQueues is then indexed using the family indices
        //  => garbage-memory-out-of-bounds!
        TLogicalQueuesSize _logicalQueuesSize;
        std::unordered_map<TQueueFamilyIndex, TLogicalQueueIndex> _lqMap;
        std::unique_ptr<TLogicalQueues[]> _logicalQueues;

        // Map that maps all DeviceQueues queues capable of performing Vk_OpX to that operation
        // An access consists of 
        //  1. taking map entry for Vk_OpX
        //  2. search for the first non-empty stack
        //  3. pop a queue from that stack
        //  4. submit the task to that queue
        //  5. insert the queue at the end of the stack
        // {
        //   Vk_Op1: vec{[&stack of queue family with Vk_Op1 as priority 1], ..., [&stack of queue family with Vk_Op1 as priority M]}
        //    ...
        //   Vk_OpN: vec{[&stack of queue family with Vk_OpN as priority 1], ..., [&stack of queue family with Vk_OpN as priority M]}
        // }
        TLogicalQueuesOpMap _queuesOpMap;

        std::shared_mutex _mutex;
    public:
        Vk_LogicalDeviceQueue(VkDevice device, const Vk_PhysicalDeviceQueue& physicalDeviceQueue)
        :
        _vkDevice(device),
        _logicalQueues(_createLogicalQueues(_vkDevice, physicalDeviceQueue.queueFamilyMap(), _logicalQueuesSize, _lqMap)),
        _queuesOpMap(Vk_LogicalDeviceQueueLib::createLogicalQueuesOpMap(physicalDeviceQueue.queueFamilyMap(), physicalDeviceQueue.queueFamilies(), _logicalQueues.get(), _lqMap, _logicalQueuesSize))
        {}

        Vk_LogicalDeviceQueue(Vk_LogicalDeviceQueue& other) = delete;
        Vk_LogicalDeviceQueue(Vk_LogicalDeviceQueue&& other) noexcept
        :
        _vkDevice(other._vkDevice),
        _logicalQueues(std::move(other._logicalQueues)),
        _logicalQueuesSize(other._logicalQueuesSize),
        _queuesOpMap(std::move(other._queuesOpMap))
        {
            other._vkDevice = nullptr;
            other._logicalQueuesSize = 0;
        }

        Vk_LogicalDeviceQueue& operator=(const Vk_LogicalDeviceQueue& other) = delete;
        Vk_LogicalDeviceQueue& operator=(Vk_LogicalDeviceQueue&& other) noexcept {
            _vkDevice = other._vkDevice;
            _logicalQueuesSize = other._logicalQueuesSize;
            _logicalQueues = std::move(other._logicalQueues);

            other._vkDevice = nullptr;
            other._logicalQueuesSize = 0;

            return *this;
        }

        ~Vk_LogicalDeviceQueue(){
            if(_logicalQueues != nullptr){
                // remove all the queues as soon as they are idle
                for(TQueueIndex i=0; i<_logicalQueuesSize; ++i) _logicalQueues[i].clear();
            }
        }

        const TLogicalQueuesOpMap& queuesOpMap() const { return _queuesOpMap; }

    private:
        static std::unique_ptr<TLogicalQueues[]> _createLogicalQueues(VkDevice device, const TDeviceQueueFamilyMap& queueFamilyMap, /*out*/TLogicalQueuesSize& logicalQueuesSize, /*out*/std::unordered_map<TQueueFamilyIndex, TLogicalQueueIndex>& lqMap){
            // get the queues for all queue families and organize them in a stack
            std::map<TQueueFamilyIndex, std::vector<TQueueIndex>> map;
            for(const auto& family : queueFamilyMap) {
                for(const auto& i : family.second) {
                    auto familyIndex = family.first;
                    if(!map.contains(familyIndex)) map.insert({familyIndex, {}});
                    map.at(familyIndex).push_back(i);
                }
            };

            // move all stacks into an array, ordered by the queue family index
            logicalQueuesSize = map.size();
            std::unique_ptr<TLogicalQueues[]> logicalQueues = std::make_unique<TLogicalQueues[]>(logicalQueuesSize);
            size_t s=0;
            for(const auto& family : map){
                logicalQueues[s].reserve(family.second.size());
                for(const auto& queueIndex : family.second)
                    logicalQueues[s].emplace_back(std::make_unique<Vk_Queue>(device, family.first, queueIndex));
                s++;
            }

            // map family index to logicalQueues index (logicalQueues is a simple array where the indexes don't necessarily correspond
            // to the family indexes)
            for(TLogicalQueueIndex s=0; s<logicalQueuesSize; ++s){
                const auto& q = *(logicalQueues[s].begin());
                lqMap.insert({q->familyIndex(), s});
            }

            return logicalQueues;
        }
    };
}