#pragma once

#include <mutex>
#include <typeinfo>
#include <atomic>

#include "../Defines.h"
#include "Vk_DataBufferLib.hpp"

namespace VK5 {

	template<typename TStructureType>
	class Vk_DataBuffer {
		Vk_GpuTargetOp _gpuTargetOp;
		Vk_PhysicalDevice* _physicalDevice;
		std::vector<TStructureType> _cpuDataBuffer;
		size_t _count;
		size_t _maxCount;
		std::string _objName;
		std::shared_mutex _localMutex;
		Vk_BufferSizeBehaviour _sizeBehaviour;
		Vk_BufferUpdateBehaviour _updateBehaviour;
		Vk_DataBufferLib::BufferType _type;
		std::string _associatedObject;

		std::atomic_int32_t _bufferIndex;
		std::vector<VkBuffer> _buffer;
		std::vector<VkDeviceMemory> _bufferMemory;
	public:
		Vk_DataBuffer(
			/**
			 * TODO:Vk_GpuTargetOp - add this param to Vk_DataBuffer and adjust all instantiations (Graphics or Compute)
			 * Vk_GpuTargetOp bufferTargetOp,
			 */
			Vk_PhysicalDevice* physicalDevice,
			const std::string& associatedObject,
			const TStructureType* pStructuredData,
			size_t count,
			Vk_BufferUpdateBehaviour updateBehaviour,
			Vk_BufferSizeBehaviour sizeBehaviour,
			std::string objName = ""
		)
			:
			/**
			 * TODO:Vk_GpuTargetOp - remove default value here
			 */
			_gpuTargetOp(Vk_GpuTargetOp::Auto),
			_physicalDevice(physicalDevice),
			_cpuDataBuffer({}),
			_count(count),
			_maxCount(Vk_DataBufferLib::getInitMaxCount(count, sizeBehaviour)),
			_objName(objName + "[" + std::string(typeid(TStructureType).name()) + "]"),
			_sizeBehaviour(sizeBehaviour),
			_updateBehaviour(updateBehaviour),
			_type(Vk_DataBufferLib::getInitBufferType<TStructureType>()),
			_associatedObject("(=" + associatedObject + "=)"),
			_bufferIndex(0),
			_buffer({}),
			_bufferMemory({})
		{
			UT::Ut_Logger::Log(typeid(this), UT::GlobalCasters::castConstructorTitle(
				Vk_Lib::formatWithObjName(_objName, (std::string("Create Vertex Buffer ") + std::string(typeid(TStructureType).name()) + _associatedObject))));

			Vk_DataBufferLib::checkAsserts(_objName, bufferCount());
			_createDataBufferForUpdateStrategy(Vk_DataBufferLib::StructuredData<TStructureType>{.count=count, .data=pStructuredData});
		}

		~Vk_DataBuffer() {
			UT::Ut_Logger::Log(typeid(this), UT::GlobalCasters::castDestructorTitle(Vk_Lib::formatWithObjName(_objName, (std::string("Destroy Vertex Buffer ") + std::string(typeid(TStructureType).name()) + _associatedObject))));
			_physicalDevice->logicalDevice().destroyBuffers(std::move(_buffer), std::move(_bufferMemory));
		}

		/*
		* Get the Vulkan buffer descriptor.
		* This will return a pointer to the current buffer
		*/
		VkBuffer vk_buffer() {
			// _bufferIndex is an atomic. It will always point to an existing entry
			// of _buffer
			return _buffer.at(_bufferIndex);
		}

		/*
		* Get the currently occupied size of the buffer in bytes
		*/
		size_t bufferByteSize() {
			auto lock = std::shared_lock<std::shared_mutex>(_localMutex);
			return _bufferByteSize();
		}

		/*
		* Get the current max buffer size in bytes
		*/
		size_t maxBufferByteSize() {
			auto lock = std::shared_lock<std::shared_mutex>(_localMutex);
			return _maxBufferByteSize();
		}

		/*
		* Get the current count of elements currently in the buffer
		*/
		size_t bufferCount() {
			auto lock = std::shared_lock<std::shared_mutex>(_localMutex);
			return _bufferCount();
		}

		/*
		* Get the current max amount of elements the buffer can hold before resizing
		*/
		size_t maxBufferCount() {
			auto lock = std::shared_lock<std::shared_mutex>(_localMutex);
			return _maxCount;
		}

