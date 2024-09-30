#pragma once

#include "../Defines.h"
#include "../application/Vk_Device.h"
#include "../Vk_Lib.hpp"
#include "Vk_Structures.hpp"

namespace VK5 {
	enum class Vk_ObjUpdate {
		/*
		* Update registers a render commands rebuild and causes window to emit a PANT event
		*/
		Promptly,
		/* 
		* Only the async part of the update takes place. To finalize the update, at least one update
		* with 'Promptly' must take place. The idea is to allow for badges of updates and rebuild
		* the rendering commands only once and emit only one PAINT event instead of doing it for all
		* updates.
		*/
		Deferred	
	};

	/*
	* TODO: reinvestigate Staged_LazyDoubleBuffering. Currently there is trouble with the vkQueueSubmit
	* NOTE: Direct_DoubleBuffering probably will never work if the program is supposed to be interactive
	*       because it seems that UniformBuffer updates no longer work properly due to update contention
	* NOTE: Direct_GlobalLock works and is an improvement over Staged_GlobalLock but will probably also
	*       cause problems if too many objects are updated in this way each frame
	*/
	enum class Vk_BufferUpdateBehaviour {
		Staged_GlobalLock, 		   			/* one buffer but global lock at data transfer */
		Staged_DoubleBuffering,    			/* two buffers on GPU at all times */
		// Staged_LazyDoubleBuffering,		/* create a new buffer on update and switch at update time */
		/* TODO: add pinned staging for the three versions above where the staging buffer is preallocated */
		Direct_GlobalLock,	   				/* use CPU accessible memory on GPU with global lock at data transfer */
		// Direct_DoubleBuffering 			/* use CPU accessible memory on GPU with double buffering */
	};

	static std::string Vk_BufferUpdateBehaviourToString(Vk_BufferUpdateBehaviour behaviour) {
		switch (behaviour) {
			case Vk_BufferUpdateBehaviour::Staged_GlobalLock: return "Staged_GlobalLock";
			case Vk_BufferUpdateBehaviour::Staged_DoubleBuffering: return "Staged_DoubleBuffering";
			// case Vk_BufferUpdateBehaviour::Staged_LazyDoubleBuffering: return "Staged_LazyDoubleBuffering";
			case Vk_BufferUpdateBehaviour::Direct_GlobalLock: return "Direct_GlobalLock";
			// case Vk_BufferUpdateBehaviour::Direct_DoubleBuffering: return "Direct_DoubleBuffering";
			default: return "Unknown";
		}
	}

	enum class Vk_BufferSizeBehaviour {
		Init_Empty_Grow_1_5,
		Init_Empty_Grow_2,
		Init_1_0_Grow_1_5,
		Init_1_5_Grow_1_5,
		Init_1_0_Grow_2,
		Init_1_5_Grow_2
	};

	static std::string Vk_BufferSizeBehaviourToString(Vk_BufferSizeBehaviour behaviour) {
		switch (behaviour) {
		case Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5: return "Init_1_0_Grow_1_5";
		case Vk_BufferSizeBehaviour::Init_1_0_Grow_2: return "Init_1_0_Grow_2";
		case Vk_BufferSizeBehaviour::Init_1_5_Grow_1_5: return "Init_1_5_Grow_1_5";
		case Vk_BufferSizeBehaviour::Init_1_5_Grow_2: return "Init_1_5_Grow_2";
		case Vk_BufferSizeBehaviour::Init_Empty_Grow_1_5: return "Init_Empty_Grow_1_5";
		case Vk_BufferSizeBehaviour::Init_Empty_Grow_2: return "Init_Empty_Grow_2";
		default: return "Unknown";
		}
	}

    class Vk_DataBufferLib {
    public:
        enum class Usage {
			Both,
			Source,
			Destination,
		};

        enum class BufferType {
            P,
            C,
            N,
            T,
            PC,
            PCN,
            PCNT,
            Index,
            Error
        };

