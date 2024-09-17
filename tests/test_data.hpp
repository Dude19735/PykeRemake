#pragma once

#include <vector>
#ifdef TEST_SCENARIO_1
	#include "../src/objects/Vk_Structures.hpp"
#else
	#include "vkviewer.hpp"
#endif

namespace UT{
	namespace VK5TestData {

// ############################################################################################################
//       ██████     █    █     █ ██████  ███████ █     █         █          █    ██████   █████  ███████       
//       █     █   █ █   ██    █ █     █ █     █ ██   ██         █         █ █   █     █ █     █ █             
//       █     █  █   █  █ █   █ █     █ █     █ █ █ █ █         █        █   █  █     █ █       █             
//       ██████  █     █ █  █  █ █     █ █     █ █  █  █         █       █     █ ██████  █  ████ █████         
//       █   █   ███████ █   █ █ █     █ █     █ █     █         █       ███████ █   █   █     █ █             
//       █    █  █     █ █    ██ █     █ █     █ █     █         █       █     █ █    █  █     █ █             
//       █     █ █     █ █     █ ██████  ███████ █     █         ███████ █     █ █     █  █████  ███████       
// ############################################################################################################
		template<class T_DataType>
		std::vector<T_DataType> ConstLarge_Data(size_t size, VK5::point_type value) {
		}

		template<>
		std::vector<VK5::Vk_Vertex_PC> ConstLarge_Data<VK5::Vk_Vertex_PC>(size_t size, VK5::point_type value) {
			std::vector<VK5::Vk_Vertex_PC> data;
			for (size_t i = 0; i < size; ++i) {
				data.push_back(VK5::Vk_Vertex_PC{ 
					glm::tvec3<VK5::point_type>(value, value, value),
					glm::tvec3<VK5::point_type>(value, value, value)
				});
			}
			return data;
		}

		template<>
		std::vector<VK5::Vk_Vertex_PCN> ConstLarge_Data<VK5::Vk_Vertex_PCN>(size_t size, VK5::point_type value) {
			std::vector<VK5::Vk_Vertex_PCN> data;
			for (size_t i = 0; i < size; ++i) {
				data.push_back(VK5::Vk_Vertex_PCN{ 
					glm::tvec3<VK5::point_type>(value, value, value),
					glm::tvec3<VK5::point_type>(value, value, value),
					glm::tvec3<VK5::point_type>(value, value, value)
				});
			}
			return data;
		}

		template<>
		std::vector<VK5::Vk_Vertex_PCNT> ConstLarge_Data<VK5::Vk_Vertex_PCNT>(size_t size, VK5::point_type value) {
			std::vector<VK5::Vk_Vertex_PCNT> data;
			for (size_t i = 0; i < size; ++i) {
				data.push_back(VK5::Vk_Vertex_PCNT{ 
					glm::tvec3<VK5::point_type>(value, value, value), 
					glm::tvec3<VK5::point_type>(value, value, value),
					glm::tvec3<VK5::point_type>(value, value, value),
					glm::tvec2<VK5::point_type>(value, value)
				});
			}
			return data;
		}

		template<>
		std::vector<VK5::index_type> ConstLarge_Data<VK5::index_type>(size_t size, VK5::point_type value) {
			std::vector<VK5::index_type> data;
			for (size_t i = 0; i < size; ++i) {
				data.push_back(static_cast<VK5::index_type>(value));
			}
			return data;
		}


// ############################################################################################################
//                                     ██████  ███████ ███ █     █ ███████                                     
//                                     █     █ █     █  █  ██    █    █                                        
//                                     █     █ █     █  █  █ █   █    █                                        
//                                     ██████  █     █  █  █  █  █    █                                        
//                                     █       █     █  █  █   █ █    █                                        
//                                     █       █     █  █  █    ██    █                                        
//                                     █       ███████ ███ █     █    █                                        
// ############################################################################################################
		std::vector<VK5::Vk_Vertex_P> Point_P(float angle=0) {
			std::vector<VK5::Vk_Vertex_P> geometry =
			{
				
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 0, 0, 0), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1, 1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1, 1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1,-1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1,-1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1, 1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1, 1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1,-1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1,-1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) }
			};
			return geometry;
		}

		std::vector<VK5::Vk_Vertex_C> Point_C()
		{
			std::vector<VK5::Vk_Vertex_C> colors =
			{
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0,   1.0,   1.0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0,   0,   0    ) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,   1.0,   0    ) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,     0, 1.0    ) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0, 1.0,   0    ) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,   1.0, 1.0    ) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0,   0, 1.0    ) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0, 0.5,   0    ) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0.5,   0, 0.5    ) }
			};
			return colors;
		}

		std::vector<VK5::index_type> Point_P_C_Indices() {
			std::vector<VK5::index_type> indices = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
			return indices;
		}