		/*
		* Resize the buffer to hold a max of newCount. This does not resize according to the resize strategy.
		* The resize strategy is only used if the buffer has to be enlarged because some new data does not fit.
		*/
		void resize(size_t newCount) {
			auto lock = std::lock_guard<std::shared_mutex>(_localMutex);
			_resizeDataBufferForUpdateStrategy(newCount, _count);
		}

		/*
		* Get the max amount of elements the buffer will be able to hold after the
		* next resize according to the resize strategy of the buffer. The resize
		* strategy is only used if the buffer has to be enlarged because some new data
		* does not fit.
		*/
		size_t nextBufferMaxCount(size_t count) {
			return _getNextMaxCount(count);
		}

		/*
		* Return a string holding the object name the buffer is holding, the type, update and resize
		* strategy
		*/
		std::string toString() const {
			return
				std::string("Data buffer ")
				+ _objName
				+ std::string(", ")
				+ Vk_DataBufferLib::BufferTypeToString(_type)
				+ std::string(", ")
				+ Vk_BufferUpdateBehaviourToString(_updateBehaviour)
				+ std::string(", ")
				+ Vk_BufferSizeBehaviourToString(_sizeBehaviour);
		}

		/*
		* Write the data of the GPU buffer into a CPU size vector and return a pointer
		* to the CPU sized buffer. Call vk_clearCpuBuffer() to clear the CPU sized data vector.
		*/
		const std::vector<TStructureType>& getData() {
			auto lock = std::lock_guard<std::shared_mutex>(_localMutex);
			_getDataToCpu();
			return _cpuDataBuffer;
		}
		
		/*
		* Clear the CPU sided vector. Note: this will not clear the GPU sided memory.
		*/
		void vk_clearCpuBuffer() {
			auto lock = std::lock_guard<std::shared_mutex>(_localMutex);
			_cpuDataBuffer.clear();
		}

		/*
		* Return the size of the CPU sided data vector. Note: this does not necessairily reflect
		* the size of the GPU sided data.
		*/
		size_t cpuDataBufferCount() {
			auto lock = std::shared_lock<std::shared_mutex>(_localMutex);
			return _cpuDataBuffer.size();
		}

		/*
		* Return the total available GPU local memory. This accesses the device and returns
		* the value the device returns.
		*/
		std::uint64_t localBufferMemoryBudget() const {
			return Vk_DataBufferLib::getDeviceLocalBufferMaxMemory(_physicalDevice);
		}

		/*
		* Return the total available GPU/CPU shared memory. In general this is GPU main memory that
		* can be used by the GPU and usually corresponds to half of the computer main memory. This accesses
		* the device and returns the value the device returns.
		*/
		std::uint64_t stagingBufferMemoryBudget() const {
			return Vk_DataBufferLib::getStagingBufferMaxMemory(_physicalDevice);
		}

		/*
		* Return the element count the buffer will be initialized with given the parameter count
		* according to the buffer's initialization strategy.
		*/
		size_t getRequiredInitCount(size_t count) const {
			return Vk_DataBufferLib::getInitMaxCount(count, _sizeBehaviour);
		}


		/*
		* Get the new max buffer count given a hypothetical newCount based on the 
		* buffer's resize strategy.
		*/
		size_t getNewRequiredMaxCount(size_t newCount) {
			size_t newMaxCount;
			auto lock = std::lock_guard<std::shared_mutex>(_localMutex);
			Vk_DataBufferLib::getNewBufferCount(_sizeBehaviour, _maxCount, newCount, newMaxCount);
			return newMaxCount;
		}

		/*
		* Update the buffer with the new structuredData of size newCount from the offset
		* newFrom on. This will update the data in the buffer starting from the newFrom offset up
		* to newTo offset.
		*
		* Both newFrom and newTo must be smaller than newCount. newCount is the **total** count of the
		* new buffer data. **count** is the number of elements, not the byte size.
		* The minimal value for newTo is 1 (newFrom + 1) to update at least one entry.
		* If newTo=0 it will be set to newCount and everything starting from newFrom will be updated.
		* Semantic behaviour:
		* ===================
		*  * newCount > current count: 
		*     - buffer will be resized and the interval [newFrom, newTo) will be copied into it
		*     - the programmer is responsible for making sure that the transfer makes sense.
		*     - examples:
		*        # newCount = oldCount + X, newFrom = oldCount, newTo = oldCount + X
		*        # newCount = oldCount, newFrom = X1 > 0, newTo = X2 < newCount
		*        # newCount = oldCount - X, newFrom = 0, newTo = newCount
		*
		* The total update duration [us] is returned.
		* For the two 'GlobalLock' update strategies, the update is completely synchronized.
		* For all other update strategies, the update goes into the currently not used buffer or a newly
		* allocated one that is later switched (lazy). The switch is performed inside the draw method
		* after a vk_rebuildAndRedraw() call on the viewer.
		*/
		int64_t update(
			const TStructureType* structuredData,
			size_t newCount,
			size_t newFrom,
			size_t newTo=0
		) {
			auto lock = std::lock_guard<std::shared_mutex>(_localMutex);
			auto t1 = std::chrono::high_resolution_clock::now();
			_updateDataBufferForUpdateStrategy({.count=newCount, .data=structuredData}, newFrom, newTo);
			auto t2 = std::chrono::high_resolution_clock::now();
			auto diff = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
			return diff;
		}