        static std::string BufferTypeToString(BufferType type) {
            switch (type) {
            case BufferType::P: return "P";
            case BufferType::C: return "C";
            case BufferType::N: return "N";
            case BufferType::T: return "T";
            case BufferType::PC: return "PC";
            case BufferType::Index: return "Index";
            case BufferType::PCN: return "PCN";
            case BufferType::PCNT: return "PCNT";
            case BufferType::Error: return "Error";
            default: return "Unknown";
            }
        }

		template<class TStructureType>
		struct StructuredData {
			size_t count;
			const TStructureType* data;
		};

        template<class TStructureType>
        static BufferType getInitBufferType() {
			std::string name = std::string(typeid(TStructureType).name());

			if (name.compare(std::string(typeid(Vk_Vertex_P).name())) == 0) return BufferType::P;
			if (name.compare(std::string(typeid(Vk_Vertex_C).name())) == 0) return BufferType::C;
			if (name.compare(std::string(typeid(Vk_Vertex_N).name())) == 0) return BufferType::N;
			if (name.compare(std::string(typeid(Vk_Vertex_T).name())) == 0) return BufferType::T;
			if (name.compare(std::string(typeid(Vk_Vertex_PC).name())) == 0) return BufferType::PC;
			else if (name.compare(std::string(typeid(Vk_Vertex_PCN).name())) == 0) return BufferType::PCN;
			else if (name.compare(std::string(typeid(Vk_Vertex_PCNT).name())) == 0) return BufferType::PCNT;
			else if (name.compare(std::string(typeid(VK5::index_type).name())) == 0) return BufferType::Index;

			UT::Ut_Logger::RuntimeError(typeid(NoneObj), "Unable to set buffer type to [{0}]. Type is not supported!", name);
			return BufferType::Error;
		}

        static size_t getInitMaxCount(size_t count, Vk_BufferSizeBehaviour sizeBehaviour) {
			if ((sizeBehaviour == Vk_BufferSizeBehaviour::Init_Empty_Grow_1_5) || (sizeBehaviour == Vk_BufferSizeBehaviour::Init_Empty_Grow_2))
				return static_cast<size_t>(std::ceil(1.0 * count));
			if ((sizeBehaviour == Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5) || (sizeBehaviour == Vk_BufferSizeBehaviour::Init_1_0_Grow_2))
				return static_cast<size_t>(std::ceil(1.0 * count));
			if ((sizeBehaviour == Vk_BufferSizeBehaviour::Init_1_5_Grow_1_5) || (sizeBehaviour == Vk_BufferSizeBehaviour::Init_1_5_Grow_2))
				return static_cast<size_t>(std::ceil(1.5 * count));
			return 0;
		}

        static size_t getNextMaxCount(Vk_BufferSizeBehaviour sizeBehaviour, size_t oldMaxCount) {
			if (
				(sizeBehaviour == Vk_BufferSizeBehaviour::Init_1_0_Grow_2) ||
				(sizeBehaviour == Vk_BufferSizeBehaviour::Init_1_5_Grow_2) ||
				(sizeBehaviour == Vk_BufferSizeBehaviour::Init_Empty_Grow_2)) 
			{
				return static_cast<size_t>(std::ceil(oldMaxCount * 2.0));
			}

			if (
				(sizeBehaviour == Vk_BufferSizeBehaviour::Init_1_0_Grow_1_5) || 
				(sizeBehaviour == Vk_BufferSizeBehaviour::Init_1_5_Grow_1_5) ||
				(sizeBehaviour == Vk_BufferSizeBehaviour::Init_Empty_Grow_1_5)) 
			{
				return static_cast<size_t>(std::ceil(oldMaxCount * 1.5));
			}

			UT::Ut_Logger::RuntimeError(typeid(NoneObj), "Unable to get new max count from [{0}]. Behaviour characteristics is not supported", oldMaxCount);
			return 0;
		}

        static bool getNewBufferCount(Vk_BufferSizeBehaviour sizeBehaviour, const size_t oldMaxCount, const size_t newCount, size_t& newMaxCount) {
			bool resize = false;
			newMaxCount = oldMaxCount;
			while (newMaxCount < newCount) {
				newMaxCount = getNextMaxCount(sizeBehaviour, newMaxCount);
				resize = true;
			}
			return resize;
		}

