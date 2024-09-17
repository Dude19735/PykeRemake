#pragma once

#include "../Defines.h"

namespace VK5{
    class Vk_CameraLib{
    public:
        struct Vk_CameraCoords {
            std::array<point_type, 3> wPos;
            std::array<point_type, 3> wLook;
            std::array<point_type, 3> wUp;
            std::array<point_type, 3> xAxis;
            std::array<point_type, 3> yAxis;
            std::array<point_type, 3> zAxis;
        };

        struct UnknownCameraTypeException : public std::exception
        {
            const char* what() const throw ()
            {
                return "Unknown camera type";
            }
        };

        static void calculateTransform(int viewportWidth, int viewportHeight, Vk_PinholeState& cs){
            cs.aspect = static_cast<point_type>(viewportWidth) / static_cast<point_type>(viewportHeight);
			point_type f1 = cs.wFar / (cs.wFar - cs.wNear);
			point_type f2 = -(cs.wNear * cs.wFar) / (cs.wFar - cs.wNear);
			point_type x = 1 / glm::tan(cs.fow / 2);
			point_type y = cs.aspect / glm::tan(cs.fow / 2);

			point_type proj[16] = {
				x, 0, 0, 0,
				0, y, 0, 0,
				0, 0,f1,f2,
				0, 0, 1, 0
			};
			cs.perspective = glm::transpose(glm::make_mat4(proj));

			glm::tvec3<point_type> lookAt = glm::normalize(cs.wLook - cs.wPos);
			glm::tvec3<point_type> newX = glm::cross(lookAt, cs.wUp);

			//! this is only init of camera => crashing is allowed
			assert(glm::l2Norm(newX) > 0 && "wUp and lookAt are collinear");

			glm::tvec3<point_type> newY = glm::cross(lookAt, newX);
			glm::tmat3x3<point_type> xyzT = glm::transpose(
                glm::mat3(
                    glm::normalize(newX), 
                    glm::normalize(newY), 
                    lookAt
                )
            );

			cs.view = glm::mat4(
				glm::tvec4<point_type>(xyzT[0], 0.0),
				glm::tvec4<point_type>(xyzT[1], 0.0),
				glm::tvec4<point_type>(xyzT[2], 0.0),
				glm::tvec4<point_type>(-(xyzT * cs.wPos), 1.0)
			);

            cs.xAxis = newX;
            cs.yAxis = newY;
            cs.zAxis = lookAt;
        }
    };
}