	private:
		size_t _getNextMaxCount(size_t oldMaxCount) {
			return Vk_DataBufferLib::getNextMaxCount(_sizeBehaviour, oldMaxCount);
		}

		size_t _maxBufferByteSize(){
			size_t s = sizeof(TStructureType);
			return static_cast<size_t>(_maxCount * s);
		}

		size_t _bufferByteSize() {
			return static_cast<size_t>(_count * sizeof(TStructureType));
		}

		size_t _bufferCount() {
			return _count;
		}

		/**
		* Resize a buffer to newMaxCount (amount of elements in the buffer). If
		* resize is a part of an update, use newFromOffset (in elements) to only
		* transfer the part of the data that we want to keep.
		*/
		void _resizeDataBufferForUpdateStrategy(size_t newMaxCount, size_t useCount) {
			Vk_DataBufferLib::checkResizeAsserts(_objName, _count, newMaxCount, useCount);
			Vk_DataBufferLib::resizeMessage(_objName, _type, _sizeBehaviour, _maxCount, newMaxCount, _count);
			UT::Ut_Logger::Warn(typeid(this), "Resizing buffer {0} from {1} to {2}", ("#Resize#" + _objName + _associatedObject), _maxCount, newMaxCount);

			std::uint64_t oldMaxSize = static_cast<std::uint64_t>(_maxBufferByteSize());

			_maxCount = newMaxCount;
			_count = useCount;

			std::uint64_t dataSize = static_cast<std::uint64_t>(_bufferByteSize());
			std::uint64_t maxSize = static_cast<std::uint64_t>(_maxBufferByteSize());

			for(int i=0; i<_buffer.size(); ++i){
				VkBuffer newBuffer = nullptr;
				VkDeviceMemory newBufferMemory = nullptr;
				try {
					VkBuffer buf = _buffer.at(i);
					VkDeviceMemory mem = _bufferMemory.at(i);
					Vk_DataBufferLib::createDeviceLocalBuffer(_physicalDevice, _type, newBuffer, newBufferMemory, maxSize, Vk_DataBufferLib::Usage::Both, _gpuTargetOp);
					std::string nn = "#Resize#" + _objName + _associatedObject;
					Vk_DataBufferLib::copyGpuToGpu(_physicalDevice, nn, buf, oldMaxSize, newBuffer, maxSize, dataSize);
					_physicalDevice->logicalDevice().destroyBuffer(buf, mem);
					_buffer.at(i) = newBuffer;
					_bufferMemory.at(i) = newBufferMemory;
				}
				catch (const OutOfDeviceMemoryException&) {
					Vk_DataBufferLib::deviceLocalMemoryOverflowMessage(_physicalDevice, _objName, maxSize);
					// copy to cpu first, remove old buffer and then copy back
					_physicalDevice->logicalDevice().destroyBuffer(newBuffer, newBufferMemory);
					_getDataToCpu();
					_physicalDevice->logicalDevice().destroyBuffers(std::move(_buffer), std::move(_bufferMemory));
					_createDataBufferForUpdateStrategy(Vk_DataBufferLib::StructuredData{.count=_cpuDataBuffer.size(), .data=_cpuDataBuffer.data()});
				}
			}
		}

