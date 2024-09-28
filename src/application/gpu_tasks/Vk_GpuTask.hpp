#pragma once

#include <mutex>
#include <condition_variable>
#include <functional>
#include <array>

#include "../../Defines.h"

namespace VK5 {
    class Vk_GpuTask;
    class Vk_QueueBase {
    friend class Vk_GpuTask;
    public:
        virtual void _enqueueFree(VkCommandBuffer commandBuffer) = 0;
        virtual void _enqueueSubmit(std::unique_ptr<Vk_GpuTask> task) = 0;
    };

    struct Vk_GpuTaskParams {};
    typedef void(*TGpuTaskRecord)(VkCommandBuffer, const Vk_GpuTaskParams&);
    typedef void(*TGpuTaskSubmit)(VkCommandBuffer, VkQueue, const Vk_GpuTaskParams&);
    typedef void(Vk_QueueBase::*TEnqueueFree)(std::unique_ptr<Vk_GpuTask>);
    typedef void(Vk_QueueBase::*TEnqueueSubmit)(std::unique_ptr<Vk_GpuTask>);

    enum class Vk_GpuTaskStages {
        Stage1_Alloc = 0,
        Stage2_Record,
        Stage3_Submit,
        Stage4_Running,
        Stage5_Finished,
        Count
    };

    class Vk_Queue;
    class Vk_GpuTaskRunner {
        Vk_GpuOp _opType;
    public:
        Vk_GpuTaskRunner(Vk_GpuOp op) : _opType(op) {}
        const Vk_GpuOp opType() const { return _opType; }
        virtual std::unique_ptr<Vk_GpuTask> waitResponsivelyUS(std::chrono::microseconds us) = 0;
        virtual std::unique_ptr<Vk_GpuTask> waitResponsivelyMS(std::chrono::milliseconds ms) = 0;
        virtual std::unique_ptr<Vk_GpuTask> waitResponsively() = 0;
    };

    typedef Vk_GpuTaskRunner* TGpuTaskRunner;

    class Vk_GpuTask : public Vk_GpuTaskRunner {
    friend class Vk_Queue;
        // static constexpr int mCount = static_cast<int>(Vk_GpuTaskStages::Count);

        // Thank you:
        // https://stackoverflow.com/questions/73158786/locking-an-array-of-stdmutex-using-a-stdlock-guard-array
        // for fancy arrays:
        template<typename Mutexes>
        auto makeScopedLock(Mutexes& mutexes)
        { return std::apply([](auto&... mutexes) { return std::scoped_lock{mutexes...}; }, mutexes); }

        VkDevice _vkDevice;
        VkFence _vkFence;
        VkCommandBuffer _vkCommandBuffer;
        VkCommandPool _vkCommandPool;

        Vk_GpuTaskParams _params;
        TGpuTaskRecord _recordFunction;
        TGpuTaskSubmit _submitFunction;
        Vk_GpuTaskStages _stage;

        std::unique_ptr<Vk_GpuTask> _self;
        Vk_QueueBase* _parentQueue;

        // check for initialization with variadic templates
        // https://greitemann.dev/2018/09/15/variadic-expansion-in-aggregate-initialization/ 
        // std::array<std::mutex, mCount> _mutex;
        // std::array<std::condition_variable, mCount> _conditions;
        struct Stage1_Alloc{
            std::mutex mutex;
            std::condition_variable condition;
        } _stage1_alloc;
        struct Stage2_Record{
            std::mutex mutex;
            std::condition_variable condition;
        } _stage2_record;
        struct Stage3_Submit{
            std::mutex mutex;
            std::condition_variable condition;
        } _stage3_submit;
        struct Stage4_Running{
            std::mutex mutex;
            std::condition_variable condition;
        } _stage4_running;
        struct Stage5_Finished{
            std::mutex mutex;
            std::condition_variable condition;
        } _stage5_finished;

        bool _terminate;
        std::thread _recordThread;
        std::thread _runningWorker;
    public:
        Vk_GpuTask(VkDevice vkDevice, Vk_GpuOp opType) 
        : 
        Vk_GpuTaskRunner(opType),
        _vkDevice(vkDevice),
        _vkFence(_createFence(_vkDevice)), 
        _vkCommandBuffer(nullptr), 
        _vkCommandPool(nullptr), 
        _params({}),
        _recordFunction(nullptr),
        _submitFunction(nullptr),
        _stage(Vk_GpuTaskStages::Stage1_Alloc),
        _self(nullptr),
        _parentQueue(nullptr),
        _terminate(false), 
        _recordThread(std::thread(&Vk_GpuTask::_recordWorker, this)),
        _runningWorker(std::thread(&Vk_GpuTask::_running, this))
        {}
        Vk_GpuTask(const Vk_GpuTask& other) = delete;
        Vk_GpuTask(Vk_GpuTask&& other) = delete;
        Vk_GpuTask& operator=(const Vk_GpuTask& other) = delete;
        Vk_GpuTask& operator=(Vk_GpuTask&& other) = delete;

        virtual ~Vk_GpuTask(){
            // std::cout << ">>>>>>>>>>>>>>>>>>> destroy task" << std::endl;
            {
                auto scopedLock = std::scoped_lock {_stage1_alloc.mutex, _stage2_record.mutex, _stage3_submit.mutex, _stage4_running.mutex, _stage5_finished.mutex };
                _terminate = true;
            }
            _stage1_alloc.condition.notify_all();
            _stage2_record.condition.notify_all();
            _stage3_submit.condition.notify_all();
            _stage4_running.condition.notify_all();
            _stage5_finished.condition.notify_all();

            if(_recordThread.joinable()) _recordThread.join();
            if(_runningWorker.joinable()) _runningWorker.join();
            vkDestroyFence(_vkDevice, _vkFence, nullptr);
        }

