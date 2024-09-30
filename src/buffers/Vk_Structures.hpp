#pragma once

#include <array>

#include "../Defines.h"
// #include "../Vk_Logger.hpp"

namespace VK5 {
	struct Vk_Vertex_PC {
		glm::tvec3<VK5::point_type> pos;
		glm::tvec3<VK5::point_type> color;

		static int innerDimensionLen(){ return 6; }

		static bool compare(const Vk_Vertex_PC& s1, const Vk_Vertex_PC& s2) {
			glm::bvec3 res1 = glm::equal(s1.pos, s2.pos);
			glm::bvec3 res2 = glm::equal(s1.color, s2.color);
			return res1.x && res1.y && res1.z && res2.x && res2.y && res2.z;
		}

		static VkVertexInputBindingDescription getBindingDescription(
			uint32_t bindingDescriptionIndex
		) {
			VkVertexInputBindingDescription bindingDescription{};
			// layout(location = [bindingDescriptionIndex]) in vec3 inVertex;
			bindingDescription.binding = bindingDescriptionIndex;
			bindingDescription.stride = sizeof(Vk_Vertex_PC);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(
			uint32_t bindingDescriptionIndex,
			uint32_t positionLocation,
			uint32_t colorLocation
		) {
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

			// layout(location = [bindingDescriptionIndex]) in vec3 inVertex;
			attributeDescriptions[0].binding = bindingDescriptionIndex;
			attributeDescriptions[0].location = positionLocation;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vk_Vertex_PC, pos);

			// layout(location = [bindingDescriptionIndex]) in vec3 inVertex;
			attributeDescriptions[1].binding = bindingDescriptionIndex;
			attributeDescriptions[1].location = colorLocation;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vk_Vertex_PC, color);

