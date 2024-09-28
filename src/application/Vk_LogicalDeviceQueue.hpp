#pragma once

#include "../Defines.h"
#include "../Vk_CI.hpp"
#include "Vk_LogicalDeviceQueueLib.hpp"
#include "Vk_PhysicalDeviceQueue.hpp"
#include "./gpu_tasks/Vk_Queue.hpp"

namespace VK5 {
    class Vk_LogicalDeviceQueue {
    private:
        VkDevice _vkDevice;
        // this one is an array becuase it has to be allocated at runtime and really has 
        // to stay where it is afterwards (std::vector doesn't necessarily do that)
        // _logicalQueueFamilies is NOT indexed using the queue family index. It only contains the queue families
        // that can actually be used, given the Vk_GpuOp priorities. For example, if queue families 0 and 2 are used
        // then family 0 is at index 0 but family 2 is at index 1. If _logicalQueueFamilies is then indexed using the family indices
        //  => garbage-memory-out-of-bounds!
        std::unique_ptr<TLogicalQueueFamilies> _logicalQueueFamilies;

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
        TLogicalQueuesOpFamilyMap _queuesOpMap;

        std::mutex _mutex;
    public:
        Vk_LogicalDeviceQueue(VkDevice device, const Vk_PhysicalDeviceQueue& physicalDeviceQueue)
        :
        _vkDevice(device),
        _logicalQueueFamilies(std::move(_createLogicalQueues(_vkDevice, physicalDeviceQueue.queueFamilyMap()))),
        _queuesOpMap(Vk_LogicalDeviceQueueLib::createLogicalQueuesOpMap(physicalDeviceQueue.queueFamilyMap(), physicalDeviceQueue.queueFamilies()))
        {}

        Vk_LogicalDeviceQueue(Vk_LogicalDeviceQueue& other) = delete;
        Vk_LogicalDeviceQueue(Vk_LogicalDeviceQueue&& other) noexcept
        :
        _vkDevice(other._vkDevice),
        _logicalQueueFamilies(std::move(other._logicalQueueFamilies)),
        _queuesOpMap(std::move(other._queuesOpMap))
        {
            other._vkDevice = nullptr;
        }

        Vk_LogicalDeviceQueue& operator=(const Vk_LogicalDeviceQueue& other) = delete;
        Vk_LogicalDeviceQueue& operator=(Vk_LogicalDeviceQueue&& other) noexcept {
            _vkDevice = other._vkDevice;
            _logicalQueueFamilies = std::move(other._logicalQueueFamilies);

            other._vkDevice = nullptr;

            return *this;
        }

        ~Vk_LogicalDeviceQueue(){}

        const TLogicalQueuesOpFamilyMap& queuesOpMap() const { return _queuesOpMap; }
        const TLogicalQueueFamilies& queueFamilies() const { return *_logicalQueueFamilies.get(); }

        std::unique_ptr<Vk_Queue> getQueue(Vk_GpuOp opType) {
            if(!_queuesOpMap.contains(opType)) return nullptr;

            const auto& opFamilyIndices = _queuesOpMap.at(opType);
            // no queues for Gpu_Op available
            if(opFamilyIndices.size() == 0) return nullptr;

            std::unique_ptr<Vk_Queue> queue = nullptr;
            {
                auto lock = std::lock_guard<std::mutex>(_mutex);
                for(auto familyIndex : opFamilyIndices){
                    auto& queueList = _logicalQueueFamilies->at(familyIndex);
                    if(queueList.size() > 0){
                        queue = std::move(queueList.front());
                        queueList.pop_front();
                        break;
                    }
                }
            }
            return std::move(queue);
        }

        void addQueue(Vk_GpuOp optype, std::unique_ptr<Vk_Queue> queue){
            auto lock = std::lock_guard<std::mutex>(_mutex);
            _logicalQueueFamilies->at(queue->familyIndex()).emplace_back(std::move(queue));
        }

    private:
        std::unique_ptr<TLogicalQueueFamilies> _createLogicalQueues(VkDevice device, const TDeviceQueueFamilyMap& queueFamilyMap){
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
            std::unique_ptr<TLogicalQueueFamilies> logicalQueues = std::make_unique<TLogicalQueueFamilies>();
            for(const auto& family : map){
                TQueueFamilyIndex familyIndex = family.first;
                if(!logicalQueues->contains(familyIndex)) logicalQueues->insert({familyIndex, {}});
                auto& ll = logicalQueues->at(familyIndex);
                for(const TQueueIndex& queueIndex : family.second)
                    ll.push_back(std::make_unique<Vk_Queue>(device, family.first, queueIndex));
            }

            return logicalQueues;
        }
    };
}