        static VkBufferUsageFlags getUsageFlags(BufferType type, Usage usage, bool staging=false){
			VkBufferUsageFlags usageFlags = 0;
			if (!staging) {
				usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
				if (type == BufferType::Index) {
					// we have an index buffer
					usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
				}
			}
			if (usage == Usage::Both) usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			else if (usage == Usage::Destination) usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			else if (usage == Usage::Source) usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

			return usageFlags;
		}

        static void checkResizeAsserts(const std::string& objName, size_t count, size_t newCount, size_t newFrom) {
			if(newCount < 0) UT::Ut_Logger::RuntimeError(typeid(NoneObj), "[{0}] New count must be >= 0 but is {1}", objName, newCount);
			if(newCount < count) UT::Ut_Logger::RuntimeError(typeid(NoneObj), "[{0}] New count must be >= current data count. Current data count is {1} but new count is {2}", objName, count, newCount);				
			if(newCount < newFrom) UT::Ut_Logger::RuntimeError(typeid(NoneObj), "[{0}] New count must be > newFrom. Current new count is {1} but newFrom is {2}", objName, newCount, newFrom);				
		}

        static void checkAsserts(const std::string& objName, size_t bufferCount) {
			if(bufferCount < 0) UT::Ut_Logger::RuntimeError(typeid(NoneObj), "[{0}] Buffer size >= 0 required but is {1}", objName, bufferCount);
		}

		static void createMessage(
            const std::string& objName,
            BufferType type, Vk_BufferSizeBehaviour sizeBehaviour, uint64_t maxCount, uint64_t count
        ) {
			UT::Ut_Logger::Trace(typeid(NoneObj), 
				Vk_Lib::formatWithObjName(objName, (
					UT::GlobalCasters::castYellow("\n\tBuffer create: ") + std::string("Create new buffer\n")
					+ UT::GlobalCasters::castYellow("\tBuffer create: ") + std::string("            type: ") + BufferTypeToString(type) + "\n"
					+ UT::GlobalCasters::castYellow("\tBuffer create: ") + std::string(" characteristics: ") + Vk_BufferSizeBehaviourToString(sizeBehaviour) + "\n"
					+ UT::GlobalCasters::castYellow("\tBuffer create: ") + std::string("       max count: ") + std::to_string(maxCount) + "\n"
					+ UT::GlobalCasters::castYellow("\tBuffer create: ") + std::string("  required count: ") + std::to_string(count) + "\n"
				))
			);
		}

		static void updateMessage(
            const std::string& objName, 
            BufferType type, Vk_BufferSizeBehaviour sizeBehaviour, Vk_BufferUpdateBehaviour updateBehaviour,
            size_t oldMaxCount, size_t newMaxCount, 
            size_t oldDataCount, size_t newDataCount
        ) {
			UT::Ut_Logger::Trace(typeid(NoneObj), 
				Vk_Lib::formatWithObjName(objName, (
					UT::GlobalCasters::castYellow("\n\tBuffer update: ") + std::string("Update buffer\n")
					+ UT::GlobalCasters::castYellow("\tBuffer update: ") + std::string("            type: ") + BufferTypeToString(type) + "\n"
					+ UT::GlobalCasters::castYellow("\tBuffer update: ") + std::string("  size behaviour: ") + Vk_BufferSizeBehaviourToString(sizeBehaviour) + "\n"
					+ UT::GlobalCasters::castYellow("\tBuffer update: ") + std::string(" update behavour: ") + Vk_BufferUpdateBehaviourToString(updateBehaviour) + "\n"
					+ UT::GlobalCasters::castYellow("\tBuffer update: ") + std::string("       old count: ") + std::to_string(oldDataCount) + "\n"
					+ UT::GlobalCasters::castYellow("\tBuffer update: ") + std::string("       new count: ") + std::to_string(newDataCount) + "\n"
					+ UT::GlobalCasters::castYellow("\tBuffer update: ") + std::string("   old max count: ") + std::to_string(oldMaxCount) + "\n"
					+ UT::GlobalCasters::castYellow("\tBuffer update: ") + std::string("   new max count: ") + std::to_string(newMaxCount) + "\n"
				))
			);
		}

