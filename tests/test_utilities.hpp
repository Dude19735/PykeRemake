#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <format>

// #pragma warning(push)
// #pragma warning(disable : 4196)
// #include "glm/glm.hpp" // vectors and matrices
// #include "glm/gtc/matrix_transform.hpp"
// #include "glm/ext.hpp"
// #include "glm/gtx/string_cast.hpp"
// #pragma warning(pop)

#include "../src/Defines.h"
#include "../src/objects/Vk_Structures.hpp"

namespace UT {
	namespace TestUtilities {
		typedef VK5::Vk_Vertex_PC TypeV;
		typedef VK5::Vk_Vertex_PCN TypeC;
		typedef VK5::Vk_Vertex_PCNT TypeN;
		typedef VK5::point_type TypeP;

		template<class T_DataType>
		void compareStructuresVectors(const std::vector<T_DataType>* vec1, const std::vector<T_DataType>* vec2) {
		}

		template<>
		void compareStructuresVectors<TypeV>(const std::vector<TypeV>* vec1, const std::vector<TypeV>* vec2) {
			assert(vec1->size() == vec2->size());
			for (int i = 0; i < vec1->size(); ++i) {
				assert(TypeV::compare(vec1->at(i), vec2->at(i)));
			}
		}

		template<>
		void compareStructuresVectors<TypeC>(const std::vector<TypeC>* vec1, const std::vector<TypeC>* vec2) {
			assert(vec1->size() == vec2->size());
			for (int i = 0; i < vec1->size(); ++i) {
				assert(TypeC::compare(vec1->at(i), vec2->at(i)));
			}
		}

		template<>
		void compareStructuresVectors<TypeN>(const std::vector<TypeN>* vec1, const std::vector<TypeN>* vec2) {
			assert(vec1->size() == vec2->size());
			for (int i = 0; i < vec1->size(); ++i) {
				assert(TypeN::compare(vec1->at(i), vec2->at(i)));
			}
		}

		template<>
		void compareStructuresVectors<TypeP>(const std::vector<TypeP>* vec1, const std::vector<TypeP>* vec2) {
			assert(vec1->size() == vec2->size());
			for (int i = 0; i < vec1->size(); ++i) {
				assert(vec1->at(i) == vec2->at(i));
			}
		}

		template<class ..._Types>
		std::string formatTimeOutput(const std::string& comment, const _Types&... _args) {
			std::string separator = VK5::GlobalCasters::castHighlightCyan("=================================================================================");
			std::string content = VK5::GlobalCasters::castCyan(std::format(comment, _args...));
			std::stringstream buf("");
			buf << separator << std::endl << content << std::endl << separator << std::endl;
			return buf.str();
		}

		const char* string_concat(const std::string& part1, size_t part2) {
			std::stringstream sstr;
			sstr << part1 << " " << part2;
			return sstr.str().c_str();
		}

		bool assert_compare(const std::vector<glm::vec3>& result, const std::vector<glm::vec3>& expected) {
			size_t size = expected.size();
			glm::vec3 prec = PRECISION * glm::vec3(1, 1, 1);
			std::stringstream sstr;
			bool assert_result = true;
			for (size_t i = 0; i < size; ++i) {
				glm::bvec3 res1 = glm::greaterThanEqual(result[i], expected[i] - prec);
				glm::bvec3 res2 = glm::lessThanEqual(result[i], expected[i] + prec);
				assert_result = assert_result && res1.x && res1.y && res1.z && res2.x && res2.y && res2.z;
			}
			return assert_result;
		}

		bool assert_compare(const std::vector<glm::vec2>& result, const std::vector<glm::vec2>& expected) {
			size_t size = expected.size();
			glm::vec2 prec = PRECISION * glm::vec2(1, 1);
			std::stringstream sstr;
			bool assert_result = true;
			for (size_t i = 0; i < size; ++i) {
				glm::bvec2 res1 = glm::greaterThanEqual(result[i], expected[i] - prec);
				glm::bvec2 res2 = glm::lessThanEqual(result[i], expected[i] + prec);
				assert_result = assert_result && res1.x && res1.y && res2.x && res2.y;
			}
			return assert_result;
		}
	}
}