		void _updateDataBufferForUpdateStrategy(
			const Vk_DataBufferLib::StructuredData<TStructureType>& structuredData,
			size_t newFrom,
			size_t newTo
		) {
			size_t newDataCount = structuredData.count - newFrom;
			std::uint64_t offsetSize = static_cast<std::uint64_t>(newFrom * sizeof(TStructureType));
			std::uint64_t newDataSize = static_cast<std::uint64_t>(newDataCount * sizeof(TStructureType));

			if(newDataSize < sizeof(TStructureType)){
				UT::Ut_Logger::Error(typeid(this), "Update of object {0}-{1} failed: data size is {2} byte but must be greater than {3} byte!", _objName, _associatedObject, newDataSize, sizeof(TStructureType));
				return;
			}

			size_t newMaxCount;
			Vk_DataBufferLib::getNewBufferCount(_sizeBehaviour, _maxCount, structuredData.count, newMaxCount);
			Vk_DataBufferLib::updateMessage(_objName, _type, _sizeBehaviour, _updateBehaviour, _maxCount, newMaxCount, _count, structuredData.count);

			if (newMaxCount > _maxCount) {
				_resizeDataBufferForUpdateStrategy(newMaxCount, _count);
				_maxCount = newMaxCount;
			}

			if(newTo == 0){
				newTo = structuredData.count;
			}
			_copyDataToBufferForUpdateStrategy(structuredData, newFrom, newTo);

			_count = structuredData.count;
		}

		void _getDataToCpu() {
			std::uint64_t bs = bufferByteSize();

			// create host buffer to copy all necessary data into cpu accessible memory, lets call it stagingBuffer
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			Vk_DataBufferLib::createStagingBuffer(_physicalDevice, _type, stagingBuffer, stagingBufferMemory, bs, Vk_DataBufferLib::Usage::Destination, _gpuTargetOp);

			// copy data to staging buffer from non-cpu accessible vertex buffer
			// free memory afterwards
			std::string nn = "#Get" + _objName + _associatedObject;
			Vk_DataBufferLib::copyGpuToGpu(_physicalDevice, nn, _buffer.at(_bufferIndex), bs, stagingBuffer, bs, bs);

			// map memory into variable to actually use it
			_cpuDataBuffer.clear();
			_cpuDataBuffer.resize(bufferCount());
			Vk_DataBufferLib::copyGpuToCpu(_physicalDevice, stagingBufferMemory, _cpuDataBuffer.data(), bs);

			_physicalDevice->logicalDevice().destroyBuffer(stagingBuffer, stagingBufferMemory);
		}

		void _createDataBufferForUpdateStrategy(const Vk_DataBufferLib::StructuredData<TStructureType>& structuredData) {
			std::uint64_t dataSize = static_cast<std::uint64_t>(_bufferByteSize());
			std::uint64_t maxSize = static_cast<std::uint64_t>(_maxBufferByteSize());

			if(dataSize < sizeof(TStructureType)){
				UT::Ut_Logger::Error(typeid(this), "Create of object {0}-{1} failed: data size is {2} byte but must be at least {3} byte!", _objName, _associatedObject, dataSize, sizeof(TStructureType));
				return;
			}

			_allocateBufferForUpdateStrategy(maxSize);

			// copy data from staging buffer to non-cpu accessible vertex buffer
			// free memory afterwards
			if (structuredData.data != nullptr) {
				Vk_DataBufferLib::copyDataToBufferWithStaging(_physicalDevice, _type, _buffer.at(_bufferIndex), _maxBufferByteSize(), structuredData, 0, _bufferCount(), _objName, _associatedObject);
			}
		}