        static std::uint64_t getDeviceLocalBufferMaxMemory(Vk_PhysicalDevice* physicalDevice) {
			return physicalDevice->queryPhysicalDeviceHeapSize(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT).size;
		}

		static void deviceLocalMemoryOverflowMessage(Vk_PhysicalDevice* physicalDevice, const std::string& objName, size_t requested) {
            auto devMem = getDeviceLocalBufferMaxMemory(physicalDevice);
			UT::Ut_Logger::Warn(typeid(NoneObj), 
				Vk_Lib::formatWithObjName(objName, (
					UT::GlobalCasters::castYellow("Device-local memory overflow:")
					+ std::string(" available memory budget is ")
					+ UT::GlobalCasters::castHighlightYellow(std::to_string(devMem))
					+ std::string(" but requested memory for device-local resize is ")
					+ UT::GlobalCasters::castHighlightYellow(std::to_string(devMem + requested))
				))
			);
		}

		static void resizeMessage(const std::string& objName, BufferType type, Vk_BufferSizeBehaviour sizeBehaviour, size_t maxCount, size_t newMaxCount, size_t count) {
			UT::Ut_Logger::Trace(typeid(NoneObj), 
				Vk_Lib::formatWithObjName(objName, (
					UT::GlobalCasters::castYellow("\n\tBuffer resize: ") + std::string("Resize buffer\n")
					+ UT::GlobalCasters::castYellow("\tBuffer resize: ") + std::string("            type: ") + BufferTypeToString(type) + "\n"
					+ UT::GlobalCasters::castYellow("\tBuffer resize: ") + std::string(" characteristics: ") + Vk_BufferSizeBehaviourToString(sizeBehaviour) + "\n"
					+ UT::GlobalCasters::castYellow("\tBuffer resize: ") + std::string("   old max count: ") + std::to_string(maxCount) + "\n"
					+ UT::GlobalCasters::castYellow("\tBuffer resize: ") + std::string("   new max count: ") + std::to_string(newMaxCount) + "\n"
					+ UT::GlobalCasters::castYellow("\tBuffer resize: ") + std::string("  required count: ") + std::to_string(count) + "\n"
				))
			);
		}

        template<class TStructureType>
		static void warnOutOfSize(const std::string& objName, size_t maxCount, size_t count) {
			UT::Ut_Logger::Warn(typeid(NoneObj), 
				Vk_Lib::formatWithObjName(objName, (
					UT::GlobalCasters::castYellow("\n\tBuffer out of size: ") + std::string("\n******************************************************************************************\n")
					+ UT::GlobalCasters::castYellow("\tBuffer out of size: ") + std::string("Extendable Vertex Buffer of type \"")
					+ UT::GlobalCasters::castYellow("\tBuffer out of size: ") + std::string(typeid(TStructureType).name())
					+ UT::GlobalCasters::castYellow("\tBuffer out of size: ") + std::string("\" has initial size ")
					+ UT::GlobalCasters::castYellow("\tBuffer out of size: ") + std::to_string(maxCount)
					+ UT::GlobalCasters::castYellow("\tBuffer out of size: ") + std::string(" but requires initial size of ")
					+ UT::GlobalCasters::castYellow("\tBuffer out of size: ") + std::to_string(count)
					+ UT::GlobalCasters::castYellow("\tBuffer out of size: ") + std::string("\nThe max size is increased to match. Consider allocating more initial space.")
					+ UT::GlobalCasters::castYellow("\tBuffer out of size: ") + std::string("\n******************************************************************************************\n")
				))
			);
		}

