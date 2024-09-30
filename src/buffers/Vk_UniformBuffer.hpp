#pragma once

#include <vector>
// #include <vulkan/vulkan.h>
#include <mutex>

//#define TIME_MEASURE_UB
#ifdef TIME_MEASURE_UB
#include <chrono>
#include <iostream>
#endif

#include "../Defines.h"
#include "../application/Vk_Device.h"

namespace VK5 {

	struct UniformBufferType_RendererMat4 {
		glm::tmat4x4<VK5::point_type> mat;

		static bool compare(UniformBufferType_RendererMat4& mat1, UniformBufferType_RendererMat4& mat2) {
			bool same = true;
			int size = mat1.mat.length();
			for (int i = 0; i < size; ++i) {
				auto m1 = mat1.mat[i];
				auto m2 = mat2.mat[i];
				glm::bvec4 res = glm::equal(m1, m2);
				same = same && res.x && res.y && res.z && res.w;
			}
			return same;
		}

		//static bool compare(const Vk_Structure_Vertex& vertex1, const Vk_Structure_Vertex& vertex2) {
		//	glm::bvec3 res1 = glm::equal(vertex1.pos, vertex2.pos);
		//	return res1.x && res1.y && res1.z;
		//}
	};

	struct UniformBufferType_ModelMat4 {
		glm::tmat4x4<VK5::point_type> mat;

		static bool compare(UniformBufferType_ModelMat4& mat1, UniformBufferType_ModelMat4& mat2) {
			bool same = true;
			int size = mat1.mat.length();
			for (int i = 0; i < size; ++i) {
				auto m1 = mat1.mat[i];
				auto m2 = mat2.mat[i];
				glm::bvec4 res = glm::equal(m1, m2);
				same = same && res.x && res.y && res.z && res.w;
			}
			return same;
		}

		//static bool compare(const Vk_Structure_Vertex& vertex1, const Vk_Structure_Vertex& vertex2) {
		//	glm::bvec3 res1 = glm::equal(vertex1.pos, vertex2.pos);
		//	return res1.x && res1.y && res1.z;
		//}
	};

	class Vk_AbstractUniformBuffer {
	public:
		Vk_AbstractUniformBuffer() {}
		virtual ~Vk_AbstractUniformBuffer() {}

		virtual std::vector<VkBuffer>* vk_uniformBuffers() = 0;
		virtual VkBuffer vk_uniformBuffer(int index) = 0;
		virtual VkDeviceSize vk_sizeofBuffer() = 0;
		virtual void vk_update(const std::uint32_t imageIndex, const void* uniformBuffer) = 0;
	};

	template<class TUniformBufferType>
	class Vk_UniformBuffer: public Vk_AbstractUniformBuffer {
	public:
		Vk_UniformBuffer(
			Vk_Device* const device,
			const std::string& associatedObject,
			uint32_t frameBufferCount,
			const TUniformBufferType& data
		) 
			: 
			_device(device),
			_frameBufferCount(frameBufferCount),
			_associatedObject("(=" + associatedObject + "=)"),
			_uniformBuffers({}),
			_uniformBuffersMemory({})
		{
			Vk_Logger::Log(typeid(this), GlobalCasters::castConstructorTitle(std::string("Create Uniform Buffer ") + std::string(typeid(TUniformBufferType).name()) + _associatedObject));

			createUniformBuffers();
			initUniformBuffer(data);
		}

		~Vk_UniformBuffer() {
			Vk_Logger::Log(typeid(this), GlobalCasters::castDestructorTitle(std::string("Destroy Uniform Buffer ") + std::string(typeid(TUniformBufferType).name()) + _associatedObject));

			VkDevice lDev = _device->vk_lDev();

			std::vector<VkBuffer>::iterator iterB = _uniformBuffers.begin();
			std::vector<VkBuffer>::iterator endB = _uniformBuffers.end();
			std::vector<VkDeviceMemory>::iterator iterM = _uniformBuffersMemory.begin();
			std::vector<VkDeviceMemory>::iterator endM = _uniformBuffersMemory.end();

			while(iterB != endB){
				vkDestroyBuffer(lDev, *iterB, nullptr);
				++iterB;
			}
			while (iterM != endM) {
				vkFreeMemory(lDev, *iterM, nullptr);
				++iterM;
			}
		}

		std::vector<VkBuffer>* vk_uniformBuffers() override {
			return &_uniformBuffers;
		}

		VkBuffer vk_uniformBuffer(int index) override {
			return _uniformBuffers[index];
		}

		VkDeviceSize vk_sizeofBuffer() override {
			return sizeof(TUniformBufferType);
		}

