#pragma once

#include <unordered_map>
#include <forward_list>
#include <mutex>

#include "../../Defines.h"
#include "Vk_GpuTask.hpp"

namespace VK5 {
    class Vk_GpuTaskPool {
    private:
        VkDevice _vkDevice;
        std::unordered_map<Vk_GpuOp, std::forward_list<std::unique_ptr<Vk_GpuTask>>> _tasks;
        std::mutex _mutex;
    public:
        Vk_GpuTaskPool(VkDevice vkDevice) : _vkDevice(vkDevice) {}
        Vk_GpuTaskPool(const Vk_GpuTaskPool& other) = delete;
        Vk_GpuTaskPool(Vk_GpuTaskPool&& other)
        :
        _vkDevice(other._vkDevice),
        _tasks(std::move(other._tasks))
        {
            other._vkDevice = nullptr;
        }

        Vk_GpuTaskPool& operator=(const Vk_GpuTaskPool& other) = delete;
        Vk_GpuTaskPool& operator=(Vk_GpuTaskPool&& other) {
            if(this == &other) return *this;
            _vkDevice = other._vkDevice;
            _tasks = std::move(other._tasks);

            other._vkDevice = nullptr;

            return *this;
        }

        ~Vk_GpuTaskPool(){}

        std::unique_ptr<Vk_GpuTask> getOrCreateTask(Vk_GpuOp op){
            auto lock = std::lock_guard<std::mutex>(_mutex);
            if(!_tasks.contains(op)){
                // create new task for op and return a unique ptr for it
                return std::move(std::make_unique<Vk_GpuTask>(_vkDevice, op));
            }
            else{
                // return one of the already present tasks
                auto& tt = _tasks.at(op);
                auto uPtr = std::move(tt.front());
                tt.pop_front();
                return std::move(uPtr);
            }
        }

        void returnTask(std::unique_ptr<Vk_GpuTask> task){
            auto lock = std::lock_guard<std::mutex>(_mutex);
            if(!_tasks.contains(task->opType())){
                _tasks.insert({task->opType(), {}});
                _tasks.at(task->opType()).emplace_front(std::move(task));
            }
            else{
                _tasks.at(task->opType()).emplace_front(std::move(task));
            }
        }
    };
}