        template<class TStructureType>
		static void copyGpuToCpu(Vk_PhysicalDevice* physicalDevice, VkDeviceMemory gpuMemoryPtr, TStructureType* cpuMemoryPtr, std::uint64_t copyByteSize) {
			VkDevice lDev = physicalDevice->vk_logicalDevice();

			void* data;
			vkMapMemory(lDev, gpuMemoryPtr, 0, static_cast<VkDeviceSize>(copyByteSize), 0, &data);
			memcpy(static_cast<void*>(cpuMemoryPtr), data, copyByteSize);
			vkUnmapMemory(lDev, gpuMemoryPtr);
		}

		static Vk_GpuTaskRunner* copyGpuToGpu(
            Vk_PhysicalDevice* physicalDevice,
            const std::string& objName,
            VkBuffer srcBuffer, std::uint64_t srcBufferSize, 
            VkBuffer dstBuffer, std::uint64_t dstBufferSize,
            std::uint64_t copyByteSize, 
            std::uint64_t srcByteOffset=0, std::uint64_t dstByteOffset=0
        ) {
			// make sure that we access inside the source buffer
			assert(srcBufferSize >= srcByteOffset + copyByteSize);
			// make sure that we access inside the dst buffer
			assert(dstBufferSize >= dstByteOffset + copyByteSize);

			return physicalDevice->enqueue(std::move(Vk_GpuTaskLib::Vk_CopyGpuToGpu(
				srcBuffer, static_cast<VkDeviceSize>(srcByteOffset),
				dstBuffer, static_cast<VkDeviceSize>(dstByteOffset),
				static_cast<VkDeviceSize>(copyByteSize), Vk_GpuTargetOp::Auto
			)), {});
		}

        template<class TStructureType>
		static void copyCpuToGpu(
            Vk_PhysicalDevice* physicalDevice,
            const Vk_DataBufferLib::StructuredData<TStructureType>& structuredData, 
			VkDeviceMemory gpuMemoryPtr, 
			std::uint64_t copyByteSize, std::uint64_t srcByteOffset=0, std::uint64_t dstByteOffset=0
        ) {
			// need srcByteOffset to be at most the total data size-1
			assert(structuredData.count*sizeof(TStructureType) >= srcByteOffset + copyByteSize);
			const TStructureType* offsetCpuMemoryPtr = structuredData.data + (srcByteOffset /sizeof(TStructureType));
			physicalDevice->copyCpuToGpu(offsetCpuMemoryPtr, gpuMemoryPtr, copyByteSize, srcByteOffset, dstByteOffset);
		}

		static void createDeviceLocalBuffer(
            Vk_PhysicalDevice* physicalDevice, BufferType type,
            VkBuffer& buffer, VkDeviceMemory& memory, VkDeviceSize size, 
			Usage usage, Vk_GpuTargetOp gpuTargetOp
        ) {
			VkBufferUsageFlags usageFlags = getUsageFlags(type, usage);
			physicalDevice->createAndAllocBuffer(usageFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, memory, size, gpuTargetOp);
		}

		static void createDeviceLocalCPUAccessibleBuffer(
            Vk_PhysicalDevice* physicalDevice, BufferType type,
            VkBuffer& buffer, VkDeviceMemory& memory, VkDeviceSize size, 
            Usage usage, Vk_GpuTargetOp gpuTargetOp
        ) {
			VkBufferUsageFlags usageFlags = getUsageFlags(type, usage);
			physicalDevice->createAndAllocBuffer(
				usageFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				buffer, memory, size, gpuTargetOp);
		}

        static void createStagingBuffer(
            Vk_PhysicalDevice* physicalDevice, BufferType type,
            VkBuffer& buffer, VkDeviceMemory& memory, std::uint64_t size, 
            Usage usage, Vk_GpuTargetOp gpuTargetOp
        ) {
			// it may be that memcpy does not copy the data right away (chaching and so on) ensure either
			//  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT is set for memory area (slightly worse performance)
			//  or vkFlushMappedMemoryRanges after writing and vkInvalidateMappedMemoryRanges after reading
			VkBufferUsageFlags usageFlags = getUsageFlags(type, usage, true);
			physicalDevice->createAndAllocBuffer(
				usageFlags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				buffer, memory, size, gpuTargetOp);
		}