			return attributeDescriptions;
		}
	};

	struct Vk_Vertex_PCN {
		glm::tvec3<VK5::point_type> pos;
		glm::tvec3<VK5::point_type> color;
		glm::tvec3<VK5::point_type> normal;

		static int innerDimensionLen(){ return 9; }

		static bool compare(const Vk_Vertex_PCN& s1, const Vk_Vertex_PCN& s2) {
			glm::bvec3 res1 = glm::equal(s1.pos, s2.pos);
			glm::bvec3 res2 = glm::equal(s1.color, s2.color);
			glm::bvec3 res3 = glm::equal(s1.normal, s2.normal);
			return
				res1.x && res1.y && res1.z &&
				res2.x && res2.y && res2.z &&
				res3.x && res3.y && res3.z;
		}

		static std::uint32_t bindingDescriptionIndex() {
			return 0;
		}

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = bindingDescriptionIndex();
			bindingDescription.stride = sizeof(Vk_Vertex_PCN);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(
			uint32_t bindingDescriptionIndex,
			uint32_t positionLocation,
			uint32_t colorLocation,
			uint32_t normalLocation
		) {
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);

			// layout(location = [bindingDescriptionIndex]) in vec3 inVertex;
			attributeDescriptions[0].binding = bindingDescriptionIndex;
			attributeDescriptions[0].location = positionLocation;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vk_Vertex_PCN, pos);

			// layout(location = [bindingDescriptionIndex]) in vec3 inVertex;
			attributeDescriptions[1].binding = bindingDescriptionIndex;
			attributeDescriptions[1].location = colorLocation;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vk_Vertex_PCN, color);

			// layout(location = [bindingDescriptionIndex]) in vec3 inVertex;
			attributeDescriptions[2].binding = bindingDescriptionIndex;
			attributeDescriptions[2].location = normalLocation;
			attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vk_Vertex_PCN, normal);

			return attributeDescriptions;
		}
	};

	struct Vk_Vertex_PCNT {
		glm::tvec3<VK5::point_type> pos;
		glm::tvec3<VK5::point_type> color;
		glm::tvec3<VK5::point_type> normal;
		glm::tvec2<VK5::point_type> uv;

		static int innerDimensionLen(){ return 11; }

		static bool compare(const Vk_Vertex_PCNT& s1, const Vk_Vertex_PCNT& s2) {
			glm::bvec3 res1 = glm::equal(s1.pos, s2.pos);
			glm::bvec3 res2 = glm::equal(s1.color, s2.color);
			glm::bvec3 res3 = glm::equal(s1.normal, s2.normal);
			glm::bvec2 res4 = glm::equal(s1.uv, s2.uv);
			return
				res1.x && res1.y && res1.z &&
				res2.x && res2.y && res2.z &&
				res3.x && res3.y && res3.z &&
				res4.x && res4.y;
		}

		static VkVertexInputBindingDescription getBindingDescription(
			uint32_t bindingDescriptionIndex
		) {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = bindingDescriptionIndex;
			bindingDescription.stride = sizeof(Vk_Vertex_PCNT);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(
			uint32_t bindingDescriptionIndex,
			uint32_t positionLocation,
			uint32_t colorLocation,
			uint32_t normalLocation,
			uint32_t uvLocation
		) {
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);

			// layout(location = [bindingDescriptionIndex]) in vec3 inVertex;
			attributeDescriptions[0].binding = bindingDescriptionIndex;
			attributeDescriptions[0].location = positionLocation;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vk_Vertex_PCNT, pos);

			// layout(location = [bindingDescriptionIndex]) in vec3 inVertex;
			attributeDescriptions[1].binding = bindingDescriptionIndex;
			attributeDescriptions[1].location = colorLocation;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vk_Vertex_PCNT, color);

			// layout(location = [bindingDescriptionIndex]) in vec3 inVertex;
			attributeDescriptions[2].binding = bindingDescriptionIndex;
			attributeDescriptions[2].location = normalLocation;
			attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vk_Vertex_PCNT, normal);

			// layout(location = [bindingDescriptionIndex]) in vec3 inVertex;
			attributeDescriptions[3].binding = bindingDescriptionIndex;
			attributeDescriptions[3].location = uvLocation;
			attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[3].offset = offsetof(Vk_Vertex_PCNT, uv);

			return attributeDescriptions;
		}
	};
	
	struct Vk_Vertex_P {

		glm::tvec3<VK5::point_type> pos;

		static int innerDimensionLen(){ return 3; }

		static bool compare(const Vk_Vertex_P& vertex1, const Vk_Vertex_P& vertex2) {
			glm::bvec3 res1 = glm::equal(vertex1.pos, vertex2.pos);
			return res1.x && res1.y && res1.z;
		}

		static VkVertexInputBindingDescription getBindingDescription(
			uint32_t bindingDescriptionIndex
		) {
			// describes rate to load data from memory for vertices and
			// number of bytes between data entries and so on
			VkVertexInputBindingDescription bindingDescription{};
			// index of binding in array of bindings
			bindingDescription.binding = bindingDescriptionIndex;
			// number of bytes between beginning of two vertices
			bindingDescription.stride = sizeof(Vk_Vertex_P);
			// vertex or instance
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static VkVertexInputAttributeDescription getAttributeDescriptions(
			uint32_t bindingDescriptionIndex,
			uint32_t positionLocation
		) {

			// the size of the array must match the amount of vector entries in the vertex
			VkVertexInputAttributeDescription attributeDescription{};

			//! characteristics for pos
			// binding index
			// layout(location = [bindingDescriptionIndex]) in vec3 inVertex;
			attributeDescription.binding = bindingDescriptionIndex;
			// shader location indicator
			attributeDescription.location = positionLocation;
			// format of the vertex data (confusingly with R,G and B), this one directly relates to vec2 or vec3 or vec4 in shaders
			attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescription.offset = offsetof(Vk_Vertex_P, pos);

			return attributeDescription;
		}
	};

	struct Vk_Vertex_C {

		glm::tvec3<VK5::point_type> color;

		static int innerDimensionLen(){ return 3; }

		static bool compare(const Vk_Vertex_C& color1, const Vk_Vertex_C& color2) {
			glm::bvec3 res1 = glm::equal(color1.color, color2.color);
			return res1.r && res1.g && res1.b;
		}

		static VkVertexInputBindingDescription getBindingDescription(
			uint32_t bindingDescriptionIndex
		) {
			// describes rate to load data from memory for vertices and
			// number of bytes between data entries and so on
			VkVertexInputBindingDescription bindingDescription{};
			// index of binding in array of bindings
			// layout(location = [bindingDescriptionIndex]) in vec3 inVertex;
			bindingDescription.binding = bindingDescriptionIndex;
			// number of bytes between beginning of two vertices
			bindingDescription.stride = sizeof(Vk_Vertex_C);
			// vertex or instance
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static VkVertexInputAttributeDescription getAttributeDescriptions(
			uint32_t bindingDescriptionIndex,
			uint32_t colorLocation
		) {

			// the size of the array must match the amount of vector entries in the vertex
			VkVertexInputAttributeDescription attributeDescription{};

			//! characteristics for color
			// binding index
			// layout(location = [bindingDescriptionIndex]) in vec3 inVertex;
			attributeDescription.binding = bindingDescriptionIndex;
			// shader location indicator
			attributeDescription.location = colorLocation;
			// format of the color data
			attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescription.offset = offsetof(Vk_Vertex_C, color);

			return attributeDescription;
		}
	};

	struct Vk_Vertex_N {

		glm::tvec3<VK5::point_type> normal;

		static int innerDimensionLen(){ return 3; }

		static bool compare(const Vk_Vertex_N& normal1, const Vk_Vertex_N& normal2) {
			glm::bvec3 res1 = glm::equal(normal1.normal, normal2.normal);
			return res1.x && res1.y && res1.z;
		}

		static VkVertexInputBindingDescription getBindingDescription(
			uint32_t bindingDescriptionIndex
		) {
			// describes rate to load data from memory for vertices and
			// number of bytes between data entries and so on
			VkVertexInputBindingDescription bindingDescription{};
			// index of binding in array of bindings
			// layout(location = [bindingDescriptionIndex]) in vec3 inVertex;
			bindingDescription.binding = bindingDescriptionIndex;
			// number of bytes between beginning of two vertices
			bindingDescription.stride = sizeof(Vk_Vertex_N);
			// vertex or instance
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static VkVertexInputAttributeDescription getAttributeDescriptions(
			uint32_t bindingDescriptionIndex,
			uint32_t normalLocation
		) {

			// the size of the array must match the amount of vector entries in the vertex
			VkVertexInputAttributeDescription attributeDescription{};

			//! characteristics for normal
			// binding index
			// layout(location = [bindingDescriptionIndex]) in vec3 inVertex;
			attributeDescription.binding = bindingDescriptionIndex;
			// shader location indicator
			attributeDescription.location = normalLocation;
			// format of the color data
			attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescription.offset = offsetof(Vk_Vertex_N, normal);

			return attributeDescription;
		}
	};

	struct Vk_Vertex_T {

		glm::tvec2<VK5::point_type> uv;

		static int innerDimensionLen(){ return 2; }

		static bool compare(const Vk_Vertex_T& uv1, const Vk_Vertex_T& uv2) {
			glm::bvec2 res1 = glm::equal(uv1.uv, uv2.uv);
			return res1.x && res1.y;
		}

		static VkVertexInputBindingDescription getBindingDescription(
			uint32_t bindingDescriptionIndex
		) {
			// describes rate to load data from memory for vertices and
			// number of bytes between data entries and so on
			VkVertexInputBindingDescription bindingDescription{};
			// index of binding in array of bindings
			// layout(location = [bindingDescriptionIndex]) in vec3 inVertex;
			bindingDescription.binding = bindingDescriptionIndex;
			// number of bytes between beginning of two vertices
			bindingDescription.stride = sizeof(Vk_Vertex_T);
			// vertex or instance
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static VkVertexInputAttributeDescription getAttributeDescriptions(
			uint32_t bindingDescriptionIndex,
			uint32_t normalLocation
		) {

			// the size of the array must match the amount of vector entries in the vertex
			VkVertexInputAttributeDescription attributeDescription{};

			//! characteristics for normal
			// binding index
			// layout(location = [bindingDescriptionIndex]) in vec3 inVertex;
			attributeDescription.binding = bindingDescriptionIndex;
			// shader location indicator
			attributeDescription.location = normalLocation;
			// format of the color data
			attributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescription.offset = offsetof(Vk_Vertex_T, uv);

			return attributeDescription;
		}
	};
}