// ############################################################################################################
//                                         █       ███ █     █ ███████                                         
//                                         █        █  ██    █ █                                               
//                                         █        █  █ █   █ █                                               
//                                         █        █  █  █  █ █████                                           
//                                         █        █  █   █ █ █                                               
//                                         █        █  █    ██ █                                               
//                                         ███████ ███ █     █ ███████                                         
// ############################################################################################################
		std::vector<VK5::Vk_Vertex_P> Line_P(float angle=0.0f) {
			return std::vector<VK5::Vk_Vertex_P> {
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1, 1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1, 1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1,-1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1,-1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1, 1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1, 1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1,-1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1,-1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) }
			};
		}

		std::vector<VK5::Vk_Vertex_C> Line_C() {
			return std::vector<VK5::Vk_Vertex_C> {
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0,   0,   0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,   1.0,   0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,     0, 1.0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0, 1.0,   0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,   1.0, 1.0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0,   0, 1.0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0, 0.5,   0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0.5,   0, 0.5) }
			};
		}

		std::vector<VK5::index_type> Line_P_C_Indices() { 
			return std::vector<VK5::index_type> {
				0,1,  1,2,  2,3,  3,0,
				4,5,  5,6,  6,7,  7,4,
				0,4,  1,5,  2,6,  3,7
			};
		}


// ############################################################################################################
//                █████  ███████ ███████ ██████  ██████           █████  █     █  █████  ███████               
//               █     █ █     █ █     █ █     █ █     █         █     █  █   █  █     █    █                  
//               █       █     █ █     █ █     █ █     █         █         █ █   █          █                  
//               █       █     █ █     █ ██████  █     █          █████     █     █████     █                  
//               █       █     █ █     █ █   █   █     █               █    █          █    █                  
//               █     █ █     █ █     █ █    █  █     █         █     █    █    █     █    █                  
//                █████  ███████ ███████ █     █ ██████           █████     █     █████     █                  
// ############################################################################################################
		std::vector<VK5::Vk_Vertex_P> Coords_P(float fromX, float toX, float tipLenX, float fromY, float toY, float tipLenY, float fromZ, float toZ, float tipLenZ)
		{
			std::vector<VK5::Vk_Vertex_P> geometry =
			{
				VK5::Vk_Vertex_P { glm::tvec3<VK5::point_type>( fromX,     0,     0) },
				VK5::Vk_Vertex_P { glm::tvec3<VK5::point_type>(   toX-tipLenX,     0,     0) },
				VK5::Vk_Vertex_P { glm::tvec3<VK5::point_type>(   toX,     0,     0) },
				VK5::Vk_Vertex_P { glm::tvec3<VK5::point_type>(     0, fromY,     0) },
				VK5::Vk_Vertex_P { glm::tvec3<VK5::point_type>(     0,   toY-tipLenY,     0) },
				VK5::Vk_Vertex_P { glm::tvec3<VK5::point_type>(     0,   toY,     0) },
				VK5::Vk_Vertex_P { glm::tvec3<VK5::point_type>(     0,     0, fromZ) },
				VK5::Vk_Vertex_P { glm::tvec3<VK5::point_type>(     0,     0,   toZ-tipLenZ) },
				VK5::Vk_Vertex_P { glm::tvec3<VK5::point_type>(     0,     0,   toZ) }
			};
			return geometry;
		}

		std::vector<VK5::Vk_Vertex_C> Coords_C(float tipR, float tipG, float tipB)
		{
			std::vector<VK5::Vk_Vertex_C> colors =
			{
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0,   0,   0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0,   0,   0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(tipR,tipG,tipB)},
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,   1.0,   0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,   1.0,   0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(tipR,tipG,tipB)},
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,     0, 1.0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,     0, 1.0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(tipR,tipG,tipB)}
			};
			return colors;
		}

		std::vector<VK5::index_type> Coords_P_C_Indices() { 
			return std::vector<VK5::index_type> {
				0,1,  1,2,
				3,4,  4,5,
				6,7,  7,8
			};
		}