        template<class TStructureType>
        static void copyDataToBufferWithStaging(
            Vk_PhysicalDevice* physicalDevice,
            BufferType type,
            VkBuffer buffer, 
			uint64_t bufferByteSize,
            const Vk_DataBufferLib::StructuredData<TStructureType>& structuredData,
            size_t from, 
            size_t to,
            const std::string& objName = "",
            const std::string& associatedObject = ""
        ){
            if(from > to){
                UT::Ut_Logger::RuntimeError(typeid(NoneObj), "Data [from, to] must be an interval of positive length but is {0} items long!", to-from);
            }

			// create host buffer to copy all necessary data into cpu accessible memory, lets call it stagingBuffer
			VkBuffer stagingBuffer = VK_NULL_HANDLE;
			VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
			uint64_t dataSize = static_cast<uint64_t>(structuredData.count * sizeof(TStructureType));
			uint64_t byteFrom = static_cast<uint64_t>(from * sizeof(TStructureType));
			uint64_t byteTo = static_cast<uint64_t>(to * sizeof(TStructureType));

			// if the buffer is initially empty, no need to copy random stuff, just create the vertex buffer
			// auto t1 = std::chrono::high_resolution_clock::now();
			createStagingBuffer(physicalDevice, type, stagingBuffer, stagingBufferMemory, byteTo - byteFrom, Usage::Source, Gk_GpuTargetOp::Auto);
			// auto t2 = std::chrono::high_resolution_clock::now();
			// std::cout << "############################333" << std::endl << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << std::endl << "############################333" << std::endl;

            std::string nn = "#Create#" + objName + associatedObject;
			// map memory into variable to actually use it
			uint64_t copyByteSize = byteTo - byteFrom;
			// srcByteOffset = byteFrom, dstByteOffset = 0 because that is the staging buffer offset that only houses the new data
			copyCpuToGpu(physicalDevice, structuredData, stagingBufferMemory, copyByteSize, byteFrom, 0);
			// srcByteOffset = 0 because that is the staging buffer offset that only houses the new data, 
			// dstByteOffset = byteFrom because we need to place the data in the right spot
			copyGpuToGpu(physicalDevice, nn, stagingBuffer, byteTo - byteFrom, buffer, bufferByteSize, copyByteSize, 0, byteFrom);
			physicalDevice->logicalDevice().destroyBuffer(stagingBuffer, stagingBufferMemory);
		}

		template<class TStructureType>
        static void copyDataToBufferDirect(
            Vk_PhysicalDevice* physicalDevice,
            BufferType type,
            VkDeviceMemory bufferMemory, 
			uint64_t bufferByteSize,
            const Vk_DataBufferLib::StructuredData<TStructureType>& structuredData,
            size_t from, 
            size_t to,
            const std::string& objName = "",
            const std::string& associatedObject = ""
        ){
            if(from > to){
                UT::Ut_Logger::RuntimeError(typeid(NoneObj), "Data [from, to] must be an interval of positive length but is {0} items long!", to-from);
            }
			uint64_t byteFrom = static_cast<uint64_t>(from * sizeof(TStructureType));
			uint64_t byteTo = static_cast<uint64_t>(to * sizeof(TStructureType));

            // std::string nn = "#Create#" + objName + associatedObject;
			// map memory into variable to actually use it
			uint64_t copyByteSize = byteTo - byteFrom;
			// srcByteOffset = byteFrom, dstByteOffset = 0 because that is the staging buffer offset that only houses the new data
			copyCpuToGpu(physicalDevice, structuredData, bufferMemory, copyByteSize, byteFrom, byteFrom);
		}

        static std::uint64_t getStagingBufferMaxMemory(Vk_PhysicalDevice* physicalDevice) {
			return physicalDevice->queryPhysicalDeviceHeapSize(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT).size;
		}

		static std::uint64_t getStagingBufferMemoryBudget(Vk_PhysicalDevice* physicalDevice) {
			return physicalDevice->queryPhysicalDeviceHeapBudget(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT).size;
		}
    };
}