		void _allocateBufferForUpdateStrategy(std::uint64_t maxSize) {
			if(_updateBehaviour == Vk_BufferUpdateBehaviour::Staged_GlobalLock){
				// create one real buffer in non-cpu accessible memory
				VkBuffer lBuffer;
				VkDeviceMemory lBufferMemory;
				Vk_DataBufferLib::createDeviceLocalBuffer(_physicalDevice, _type, lBuffer, lBufferMemory, maxSize, Vk_DataBufferLib::Usage::Both, _gpuTargetOp);
				_buffer.push_back(lBuffer);
				_bufferMemory.push_back(lBufferMemory);
			}
			// else if(_updateBehaviour == Vk_BufferUpdateBehaviour::Staged_LazyDoubleBuffering){
			// 	// create one buffer and leave the other one empty
			// 	VkBuffer lBuffer;
			// 	VkDeviceMemory lBufferMemory;
			// 	Vk_DataBufferLib::createDeviceLocalBuffer(_physicalDevice, _type, lBuffer, lBufferMemory, maxSize, Vk_DataBufferLib::Usage::Both, _gpuTargetOp);
			// 	_buffer.push_back(lBuffer);
			// 	_bufferMemory.push_back(lBufferMemory);
			// 	_buffer.push_back(nullptr);
			// 	_bufferMemory.push_back(nullptr);
			// }
			else if(_updateBehaviour == Vk_BufferUpdateBehaviour::Staged_DoubleBuffering){
				// create two real _buffers in non-cpu accessible memory
				for(int i=0; i<2; ++i){
					VkBuffer lBuffer;
					VkDeviceMemory lBufferMemory;
					Vk_DataBufferLib::createDeviceLocalBuffer(_physicalDevice, _type, lBuffer, lBufferMemory, maxSize, Vk_DataBufferLib::Usage::Both, _gpuTargetOp);
					_buffer.push_back(lBuffer);
					_bufferMemory.push_back(lBufferMemory);
				}
			}
			else if(_updateBehaviour == Vk_BufferUpdateBehaviour::Direct_GlobalLock){
				// create one real _buffer in cpu-accessible memory
				VkBuffer lBuffer;
				VkDeviceMemory lBufferMemory;
				Vk_DataBufferLib::createDeviceLocalCPUAccessibleBuffer(_physicalDevice, _type, lBuffer, lBufferMemory, maxSize, Vk_DataBufferLib::Usage::Both, _gpuTargetOp);
				_buffer.push_back(lBuffer);
				_bufferMemory.push_back(lBufferMemory);
			}
			// else if(_updateBehaviour == Vk_BufferUpdateBehaviour::Direct_DoubleBuffering){
			// 	for(int i=0; i<2; ++i){
			// 		VkBuffer lBuffer;
			// 		VkDeviceMemory lBufferMemory;
			// 		Vk_DataBufferLib::createDeviceLocalCPUAccessibleBuffer(_physicalDevice, _type, lBuffer, lBufferMemory, maxSize, Vk_DataBufferLib::Usage::Both);
			// 		_buffer.push_back(lBuffer);
			// 		_bufferMemory.push_back(lBufferMemory);
			// 	}
			// }
			else {
				UT::Ut_Logger::RuntimeError(typeid(this), "Unsupported update behaviour: {0}", Vk_BufferUpdateBehaviourToString(_updateBehaviour));
			}
		}

		void flipIndex(){
			// Try to lock the localMutex to avoid a potential wait time. There should be no danger
			// of a deadlock here
			// if(_localMutex.try_lock()){
				// if we manage to get the lock, flip the buffer index...
				_bufferIndex = (this->_bufferIndex+1)%2; 
			// 	_localMutex.unlock();
			// }
			// else {
			// 	// ...otherwise, add the flip function to the device updates one more time
			// 	if(attempts == 0) return;

			// 	// try "attempts" times to flip the buffer. If it doesn't work, re-enqueue the
			// 	// flip command. Note: this is not recursion. It's adding one lambda function to a queue
			// 	// using the **up-to-date** _bufferIndex
			// 	_physicalDevice->bridge.addUpdateForNextFrame(
			// 		[this, &attempts](){ 
			// 			if(_localMutex.try_lock()){
			// 				this->_bufferIndex = (this->_bufferIndex+1)%2; 
			// 				_localMutex.unlock();
			// 			}
			// 			else{
			// 				flipIndex(attempts - 1);
			// 			}
			// 		}
			// 	);
			// }
		}

		void flipIndexAndErase(){
			if(_bufferIndex == 1){
				std::cout << "lol" << std::endl;
			}
			int oldInd = _bufferIndex;
			// Note: this can take some time...
			_physicalDevice->logicalDevice().destroyBuffer(buffer.at(oldInd), _bufferMemory.at(oldInd));
			_buffer.at(oldInd) = nullptr;
			_bufferMemory.at(oldInd) = nullptr;
			_bufferIndex = (_bufferIndex+1)%2; 
			std::cout << "######################################## " << _associatedObject << " | " << _objName << " | " << _bufferIndex << std::endl;
		}

		// void flipAndErase(int attempts){
		// 	// Try to lock the localMutex to avoid a potential wait time. There should be no danger
		// 	// of a deadlock here
		// 	if(_localMutex.try_lock()){
		// 		// if we manage to get the lock, flip the buffer index...
		// 		flipIndexAndErase();
		// 		_localMutex.unlock();
		// 	}
		// 	else {
		// 		// ...otherwise, add the flip function to the device updates one more time
		// 		if(attempts == 0) return;