// ############################################################################################################
//    █████  █     █ ██████  ███████         █     █ ███████         █     █ ███████ ██████  █     █  █████    
//   █     █ █     █ █     █ █               █  █  █ █     █         ██    █ █     █ █     █ ██   ██ █     █   
//   █       █     █ █     █ █               █  █  █ █     █         █ █   █ █     █ █     █ █ █ █ █ █         
//   █       █     █ ██████  █████           █  █  █ █     █         █  █  █ █     █ ██████  █  █  █  █████    
//   █       █     █ █     █ █               █  █  █ █     █         █   █ █ █     █ █   █   █     █       █   
//   █     █ █     █ █     █ █               █  █  █ █     █         █    ██ █     █ █    █  █     █ █     █   
//    █████   █████  ██████  ███████          ██ ██  ███████         █     █ ███████ █     █ █     █  █████    
// ############################################################################################################
		std::vector<VK5::Vk_Vertex_P> Cube1_P(float angle=0.0f)
		{
			std::vector<VK5::Vk_Vertex_P> geometry =
			{
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1, 1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1, 1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1,-1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1,-1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1, 1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1, 1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1,-1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1,-1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) }
			};
			return geometry;
		}

		std::vector<VK5::Vk_Vertex_C> Cube1_C()
		{
			std::vector<VK5::Vk_Vertex_C> colors =
			{
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0,   0,   0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,   1.0,   0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,     0, 1.0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0, 1.0,   0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,   1.0, 1.0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0,   0, 1.0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0, 0.5,   0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0.5,   0, 0.5) }
			};
			return colors;
		}

		std::vector<VK5::Vk_Vertex_N> Cube1_N()
		{
			std::vector<VK5::Vk_Vertex_N> normals =
			{
				VK5::Vk_Vertex_N { glm::tvec3<VK5::point_type>( 1, 1, 1) },
				VK5::Vk_Vertex_N { glm::tvec3<VK5::point_type>(-1, 1, 1) },
				VK5::Vk_Vertex_N { glm::tvec3<VK5::point_type>(-1,-1, 1) },
				VK5::Vk_Vertex_N { glm::tvec3<VK5::point_type>( 1,-1, 1) },
				VK5::Vk_Vertex_N { glm::tvec3<VK5::point_type>( 1, 1,-1) },
				VK5::Vk_Vertex_N { glm::tvec3<VK5::point_type>(-1, 1,-1) },
				VK5::Vk_Vertex_N { glm::tvec3<VK5::point_type>(-1,-1,-1) },
				VK5::Vk_Vertex_N { glm::tvec3<VK5::point_type>( 1,-1,-1) }
			};
			return normals;
		}

		std::vector<VK5::Vk_Vertex_PC> Cube1_PC()
		{
			std::vector<VK5::Vk_Vertex_PC> geometry =
			{
				VK5::Vk_Vertex_PC { glm::tvec3<VK5::point_type>( 1, 1, 1),  glm::tvec3<VK5::point_type>(1.0,   0,   0) },
				VK5::Vk_Vertex_PC { glm::tvec3<VK5::point_type>(-1, 1, 1),  glm::tvec3<VK5::point_type>(0,   1.0,   0) },
				VK5::Vk_Vertex_PC { glm::tvec3<VK5::point_type>(-1,-1, 1),  glm::tvec3<VK5::point_type>(0,     0, 1.0) },
				VK5::Vk_Vertex_PC { glm::tvec3<VK5::point_type>( 1,-1, 1),  glm::tvec3<VK5::point_type>(1.0, 1.0,   0) },
				VK5::Vk_Vertex_PC { glm::tvec3<VK5::point_type>( 1, 1,-1),  glm::tvec3<VK5::point_type>(0,   1.0, 1.0) },
				VK5::Vk_Vertex_PC { glm::tvec3<VK5::point_type>(-1, 1,-1),  glm::tvec3<VK5::point_type>(1.0,   0, 1.0) },
				VK5::Vk_Vertex_PC { glm::tvec3<VK5::point_type>(-1,-1,-1),  glm::tvec3<VK5::point_type>(1.0, 0.5,   0) },
				VK5::Vk_Vertex_PC { glm::tvec3<VK5::point_type>( 1,-1,-1),  glm::tvec3<VK5::point_type>(0.5,   0, 0.5) }
			};
			return geometry;
		}

		std::vector<VK5::Vk_Vertex_PC> Cube2_PC()
		{
			std::vector<VK5::Vk_Vertex_PC> geometry =
			{
				VK5::Vk_Vertex_PC { glm::tvec3<VK5::point_type>( 2, 2, 2),  glm::tvec3<VK5::point_type>(0.7,   0,   0) },
				VK5::Vk_Vertex_PC { glm::tvec3<VK5::point_type>(-2, 2, 2),  glm::tvec3<VK5::point_type>(0,   0.7,   0) },
				VK5::Vk_Vertex_PC { glm::tvec3<VK5::point_type>(-2,-2, 2),  glm::tvec3<VK5::point_type>(0,     0, 0.7) },
				VK5::Vk_Vertex_PC { glm::tvec3<VK5::point_type>( 2,-2, 2),  glm::tvec3<VK5::point_type>(0.7, 0.7,   0) },
				VK5::Vk_Vertex_PC { glm::tvec3<VK5::point_type>( 2, 2,-2),  glm::tvec3<VK5::point_type>(0,   0.7, 0.7) },
				VK5::Vk_Vertex_PC { glm::tvec3<VK5::point_type>(-2, 2,-2),  glm::tvec3<VK5::point_type>(0.7,   0, 0.7) },
				VK5::Vk_Vertex_PC { glm::tvec3<VK5::point_type>(-2,-2,-2),  glm::tvec3<VK5::point_type>(0.7, 0.2,   0) },
				VK5::Vk_Vertex_PC { glm::tvec3<VK5::point_type>( 2,-2,-2),  glm::tvec3<VK5::point_type>(0.2,   0, 0.2) }
			};
			return geometry;
		}

		std::vector<VK5::Vk_Vertex_PCN> Cube1_PCN()
		{
			std::vector<VK5::Vk_Vertex_PCN> geometry =
			{
				VK5::Vk_Vertex_PCN { glm::tvec3<VK5::point_type>( 1, 1, 1),  glm::tvec3<VK5::point_type>(1.0,   0,   0),  glm::tvec3<VK5::point_type>( 1, 1, 1) },
				VK5::Vk_Vertex_PCN { glm::tvec3<VK5::point_type>(-1, 1, 1),  glm::tvec3<VK5::point_type>(0,   1.0,   0),  glm::tvec3<VK5::point_type>(-1, 1, 1) },
				VK5::Vk_Vertex_PCN { glm::tvec3<VK5::point_type>(-1,-1, 1),  glm::tvec3<VK5::point_type>(0,     0, 1.0),  glm::tvec3<VK5::point_type>(-1,-1, 1) },
				VK5::Vk_Vertex_PCN { glm::tvec3<VK5::point_type>( 1,-1, 1),  glm::tvec3<VK5::point_type>(1.0, 1.0,   0),  glm::tvec3<VK5::point_type>( 1,-1, 1) },
				VK5::Vk_Vertex_PCN { glm::tvec3<VK5::point_type>( 1, 1,-1),  glm::tvec3<VK5::point_type>(0,   1.0, 1.0),  glm::tvec3<VK5::point_type>( 1, 1,-1) },
				VK5::Vk_Vertex_PCN { glm::tvec3<VK5::point_type>(-1, 1,-1),  glm::tvec3<VK5::point_type>(1.0,   0, 1.0),  glm::tvec3<VK5::point_type>(-1, 1,-1) },
				VK5::Vk_Vertex_PCN { glm::tvec3<VK5::point_type>(-1,-1,-1),  glm::tvec3<VK5::point_type>(1.0, 0.5,   0),  glm::tvec3<VK5::point_type>(-1,-1,-1) },
				VK5::Vk_Vertex_PCN { glm::tvec3<VK5::point_type>( 1,-1,-1),  glm::tvec3<VK5::point_type>(0.5,   0, 0.5),  glm::tvec3<VK5::point_type>( 1,-1,-1) }
			};
			return geometry;
		}

		std::vector<VK5::Vk_Vertex_PCN> Cube2_PCN()
		{
			std::vector<VK5::Vk_Vertex_PCN> geometry =
			{
				VK5::Vk_Vertex_PCN { glm::tvec3<VK5::point_type>( 2, 2, 2),  glm::tvec3<VK5::point_type>(0.7,   0,   0),  glm::tvec3<VK5::point_type>( 1, 1, 1) },
				VK5::Vk_Vertex_PCN { glm::tvec3<VK5::point_type>(-2, 2, 2),  glm::tvec3<VK5::point_type>(0,   0.7,   0),  glm::tvec3<VK5::point_type>(-1, 1, 1) },
				VK5::Vk_Vertex_PCN { glm::tvec3<VK5::point_type>(-2,-2, 2),  glm::tvec3<VK5::point_type>(0,     0, 0.7),  glm::tvec3<VK5::point_type>(-1,-1, 1) },
				VK5::Vk_Vertex_PCN { glm::tvec3<VK5::point_type>( 2,-2, 2),  glm::tvec3<VK5::point_type>(0.7, 0.7,   0),  glm::tvec3<VK5::point_type>( 1,-1, 1) },
				VK5::Vk_Vertex_PCN { glm::tvec3<VK5::point_type>( 2, 2,-2),  glm::tvec3<VK5::point_type>(0,   0.7, 0.7),  glm::tvec3<VK5::point_type>( 1, 1,-1) },
				VK5::Vk_Vertex_PCN { glm::tvec3<VK5::point_type>(-2, 2,-2),  glm::tvec3<VK5::point_type>(0.7,   0, 0.7),  glm::tvec3<VK5::point_type>(-1, 1,-1) },
				VK5::Vk_Vertex_PCN { glm::tvec3<VK5::point_type>(-2,-2,-2),  glm::tvec3<VK5::point_type>(0.7, 0.2,   0),  glm::tvec3<VK5::point_type>(-1,-1,-1) },
				VK5::Vk_Vertex_PCN { glm::tvec3<VK5::point_type>( 2,-2,-2),  glm::tvec3<VK5::point_type>(0.2,   0, 0.2),  glm::tvec3<VK5::point_type>( 1,-1,-1) }
			};
			return geometry;
		}

		std::vector<VK5::Vk_Vertex_PCNT> Cube1_PCNT()
		{
			std::vector<VK5::Vk_Vertex_PCNT> normals = {
			/*00*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>( 1, 1, 1),  glm::tvec3<VK5::point_type>(1.0,   0,   0),  glm::tvec3<VK5::point_type>( 1, 1, 1), glm::tvec2<VK5::point_type>(0.25f, 0.50f) },
			/*01*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>(-1, 1, 1),  glm::tvec3<VK5::point_type>(0,   1.0,   0),  glm::tvec3<VK5::point_type>(-1, 1, 1), glm::tvec2<VK5::point_type>(0.25f, 0.25f) },
			/*02*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>(-1,-1, 1),  glm::tvec3<VK5::point_type>(0,     0, 1.0),  glm::tvec3<VK5::point_type>(-1,-1, 1), glm::tvec2<VK5::point_type>(0.50f, 0.25f) },
			/*03*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>( 1,-1, 1),  glm::tvec3<VK5::point_type>(1.0, 1.0,   0),  glm::tvec3<VK5::point_type>( 1,-1, 1), glm::tvec2<VK5::point_type>(0.50f, 0.50f) },
			/*04*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>( 1, 1,-1),  glm::tvec3<VK5::point_type>(0,   1.0, 1.0),  glm::tvec3<VK5::point_type>( 1, 1,-1), glm::tvec2<VK5::point_type>(0.00f, 0.50f) },
			/*05*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>(-1, 1,-1),  glm::tvec3<VK5::point_type>(1.0,   0, 1.0),  glm::tvec3<VK5::point_type>(-1, 1,-1), glm::tvec2<VK5::point_type>(0.00f, 0.25f) },
			/*06*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>(-1,-1,-1),  glm::tvec3<VK5::point_type>(1.0, 0.5,   0),  glm::tvec3<VK5::point_type>(-1,-1,-1), glm::tvec2<VK5::point_type>(0.75f, 0.25f) },
			/*07*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>( 1,-1,-1),  glm::tvec3<VK5::point_type>(0.5,   0, 0.5),  glm::tvec3<VK5::point_type>( 1,-1,-1), glm::tvec2<VK5::point_type>(0.75f, 0.50f) },

			/*08*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>(-1, 1,-1),  glm::tvec3<VK5::point_type>(1.0,   0, 1.0),  glm::tvec3<VK5::point_type>(-1, 1,-1), glm::tvec2<VK5::point_type>(1.00f, 0.25f) },
			/*09*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>( 1, 1,-1),  glm::tvec3<VK5::point_type>(0,   1.0, 1.0),  glm::tvec3<VK5::point_type>( 1, 1,-1), glm::tvec2<VK5::point_type>(1.00f, 0.50f) },
			/*10*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>(-1, 1,-1),  glm::tvec3<VK5::point_type>(1.0,   0, 1.0),  glm::tvec3<VK5::point_type>(-1, 1,-1), glm::tvec2<VK5::point_type>(0.75f, 0.00f) },
			/*11*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>(-1, 1, 1),  glm::tvec3<VK5::point_type>(0,   1.0,   0),  glm::tvec3<VK5::point_type>(-1, 1, 1), glm::tvec2<VK5::point_type>(0.50f, 0.00f) },
			/*12*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>( 1, 1, 1),  glm::tvec3<VK5::point_type>(1.0,   0,   0),  glm::tvec3<VK5::point_type>( 1, 1, 1), glm::tvec2<VK5::point_type>(0.50f, 0.75f) },
			/*13*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>( 1, 1,-1),  glm::tvec3<VK5::point_type>(0,   1.0, 1.0),  glm::tvec3<VK5::point_type>( 1, 1,-1), glm::tvec2<VK5::point_type>(0.75f, 0.75f) },
			};
			return normals;
		}

		std::vector<VK5::Vk_Vertex_PCNT> Cube2_PCNT()
		{
			std::vector<VK5::Vk_Vertex_PCNT> normals = {
			/*00*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>( 2, 2, 2),  glm::tvec3<VK5::point_type>(0.7,   0,   0),  glm::tvec3<VK5::point_type>( 1, 1, 1), glm::tvec2<VK5::point_type>(0.25f, 0.50f) },
			/*01*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>(-2, 2, 2),  glm::tvec3<VK5::point_type>(0,   0.7,   0),  glm::tvec3<VK5::point_type>(-1, 1, 1), glm::tvec2<VK5::point_type>(0.25f, 0.25f) },
			/*02*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>(-2,-2, 2),  glm::tvec3<VK5::point_type>(0,     0, 0.7),  glm::tvec3<VK5::point_type>(-1,-1, 1), glm::tvec2<VK5::point_type>(0.50f, 0.25f) },
			/*03*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>( 2,-2, 2),  glm::tvec3<VK5::point_type>(0.7, 0.7,   0),  glm::tvec3<VK5::point_type>( 1,-1, 1), glm::tvec2<VK5::point_type>(0.50f, 0.50f) },
			/*04*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>( 2, 2,-2),  glm::tvec3<VK5::point_type>(0,   0.7, 0.7),  glm::tvec3<VK5::point_type>( 1, 1,-1), glm::tvec2<VK5::point_type>(0.00f, 0.50f) },
			/*05*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>(-2, 2,-2),  glm::tvec3<VK5::point_type>(0.7,   0, 0.7),  glm::tvec3<VK5::point_type>(-1, 1,-1), glm::tvec2<VK5::point_type>(0.00f, 0.25f) },
			/*06*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>(-2,-2,-2),  glm::tvec3<VK5::point_type>(0.7, 0.2,   0),  glm::tvec3<VK5::point_type>(-1,-1,-1), glm::tvec2<VK5::point_type>(0.75f, 0.25f) },
			/*07*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>( 2,-2,-2),  glm::tvec3<VK5::point_type>(0.2,   0, 0.2),  glm::tvec3<VK5::point_type>( 1,-1,-1), glm::tvec2<VK5::point_type>(0.75f, 0.50f) },

			/*08*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>(-2, 2,-2),  glm::tvec3<VK5::point_type>(0.7,   0, 0.7),  glm::tvec3<VK5::point_type>(-1, 1,-1), glm::tvec2<VK5::point_type>(1.00f, 0.25f) },
			/*09*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>( 2, 2,-2),  glm::tvec3<VK5::point_type>(0,   0.7, 0.7),  glm::tvec3<VK5::point_type>( 1, 1,-1), glm::tvec2<VK5::point_type>(1.00f, 0.50f) },
			/*10*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>(-2, 2,-2),  glm::tvec3<VK5::point_type>(0.7,   0, 0.7),  glm::tvec3<VK5::point_type>(-1, 1,-1), glm::tvec2<VK5::point_type>(0.75f, 0.00f) },
			/*11*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>(-2, 2, 2),  glm::tvec3<VK5::point_type>(0,   0.7,   0),  glm::tvec3<VK5::point_type>(-1, 1, 1), glm::tvec2<VK5::point_type>(0.50f, 0.00f) },
			/*12*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>( 2, 2, 2),  glm::tvec3<VK5::point_type>(0.7,   0,   0),  glm::tvec3<VK5::point_type>( 1, 1, 1), glm::tvec2<VK5::point_type>(0.50f, 0.75f) },
			/*13*/	VK5::Vk_Vertex_PCNT { glm::tvec3<VK5::point_type>( 2, 2,-2),  glm::tvec3<VK5::point_type>(0,   0.7, 0.7),  glm::tvec3<VK5::point_type>( 1, 1,-1), glm::tvec2<VK5::point_type>(0.75f, 0.75f) },
			};
			return normals;
		}

		std::vector<VK5::index_type> Cube1_PCNT_Indices() {
			std::vector<VK5::index_type> indices = { 
				/*top   */ 0,1,2,  2,3,0,
				/*back  */ 0,5,1,  0,4,5,  
				/*right */ 3,7,0,  7,4,0,  
				/*left  */ 1,5,2,  5,6,2,  
				/*front */ 7,3,2,  2,6,7,  
				/*bottom*/ 7,6,5,  5,4,7 
			};
			return indices;
		}

		std::vector<VK5::index_type> Cube1_P_C_Indices() {
			return Cube1_PCNT_Indices();
		}

		std::vector<VK5::index_type> Cube1_PC_Indices() {
			return Cube1_PCNT_Indices();
		}

		std::vector<VK5::index_type> Cube1_PCN_Indices() {
			return Cube1_PCNT_Indices();
		}

		std::vector<VK5::index_type> Cube2_PCNT_Indices() {
			std::vector<VK5::index_type> indices = { 
				/*top   */ 0,1,2,  2,3,0,
				/*back  */ 0,5,1,  0,4,5,  
				/*right */ 3,7,0,  7,4,0,  
				/*left  */ 1,5,2,  5,6,2,  
				/*front */ 7,3,2,  2,6,7,  
				/*bottom*/ 7,6,5,  5,4,7 
			};
			return indices;
		}

		std::vector<VK5::index_type> Cube2_P_C_Indices() {
			return Cube2_PCNT_Indices();
		}

		std::vector<VK5::index_type> Cube2_PC_Indices() {
			return Cube2_PCNT_Indices();
		}

		std::vector<VK5::index_type> Cube2_PCN_Indices() {
			return Cube2_PCNT_Indices();
		}

