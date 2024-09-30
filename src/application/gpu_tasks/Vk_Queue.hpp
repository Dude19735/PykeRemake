#pragma once

#include <thread>
#include <queue>
#include <set>
#include <condition_variable>
#include <unordered_map>
#include <vector>

#include "../../Defines.h"
#include "../../Vk_CI.hpp"
#include "../Vk_PhysicalDeviceQueueLib.hpp"
#include "Vk_GpuTask.hpp"

namespace VK5 {
    class Vk_Queue : public Vk_QueueBase {
    private:
        VkDevice _vkDevice;
        TQueueFamilyIndex _familyIndex;
        TQueueIndex _queueIndex;
        VkQueue _vkQueue;
        VkCommandPool _vkCommandPool;
        TGpuTargetOpFamilies _queueFamilyForTargetOp;

        bool _terminate;

        /**
         * NOTE: the sequence of these variables is important. They are initialized exactly
         * in the sequence they are written. If _allocFreeThread starts running before _allocFreeCondition
         * and/or _allocFreeMutex are fully initialized, the whole thing won't work!!!
         */
        std::queue<std::unique_ptr<Vk_GpuTask>> _allocTasks;
        std::queue<VkCommandBuffer> _usedCommandBuffers;
        std::mutex _allocFreeMutex;
        std::condition_variable _allocFreeCondition;
        std::thread _allocFreeThread;

        /**
         * NOTE: the sequence of these variables is important. They are initialized exactly
         * in the sequence they are written. First, write the queue, then the mutex, then the
         * condition_variable and at the very end, the thread. This will guarantee, that every
         * part has what they need up and running in time.
         */
        std::queue<std::unique_ptr<Vk_GpuTask>> _submitTasks;
        std::mutex _submitMutex;
        std::condition_variable _submitCondition;
        std::thread _submitThread;

    public:
        // regular constructor
        Vk_Queue(VkDevice vkDevice, uint32_t familyIndex, uint32_t queueIndex, std::unordered_map<Vk_GpuTargetOp, std::vector<TQueueFamilyIndex>> queueFamilyForTargetOp)
        : 
        _vkDevice(vkDevice), _familyIndex(familyIndex), _queueIndex(queueIndex),
        _vkQueue(_getDeviceQueue(_vkDevice, _familyIndex, _queueIndex)), _vkCommandPool(_createCommandPool(_vkDevice, _familyIndex)),
        _queueFamilyForTargetOp(queueFamilyForTargetOp), _terminate(false), 
        _allocFreeThread(std::thread(&Vk_Queue::_allocFreeLoop, this)),
        _submitThread(std::thread(&Vk_Queue::_submitLoop, this))
        {}

        Vk_Queue(const Vk_Queue& other) = delete;
        Vk_Queue(Vk_Queue&& other) = delete;
        Vk_Queue& operator=(const Vk_Queue& other) = delete;
        Vk_Queue& operator=(Vk_Queue&& other) = delete;

        ~Vk_Queue(){
            {
                std::scoped_lock lock {_allocFreeMutex, _submitMutex };
                _terminate = true;
            }
            /**
             * NOTE: make sure all the locks are released before notifying!
             */
            _allocFreeCondition.notify_one();
            if(_allocFreeThread.joinable()) _allocFreeThread.join();
            _submitCondition.notify_one();
            if(_submitThread.joinable()) _submitThread.join();

            if(_vkQueue != nullptr) vkQueueWaitIdle(_vkQueue);
            while(!_usedCommandBuffers.empty()){
                VkCommandBuffer b = _usedCommandBuffers.front();
                vkFreeCommandBuffers(_vkDevice, _vkCommandPool, 1, &b);
                _usedCommandBuffers.pop();
            }
            if(_vkCommandPool != nullptr) vkDestroyCommandPool(_vkDevice, _vkCommandPool, nullptr);
        }

        const TQueueFamilyIndex familyIndex() const { return _familyIndex; }
        const TQueueIndex queueIndex() const { return _queueIndex; }

        std::string toString() const {
            std::stringstream ss;
            ss << _familyIndex << "|" << _queueIndex;
            return ss.str();
        }