		void vk_update(const std::uint32_t imageIndex, const void* uniformBuffer) override {
			update(imageIndex, reinterpret_cast<const TUniformBufferType*>(uniformBuffer));
		}

		TUniformBufferType vk_getData(std::uint32_t imageIndex) {
			return getDataFromGpu(imageIndex);
		}

		int vk_frameCount() { return static_cast<int>(_uniformBuffers.size()); }

	private:
		Vk_Device* _device;
		uint32_t _frameBufferCount;
		std::mutex _localMutex;
		std::string _associatedObject;

		std::vector<VkBuffer> _uniformBuffers;
		std::vector<VkDeviceMemory> _uniformBuffersMemory;

		void initUniformBuffer(const TUniformBufferType& data) {
#ifdef TIME_MEASURE_UB
			std::cout << GlobalCasters::castHighlightGreen(GlobalCasters::castRed("Start create =====================")) << std::endl;
#endif
			for (std::uint32_t i = 0; i < _frameBufferCount; ++i) {
				update(i, &data);
			}
#ifdef TIME_MEASURE_UB
			std::cout << GlobalCasters::castHighlightGreen(GlobalCasters::castRed("End create =======================")) << std::endl;
#endif
		}

		void update(std::uint32_t imageIndex, const TUniformBufferType* uniformBuffer) {
			void* data;
			VkDevice lDev = _device->vk_lDev();
			{
#ifdef TIME_MEASURE_UB
				auto start = std::chrono::high_resolution_clock::now();
#endif
				vkMapMemory(lDev, _uniformBuffersMemory[imageIndex], 0, sizeof(TUniformBufferType), 0, &data);
#ifdef TIME_MEASURE_UB
				auto end = std::chrono::high_resolution_clock::now();
				auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
				std::cout << GlobalCasters::castHighlightCyan("Update: Map memmory time diff:\t\t") << diff << std::endl;

				start = std::chrono::high_resolution_clock::now();
#endif
				memcpy(data, uniformBuffer, sizeof(TUniformBufferType));
#ifdef TIME_MEASURE_UB
				end = std::chrono::high_resolution_clock::now();
				diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
				std::cout << GlobalCasters::castHighlightGreen("Update: Memcpy time diff:\t\t") << diff << std::endl;

				start = std::chrono::high_resolution_clock::now();
#endif
				vkUnmapMemory(lDev, _uniformBuffersMemory[imageIndex]);
#ifdef TIME_MEASURE_UB
				end = std::chrono::high_resolution_clock::now();
				diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
				std::cout << GlobalCasters::castHighlightYellow("Update: Unmap time diff:\t\t") << diff << std::endl;
#endif
			}
		}

		TUniformBufferType getDataFromGpu(std::uint32_t imageIndex) {
			void* data;
			TUniformBufferType uniformBuffer;
			VkDevice lDev = _device->vk_lDev();
			{
#ifdef TIME_MEASURE_UB
				auto start = std::chrono::high_resolution_clock::now();
#endif
				vkMapMemory(lDev, _uniformBuffersMemory[imageIndex], 0, sizeof(TUniformBufferType), 0, &data);
#ifdef TIME_MEASURE_UB
				auto end = std::chrono::high_resolution_clock::now();
				auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
				std::cout << GlobalCasters::castHighlightGreen("Get: Map memmory time diff:\t\t") << diff << std::endl;

				start = std::chrono::high_resolution_clock::now();
#endif
				memcpy(&uniformBuffer, data, sizeof(TUniformBufferType));
#ifdef TIME_MEASURE_UB
				end = std::chrono::high_resolution_clock::now();
				diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
				std::cout << GlobalCasters::castHighlightRed("Get: Memcpy time diff:\t\t") << diff << std::endl;

				start = std::chrono::high_resolution_clock::now();
#endif
				vkUnmapMemory(lDev, _uniformBuffersMemory[imageIndex]);
#ifdef TIME_MEASURE_UB
				end = std::chrono::high_resolution_clock::now();
				diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
				std::cout << GlobalCasters::castHighlightYellow("Get: Unmap time diff:\t\t") << diff << std::endl;
#endif
			}
			return uniformBuffer;
		}

		void createUniformBuffers()
		{
			VkDeviceSize bufferSize = sizeof(TUniformBufferType);

			_uniformBuffers.resize(_frameBufferCount);
			_uniformBuffersMemory.resize(_frameBufferCount);

			for (size_t i = 0; i < _frameBufferCount; ++i) {
				_device->vk_createBuffer(
					VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					_uniformBuffers[i],
					_uniformBuffersMemory[i],
					bufferSize);
			}

			// don't use vkMapMemory here because we will update this memory before every frame rendering operation
		}
	};
}