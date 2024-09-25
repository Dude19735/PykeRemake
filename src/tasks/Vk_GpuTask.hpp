#pragma once

#include <mutex>
#include <condition_variable>
#include <functional>

#include "../Defines.h"

namespace VK5 {
    class Vk_Queue;

    struct Vk_GpuTaskParams {};
    typedef std::function<void(VkCommandBuffer, const Vk_GpuTaskParams&)> TGpuTaskRecord;
    typedef std::function<void()> TGpuTaskFollowup;

    class Vk_GpuTask {
    friend class Vk_Queue;
        VkDevice _vkDevice;
        Vk_GpuOp _opType;
        VkFence _vkFence;
        VkCommandBuffer _vkCommandBuffer;
        VkCommandPool _vkCommandPool;

        std::mutex _taskMutex;
        std::condition_variable _taskCondition;
        TGpuTaskRecord _recordFunction;
        Vk_GpuTaskParams _params;
        TGpuTaskFollowup _followup;

        bool _terminate;
        std::mutex _signalMutex;
        std::condition_variable _signalCondition;
        std::thread _signalThread;
    public:
        Vk_GpuTask(VkDevice vkDevice, Vk_GpuOp opType, const TGpuTaskRecord& recordFunction, const Vk_GpuTaskParams& params) 
        : 
        _vkDevice(vkDevice), _opType(opType), _vkFence(_createFence(_vkDevice)), 
        _vkCommandBuffer(nullptr), _vkCommandPool(nullptr), _recordFunction(std::move(recordFunction)), _params(std::move(params)),
        _terminate(false), _signalThread(std::thread(&Vk_GpuTask::_signalWorker, this))
        {}
        Vk_GpuTask(const Vk_GpuTask& other) = delete;
        Vk_GpuTask(Vk_GpuTask&& other) = delete;
        Vk_GpuTask& operator=(const Vk_GpuTask& other) = delete;
        Vk_GpuTask& operator=(Vk_GpuTask&& other) = delete;

        virtual ~Vk_GpuTask(){
            {
                auto lock = std::unique_lock<std::mutex>(_signalMutex);
                _terminate = true;
            }
            _signalCondition.notify_all();
            _signalThread.join();
            _taskCondition.notify_all();
            vkDestroyFence(_vkDevice, _vkFence, nullptr);
        }
        const Vk_GpuOp opType() const { return _opType; }

        void update(const TGpuTaskRecord& recordFunction, const Vk_GpuTaskParams& params){
            _params = std::move(params);
            _recordFunction = std::move(recordFunction);
        }

        void wait() {
            auto lock = std::unique_lock<std::mutex>(_taskMutex);
            _taskCondition.wait(lock);
        }

    private:
        VkFence fence() { return _vkFence; }
        void allocAndRecord(VkCommandPool commandPool){
            if(_vkCommandBuffer != nullptr) UT::Ut_Logger::RuntimeError(typeid(this), "CommandBuffer is not nullptr! Alloc before free => this is a bug!");
            auto createInfo = Vk_CI::VkCommandBufferAllocateInfo_W(1, commandPool).data;
            Vk_CheckVkResult(typeid(this), vkAllocateCommandBuffers(_vkDevice, &createInfo, &_vkCommandBuffer), "Failed to allocate command buffers!");
            _recordFunction(_vkCommandBuffer, _params);
        }

        virtual void submit(VkQueue vkQueue) = 0;

        void after(const TGpuTaskFollowup& followup) {
            auto lock = std::unique_lock<std::mutex>(_signalMutex);
            _followup = std::move(followup);
            _signalCondition.notify_all();
        }

        void free(VkCommandPool commandPool){
            if(_vkCommandBuffer == nullptr) UT::Ut_Logger::RuntimeError(typeid(this), "CommandBuffer is nullptr! Free before alloc => this is a bug!");            
            vkFreeCommandBuffers(_vkDevice, commandPool, 1, &_vkCommandBuffer);
        }

        void _signalWorker(){
            while(true){
                auto lock = std::unique_lock<std::mutex>(_signalMutex);
                _signalCondition.wait(lock, [this](){
                    return _terminate;
                });

                VkResult res = vkWaitForFences(_vkDevice, 1, &_vkFence, VK_TRUE, GLOBAL_FENCE_TIMEOUT);
                if(res == VK_TIMEOUT) UT::Ut_Logger::RuntimeError(typeid(this), "Signal timeout");
                else if (res != VK_SUCCESS) UT::Ut_Logger::RuntimeError(typeid(this), "Signal catastrophic result!");
                _taskCondition.notify_all();
                _followup();
            }
        }

        VkFence _createFence(VkDevice vkDevice){
            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            // we must create the fences in the signaled state to ensure that on the first call to drawFrame
            // vkWaitForFences won't wait indefinitely
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            VkFence fence;
            Vk_CheckVkResult(typeid(this), vkCreateFence(vkDevice, &fenceInfo, nullptr, &fence), "Failed to create in flight fences");
            return fence;
        }
    };
}