        bool update(const Vk_GpuTaskParams& params, TGpuTaskRecord recordFunction, TGpuTaskSubmit submitFunction) { 
            _params = std::move(params); 
            if(recordFunction) _recordFunction = recordFunction;
            if(submitFunction) _submitFunction = submitFunction;
            return true;
        }

        std::unique_ptr<Vk_GpuTask> waitResponsivelyUS(std::chrono::microseconds us) {
            // wait until signaled
            // std::cout << "waiting for task to finish..." << std::endl;
            auto lock = std::unique_lock<std::mutex>(_stage5_finished.mutex);
            _stage5_finished.condition.wait_for(lock, us, [this](){ return _stage == Vk_GpuTaskStages::Stage5_Finished || _terminate; });

            // std::cout << "   ... task finished" << std::endl;
            return std::move(_self);
        }

        std::unique_ptr<Vk_GpuTask> waitResponsivelyMS(std::chrono::milliseconds ms) {
            // wait until signaled
            // std::cout << "waiting for task to finish..." << std::endl;
            auto lock = std::unique_lock<std::mutex>(_stage5_finished.mutex);
            _stage5_finished.condition.wait_for(lock, ms, [this](){  return _stage == Vk_GpuTaskStages::Stage5_Finished || _terminate; });

            // std::cout << "   ... task finished" << std::endl;
            return std::move(_self);
        }

        std::unique_ptr<Vk_GpuTask> waitResponsively() {
            // wait until signaled
            // std::cout << "waiting for task to finish..." << std::endl;
            auto lock = std::unique_lock<std::mutex>(_stage5_finished.mutex);
            _stage5_finished.condition.wait(lock, [this](){ return _stage == Vk_GpuTaskStages::Stage5_Finished || _terminate; });

            std::cout << "   ... task finished" << std::endl;
            return std::move(_self);
        }

    private:
        // stage 1 and 5 (serialized for VkCommandPool)
        void passAlloc(std::unique_ptr<Vk_GpuTask> self, VkCommandBuffer commandBuffer, Vk_QueueBase* pParentQueue){
            if(_vkCommandBuffer != nullptr) UT::Ut_Logger::RuntimeError(typeid(this), "CommandBuffer is not nullptr! Alloc before free => this is a bug!");
            _vkCommandBuffer = commandBuffer;
            _self = std::move(self);
            _parentQueue = pParentQueue;

            // goto next
            _stage = Vk_GpuTaskStages::Stage2_Record;
            _stage2_record.condition.notify_one();
            // std::cout << "T1." << std::endl;
        }

        void _recordWorker(){
            // std::cout << ">>>>>>>>>>>>>>>>>>> Start record worker" << std::endl;
            // run endlessly... we should techincally get one task at the time
            while(true){
                {
                    auto lock = std::unique_lock<std::mutex>(_stage2_record.mutex);
                    _stage2_record.condition.wait(lock, [this](){
                        return _stage == Vk_GpuTaskStages::Stage2_Record || _terminate;
                    });

                    if(_terminate){
                        // std::cout << ">>>>>>>>>>>>>>>>>>> Stop record worker" << std::endl;
                        return;
                    }

                    // std::cout << "T2." << std::endl;
                    if(_recordFunction) _recordFunction(_vkCommandBuffer, _params);

                    // goto next: back to Vk_Queue because this one has to be in sync
                    _stage = Vk_GpuTaskStages::Stage3_Submit;
                }
                /**
                 * NOTE: make sure all locks are released before calling this one
                 * _enqueueSubmit is thread safe
                 */
                _parentQueue->_enqueueSubmit(std::move(_self));
            }
        }

        void submit(std::unique_ptr<Vk_GpuTask> self, VkQueue vkQueue) {
            // submit task, run from Vk_Queue
            // std::cout << "T3." << std::endl;
            if(_submitFunction) _submitFunction(_vkCommandBuffer, vkQueue, _params);
            _self = std::move(self);

            // goto next
            _stage = Vk_GpuTaskStages::Stage4_Running;
            _stage4_running.condition.notify_one();
        }

        void _running() {
            // std::cout << ">>>>>>>>>>>>>>>>>>> Start task runner" << std::endl;
            while(true) {
                {
                    auto lock = std::unique_lock<std::mutex>(_stage4_running.mutex);
                    _stage4_running.condition.wait(lock, [this](){
                        return _stage == Vk_GpuTaskStages::Stage4_Running || _terminate;
                    });

                    if(_terminate){
                        // std::cout << ">>>>>>>>>>>>>>>>>>> Stop task runner" << std::endl;
                        return;
                    }
                    
                    // std::cout << "T4." << std::endl;
                    VkResult res = vkWaitForFences(_vkDevice, 1, &_vkFence, VK_TRUE, GLOBAL_FENCE_TIMEOUT);
                    if(res == VK_TIMEOUT) UT::Ut_Logger::RuntimeError(typeid(this), "Signal timeout");
                    else if (res != VK_SUCCESS) UT::Ut_Logger::RuntimeError(typeid(this), "Signal catastrophic result!");

                    // std::cout << "T4.." << std::endl;
                    _parentQueue->_enqueueFree(_vkCommandBuffer);
                    _vkCommandBuffer = nullptr;
                    _parentQueue = nullptr;

                    _stage = Vk_GpuTaskStages::Stage5_Finished;
                }
                /**
                 * NOTE: make sure all locks are released before calling notify
                 */
                _stage5_finished.condition.notify_all(); // everyone who is waiting
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