#pragma once

#include <thread>
#include <queue>
#include <condition_variable>

#include "../Defines.h"
#include "../Vk_CI.hpp"
#include "../tasks/Vk_GpuTask.hpp"

namespace VK5 {
    class Vk_Queue {
    private:
        VkDevice _vkDevice;
        VkQueue _vkQueue;
        VkCommandPool _vkCommandPool;
        TQueueFamilyIndex _familyIndex;
        TQueueIndex _queueIndex;

        bool _terminate;
        std::thread _submitThread;
        std::mutex _mainMutex;
        std::condition_variable _submitCondition;
        std::queue<Vk_GpuTask*> _submitTasks;

    public:
        // regular constructor
        Vk_Queue(VkDevice vkDevice, uint32_t familyIndex, uint32_t queueIndex) 
        : 
        _vkDevice(vkDevice), _vkQueue(nullptr), _vkCommandPool(nullptr), _familyIndex(familyIndex), _queueIndex(queueIndex),
        _terminate(false), _submitThread(std::thread(&Vk_Queue::submitLoop, this))
        {
            // get the queue from the device
            vkGetDeviceQueue(_vkDevice, _familyIndex, _queueIndex, &_vkQueue);
            // create command pool for the device graphics queue
            auto cmCreateInfo = Vk_CI::VkCommandPoolCreateInfo_W(familyIndex).data;
            Vk_CheckVkResult(typeid(NoneObj), vkCreateCommandPool(_vkDevice, &cmCreateInfo, nullptr, &_vkCommandPool), std::string("Unable to create command pool for queue ") + Vk_Queue::toString());
        }

        Vk_Queue(const Vk_Queue& other) = delete;
        Vk_Queue(Vk_Queue&& other) = delete;
        Vk_Queue& operator=(const Vk_Queue& other) = delete;
        Vk_Queue& operator=(Vk_Queue&& other) = delete;

        ~Vk_Queue(){
            auto lock = std::unique_lock<std::mutex>(_mainMutex);
            _terminate = true;
            _submitCondition.notify_all();
            _submitThread.join();

            if(_vkQueue != nullptr) vkQueueWaitIdle(_vkQueue);
            if(_vkCommandPool != nullptr) vkDestroyCommandPool(_vkDevice, _vkCommandPool, nullptr);
        }

        const TQueueFamilyIndex familyIndex() const { return _familyIndex; }
        const TQueueIndex queueIndex() const { return _queueIndex; }

        std::string toString() const {
            std::stringstream ss;
            ss << _familyIndex << "|" << _queueIndex;
            return ss.str();
        }

        void enqueue(Vk_GpuTask* task){
            Vk_GpuTask* ret = nullptr;
            {
				std::unique_lock<std::mutex> lock(_mainMutex);
                _submitTasks.push(task);
			}
			_submitCondition.notify_one();
        }

    private:
        void submitLoop() {
            while(true){
                auto lock = std::unique_lock<std::mutex>(_mainMutex);
                _submitCondition.wait(lock, [this](){
                    return !_submitTasks.empty() || _terminate;
                });
                
                if(_terminate){
                    std::queue<Vk_GpuTask*> empty;
                    std::swap(_submitTasks, empty);
                    return;
                }
                
                auto task = std::move(_submitTasks.front());
                _submitTasks.pop();

                task->allocAndRecord(_vkCommandPool);
                task->submit(_vkQueue);
                task->after(std::bind([&](){followup(task);}));
            }
        }

        void followup(Vk_GpuTask* task){
            auto lock = std::unique_lock<std::mutex>(_mainMutex);
            task->free(_vkCommandPool);
        }

        VkCommandBuffer getCommandBuffer(){
            return *getCommandBuffers(1).begin();
        }

        std::vector<VkCommandBuffer> getCommandBuffers(size_t n){
            std::vector<VkCommandBuffer> commandBuffer(n);
			VkCommandBufferAllocateInfo allocInfo = Vk_CI::VkCommandBufferAllocateInfo_W(static_cast<uint32_t>(n), _vkCommandPool).data;
			Vk_CheckVkResult(typeid(this), vkAllocateCommandBuffers(_vkDevice, &allocInfo, commandBuffer.data()), "Failed to allocate command buffers for queue [FI:{0} | QI:{1}]", _familyIndex, _queueIndex);
            return commandBuffer;
        }
    };
}