		// 		// try "attempts" times to flip the buffer. If it doesn't work, re-enqueue the
		// 		// flip command. Note: this is not recursion. It's adding one lambda function to a queue
		// 		// using the **up-to-date** _bufferIndex
		// 		_physicalDevice->bridge.addUpdateForNextFrame(
		// 			[this, &attempts](){ 
		// 				if(_localMutex.try_lock()){
		// 					flipIndexAndErase();
		// 					_localMutex.unlock();
		// 				}
		// 				else{
		// 					flipAndErase(attempts - 1);
		// 				}
		// 			}
		// 		);
		// 	}
		// }

		void _copyDataToBufferForUpdateStrategy(const Vk_DataBufferLib::StructuredData<TStructureType>& structuredData, size_t from, size_t to){
			if(_updateBehaviour == Vk_BufferUpdateBehaviour::Staged_GlobalLock){
				// this one must be global lock because we copy data during a rendering process
				// which changes data that the rendering commands access during drawing
				auto lock = AcquireGlobalWriteLock("Vk_DataBuffer[_copyDataToBufferForUpdateStrategy]");
				Vk_DataBufferLib::copyDataToBufferWithStaging(_physicalDevice, _type, _buffer.at(0), maxBufferSize(), structuredData, from, to, _objName, _associatedObject);
			}
			else if(_updateBehaviour == Vk_BufferUpdateBehaviour::Direct_GlobalLock){
				// this one must be global lock because we copy data during a rendering process
				// which changes data that the rendering commands access during drawing
				auto lock = AcquireGlobalWriteLock("Vk_DataBuffer[_copyDataToBufferForUpdateStrategy]");
				Vk_DataBufferLib::copyDataToBufferDirect(_physicalDevice, _type, _bufferMemory.at(0), maxBufferSize(), structuredData, from, to, _objName, _associatedObject);
			}
			else if(_updateBehaviour == Vk_BufferUpdateBehaviour::Staged_DoubleBuffering){
				// copy to not used buffer and then flip them inside a synchronized bridge update
				// the update function is globally synched, so no need for mutexes here apart from the local one
				Vk_DataBufferLib::copyDataToBufferWithStaging(_physicalDevice, _type, _buffer.at((_bufferIndex+1)%2), maxBufferSize(), structuredData, from, to, _objName, _associatedObject);
				_physicalDevice->bridge.addUpdateForNextFrame( [this](){ this->flipIndex(); });
			}
			// else if(_updateBehaviour == Vk_BufferUpdateBehaviour::Direct_DoubleBuffering){
			// 	// copy to not used buffer and then flip them inside a synchronized bridge update
			// 	Vk_DataBufferLib::copyDataToBufferDirect(_physicalDevice, _type, _bufferMemory.at((_bufferIndex+1)%2), maxBufferSize(), structuredData, from, to, _objName, _associatedObject);
			// 	_physicalDevice->bridge.addUpdateForNextFrame( [this](){ this->flipIndex(); } );
			// }
			// else if(_updateBehaviour == Vk_BufferUpdateBehaviour::Staged_LazyDoubleBuffering){
			// 	// first allocate new buffer
			// 	std::uint64_t maxSize = static_cast<std::uint64_t>(maxBufferSize());
			// 	VkBuffer lBuffer;
			// 	VkDeviceMemory lBufferMemory;
			// 	Vk_DataBufferLib::createDeviceLocalBuffer(_physicalDevice, _type, lBuffer, lBufferMemory, maxSize, Vk_DataBufferLib::Usage::Both, _gpuTargetOp);
			// 	int ind = (_bufferIndex+1)%2;
			// 	if(_buffer.at(ind) != nullptr){
			// 		UT::Ut_Logger::RuntimeError(typeid(NoneObj), "Lazy allocated buffer at index {0} should be nullptr but isn't", ind);
			// 	}
			// 	_buffer.at(ind) = lBuffer;
			// 	_bufferMemory.at(ind) = lBufferMemory;
			// 	// then copy data into it
			// 	Vk_DataBufferLib::copyDataToBufferWithStaging(_physicalDevice, _type, _buffer.at(ind), maxBufferSize(), structuredData, from, to, _objName, _associatedObject);
			// 	// then schedule index switch and deletion of old data (this will be executed in sync, so no need for mutexes)
			// 	_physicalDevice->bridge.addUpdateForNextFrame( [this](){ this->flipIndexAndErase(); } );
			// }
			else {
				UT::Ut_Logger::RuntimeError(typeid(this), "Unknown update behaviour strategy {0}", Vk_BufferUpdateBehaviourToString(_updateBehaviour));
			}
		}
	};
}