// ############################################################################################################
//        █████  █     █ ██████  ███████         █     █         █     █ ███████ ██████  █     █  █████        
//       █     █ █     █ █     █ █               █  █  █         ██    █ █     █ █     █ ██   ██ █     █       
//       █       █     █ █     █ █               █  █  █         █ █   █ █     █ █     █ █ █ █ █ █             
//       █       █     █ ██████  █████           █  █  █         █  █  █ █     █ ██████  █  █  █  █████        
//       █       █     █ █     █ █               █  █  █         █   █ █ █     █ █   █   █     █       █       
//       █     █ █     █ █     █ █               █  █  █         █    ██ █     █ █    █  █     █ █     █       
//        █████   █████  ██████  ███████          ██ ██          █     █ ███████ █     █ █     █  █████        
// ############################################################################################################
		std::vector<VK5::Vk_Vertex_P> Cube2_P(float angle=0.0f)
		{
			std::vector<VK5::Vk_Vertex_P> geometry =
			{
				/* 00t */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1, 1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 01b */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1, 1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 02r */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1, 1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },

				/* 03t */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1, 1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 04b */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1, 1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 05l */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1, 1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },

				/* 06t */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1,-1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 07l */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1,-1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 08f */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1,-1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },

				/* 09t */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1,-1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 10f */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1,-1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 11r */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1,-1, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },

				/* 12u */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1, 1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 13b */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1, 1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 14r */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1, 1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },

				/* 15u */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1, 1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 16b */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1, 1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 17l */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1, 1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },

				/* 18u */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1,-1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 19l */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1,-1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 20f */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1,-1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },

				/* 21t */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1,-1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 22f */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1,-1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 23r */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1,-1,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) }
			};
			return geometry;
		}

		std::vector<VK5::Vk_Vertex_P> Cube2_N(float angle=0.0f)
		{
			/*t=top, b=back, r=right, l=left, f=front, u=under(==bottom)*/
			std::vector<VK5::Vk_Vertex_P> geometry =
			{
				/* 00t */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 0, 0, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 01b */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 0, 1, 0), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 02r */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1, 0, 0), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },

				/* 03t */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 0, 0, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 04b */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 0, 1, 0), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 05l */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1, 0, 0), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },

				/* 06t */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 0, 0, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 07l */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1, 0, 0), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 08f */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 0,-1, 0), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },

				/* 09t */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 0, 0, 1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 10r */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 0,-1, 0), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 11f */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1, 0, 0), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },

				/* 12u */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 0, 0,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 13b */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 0, 1, 0), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 14r */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1, 0, 0), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },

				/* 15u */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 0, 0,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 16b */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 0, 1, 0), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 17l */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1, 0, 0), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },

				/* 18u */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 0, 0,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 19l */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>(-1, 0, 0), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 20f */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 0,-1, 0), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },

				/* 21u */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 0, 0,-1), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 22r */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 0,-1, 0), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) },
				/* 23f */ VK5::Vk_Vertex_P { glm::rotate(glm::tvec3<VK5::point_type>( 1, 0, 0), angle, glm::tvec3<VK5::point_type>( 0, 0, 1)) }
			};
			return geometry;
		}		

		std::vector<VK5::Vk_Vertex_C> Cube2_C()
		{
			std::vector<VK5::Vk_Vertex_C> colors =
			{
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0,   0,   0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0,   0,   0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0,   0,   0) },

				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,   1.0,   0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,   1.0,   0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,   1.0,   0) },

				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,     0, 1.0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,     0, 1.0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,     0, 1.0) },

				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0, 1.0,   0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0, 1.0,   0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0, 1.0,   0) },

				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,   1.0, 1.0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,   1.0, 1.0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0,   1.0, 1.0) },

				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0,   0, 1.0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0,   0, 1.0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0,   0, 1.0) },

				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0, 0.5,   0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0, 0.5,   0) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(1.0, 0.5,   0) },

				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0.5,   0, 0.5) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0.5,   0, 0.5) },
				VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0.5,   0, 0.5) }
			};
			return colors;
		}

		std::vector<VK5::index_type> Cube2_P_C_N_Indices() {
			std::vector<VK5::index_type> indices = { 
				/*top   */  0, 3, 6,   6, 9, 0,
				/*back  */  1,16, 4,   1,13,16,  
				/*right */ 10,22, 2,  22,14, 2,  
				/*left  */  5,17, 7,  17,19, 7,  
				/*front */ 23,11, 8,   8,20,23,  
				/*bottom*/ 21,18,15,  15,12,21 
			};
			return indices;
		}

		std::vector<VK5::Vk_Vertex_P> Cube2_NormalLines_P(float len, float angle=0.0f)
		{
			std::vector<VK5::Vk_Vertex_P> res;
			auto p = Cube2_P(angle);
			auto n = Cube2_N(angle);
			size_t s = p.size();
			for(size_t i=0; i<s; ++i){
				res.push_back(p.at(i));
				auto pPos = p.at(i).pos;
				auto nPos = n.at(i).pos;
				auto f = pPos + len*nPos;
				res.push_back(VK5::Vk_Vertex_P{ .pos=f });
			};

			return res;
		}

		std::vector<VK5::index_type> Cube2_NormalLines_Indices() {
			std::vector<VK5::index_type> indices;
			for(size_t i=0; i<48; ++i){
				indices.push_back(i);
			}
			return indices;
		}

		std::vector<VK5::Vk_Vertex_C> Cube2_NormalLines_C()
		{
			return std::vector<VK5::Vk_Vertex_C>(48, VK5::Vk_Vertex_C { glm::tvec3<VK5::point_type>(0.0, 1.0, 0.0) });
		}
	}
}