        TGpuTaskRunner enqueue(std::unique_ptr<Vk_GpuTask> task){
            TGpuTaskRunner res = nullptr;
            {
				std::unique_lock<std::mutex> lock(_allocFreeMutex);
                /**
                 * NOTE: This resetTask is important: it ensures that a task is always in Stage1 synchronously
                 * with the dispatching thread. This ensures that the expression
                 *    task = queue->enqueue(std::move(task))->waitResponsively();
                 * works properly. Without this, waitResponsively (and friends) will probably wait only the first time a task is
                 * enqueued. Afterwards, task->_stage == Stage5 before the next GpuTask::passAlloc runs and sets _stage=Stage2.
                 * NOTE: each GpuTask is >moved< into a Queue. From the outside, the GpuTask is replaced with a GpuTaskRunner ptr
                 * until it is returned by a waitResponsively() (and friends). GpuTaskRunner ptrs can't modify a task.
                 */
                task->resetTask();
                res = reinterpret_cast<TGpuTaskRunner>(task.get());
                _allocTasks.push(std::move(task));
			}
			_allocFreeCondition.notify_one();
            return res;
        }

    private:
        void _enqueueFree(VkCommandBuffer commandBuffer){
            {
				std::unique_lock<std::mutex> lock(_allocFreeMutex);
                _usedCommandBuffers.push(commandBuffer);
			}
        }

        void _allocFreeLoop() {
            std::unique_ptr<Vk_GpuTask> cTask = nullptr;
            VkCommandBuffer vkCommandBuffer;
            while(true){
                {
                    auto lock = std::unique_lock<std::mutex>(_allocFreeMutex);
                    _allocFreeCondition.wait(lock, [this](){
                        return !_allocTasks.empty() || !_usedCommandBuffers.empty() || _terminate;
                    });
                    
                    if(_terminate){
                        std::queue<std::unique_ptr<Vk_GpuTask>> empty;
                        std::swap(_allocTasks, empty);
                        return;
                    }

                    // first check, if we have some task that is finished and deposited it's command buffer
                    // back here.
                    if(_allocTasks.size() > 0){
                        if(!_usedCommandBuffers.empty()){
                            vkCommandBuffer = _usedCommandBuffers.front();
                            _usedCommandBuffers.pop();
                        }
                        else {
                            // if we don't have a spare command buffer, get one from the pool
                            // technically, the cost should ammortize, but maybe it doesn't, so reuse rather than get new ones
                            auto createInfo = Vk_CI::VkCommandBufferAllocateInfo_W(1, _vkCommandPool).data;
                            Vk_CheckVkResult(typeid(this), vkAllocateCommandBuffers(_vkDevice, &createInfo, &vkCommandBuffer), "Failed to allocate command buffers!");
                        }
                        cTask = std::move(_allocTasks.front());
                        _allocTasks.pop();
                    }
                }
                if(cTask) cTask->passAlloc(std::move(cTask), vkCommandBuffer, this, &_queueFamilyForTargetOp);
            }
        }

        void _enqueueSubmit(std::unique_ptr<Vk_GpuTask> task){
            {
				std::unique_lock<std::mutex> lock(_submitMutex);
                _submitTasks.push(std::move(task));
			}
			_submitCondition.notify_one();
        }

        void _submitLoop() {
            std::unique_ptr<Vk_GpuTask> cTask = nullptr;
            while(true){
                {
                    auto lock = std::unique_lock<std::mutex>(_submitMutex);
                    _submitCondition.wait(lock, [this](){
                        return !_submitTasks.empty() || _terminate;
                    });
                    
                    if(_terminate){
                        std::queue<std::unique_ptr<Vk_GpuTask>> empty;
                        std::swap(_submitTasks, empty);
                        return;
                    }
                    
                    cTask = std::move(_submitTasks.front());
                    _submitTasks.pop();
                }
                if(cTask) cTask->submit(std::move(cTask), _vkQueue);
            }
        }

        VkQueue _getDeviceQueue(VkDevice vkDevice, TQueueFamilyIndex familyIndex, TQueueIndex queueIndex) {
            VkQueue vkQueue;
            // get the queue from the device
            vkGetDeviceQueue(vkDevice, familyIndex, queueIndex, &vkQueue);
            return vkQueue;
        }

        VkCommandPool _createCommandPool(VkDevice vkDevice, TQueueFamilyIndex familyIndex) {
            VkCommandPool vkCommandPool;
            // create command pool for the device graphics queue
            auto cmCreateInfo = Vk_CI::VkCommandPoolCreateInfo_W(familyIndex).data;
            Vk_CheckVkResult(typeid(NoneObj), vkCreateCommandPool(_vkDevice, &cmCreateInfo, nullptr, &vkCommandPool), std::string("Unable to create command pool for queue ") + Vk_Queue::toString());
            return vkCommandPool;
        }
    };
}