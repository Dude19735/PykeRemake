#pragma once

#include "../Defines.h"

#include "Vk_CameraLib.hpp"

namespace VK5 {
    class Vk_ViewerSteeringLib {
    public:
		static glm::tmat3x3<point_type> get3x3Matrix(const glm::tmat4x4<point_type>& matrix) {
			glm::tmat3x3<point_type> temp(
				glm::tvec3<point_type>(matrix[0][0], matrix[0][1], matrix[0][2]),
				glm::tvec3<point_type>(matrix[1][0], matrix[1][1], matrix[1][2]),
				glm::tvec3<point_type>(matrix[2][0], matrix[2][1], matrix[2][2])
			);
			return temp;
		}

        static void rotation_CameraCentric(Vk_CameraState& cameraState, double xpos, double ypos) {
			point_type dx = static_cast<point_type>(cameraState.viewport.parentMouseState()->dx);
			point_type dy = static_cast<point_type>(cameraState.viewport.parentMouseState()->dy);
			// point_type dx = static_cast<point_type>((xpos - vps.lastMousePosX) / static_cast<point_type>(vps.height()));
			// point_type dy = static_cast<point_type>((ypos - vps.lastMousePosY) / static_cast<point_type>(vps.width()));

			// isolate rotation matrix
			auto& pss = cameraState.pinhole;
			glm::tmat3x3<point_type> tempView = get3x3Matrix(pss.view);
			// set new lookAt vector in camera coordinates
			glm::tvec3<point_type> cnLookAt = glm::vec3(-dx, -dy, 1);
			// transform new lookAt vector into world coordinates (note: tempView is orthogonal)
			glm::tvec3<point_type> wnLookAt = glm::normalize(glm::transpose(tempView) * cnLookAt);
			// get the old lookAt vector (z vector from tempView == old z-axis)
			// glm::tvec3<point_type> lookAt = glm::row(tempView, 2);

			// calculate the new x axis direction
			glm::tvec3<point_type> newX = glm::cross(wnLookAt, pss.wUp);

			//! this is only init of camera => crashing is allowed
			// assert(glm::l2Norm(newX) > 0 && "wUp and lookAt are collinear");

			// calculate the new y axis
			glm::tvec3<point_type> newY = glm::cross(wnLookAt, newX);
			glm::tmat3x3<point_type> xyzT = glm::transpose(glm::mat3(glm::normalize(newX), glm::normalize(newY), wnLookAt));

			pss.view = glm::mat4(
				glm::tvec4<point_type>(xyzT[0], 0.0),
				glm::tvec4<point_type>(xyzT[1], 0.0),
				glm::tvec4<point_type>(xyzT[2], 0.0),
				glm::tvec4<point_type>(-(xyzT * pss.wPos), 1.0)
			);

			// calculate the distance between the camera position and the old wLookAt
			point_type dist = glm::l2Norm(pss.wPos - pss.wLook);
			// set the new lookAt point for the camera at the same distance in direction of the
			// new look direction.
			pss.wLook = dist*wnLookAt + pss.wPos;

			// set the camera new up direction to the negative of the new y axis
			pss.wUp = -glm::normalize(newY);
		}

        static void rotation_ObjectCentric(Vk_CameraState& cameraState, double xpos, double ypos) {
			auto& vps = cameraState.viewport;
			auto& pss = cameraState.pinhole;

			point_type dx = static_cast<point_type>(vps.parentMouseState()->dx);
			point_type dy = static_cast<point_type>(vps.parentMouseState()->dy);
			// point_type dx = static_cast<point_type>((xpos - vps.lastMousePosX) / static_cast<point_type>(vps.height));
			// point_type dy = static_cast<point_type>((ypos - vps.lastMousePosY) / static_cast<point_type>(vps.width));

			// calculate the distance between the camera position and the old wLookAt
			point_type dist = glm::l2Norm(pss.wPos - pss.wLook);

			// isolate rotation matrix
			glm::tmat3x3<point_type> tempView = get3x3Matrix(pss.view);

			// get new looking direction based on dy and dy
			glm::tvec3<point_type> cnLookAt = glm::vec3(dx, dy, 1);
			// transform new looking direction into world coordinates
			glm::tvec3<point_type> wnLookAt = glm::normalize(glm::transpose(tempView) * cnLookAt);

			// calculate new place of camera based on camera lookAt point and new looking direction
			glm::tvec3<point_type> wnCameraPos = pss.wLook - dist*wnLookAt;
			
			// calculate the new x axis direction
			glm::tvec3<point_type> newX = glm::cross(wnLookAt, pss.wUp);
			glm::tvec3<point_type> newY = glm::cross(wnLookAt, newX);
			glm::tmat3x3<point_type> xyzT = glm::transpose(glm::mat3(glm::normalize(newX), glm::normalize(newY), wnLookAt));

			pss.wPos = wnCameraPos;

			// set the new lookAt point for the camera at the same distance in direction of the
			// new look direction.
			pss.wLook = dist*wnLookAt + wnCameraPos;

			// set the camera new up direction to the negative of the new y axis
			pss.wUp = -glm::normalize(newY);

			pss.view = glm::mat4(
				glm::tvec4<point_type>(xyzT[0], 0.0),
				glm::tvec4<point_type>(xyzT[1], 0.0),
				glm::tvec4<point_type>(xyzT[2], 0.0),
				glm::tvec4<point_type>(-(xyzT * wnCameraPos), 1.0)
			);
		}

        static void zoom(Vk_CameraState& cameraState, double dz) {
			auto& vps = cameraState.viewport;
			auto& pss = cameraState.pinhole;

			// isolate rotation matrix
			glm::tmat3x3<point_type> tempView = get3x3Matrix(pss.view);
			glm::tvec3<point_type> lookAt = glm::row(tempView, 2);

			// calculate the new position
			// only use the xoffset since the mouse wheel can only act vertically
			// std::cout << xoffset << " " << yoffset << std::endl;
			const point_type acc = 1.0f; // 0.25e-1f;
			pss.wPos = pss.wPos + acc * dz * lookAt;

			// write back into the view matrix
			glm::tvec3<point_type> newTranslation = -(tempView * pss.wPos);
			pss.view = glm::tmat4x4<point_type>(
				glm::column(pss.view, 0),			/*new x axis*/
				glm::column(pss.view, 1),			/*new y axis*/
				glm::column(pss.view, 2),			/*new z axis*/
				glm::tvec4<point_type>(newTranslation, 1.0)	/*new position of the camera*/
			);
		}

        static void pan(Vk_CameraState& cameraState, double xpos, double ypos) {
			auto& vps = cameraState.viewport;
			auto& pss = cameraState.pinhole;

			glm::tmat3x3<point_type> tempView = get3x3Matrix(pss.view);
			glm::tvec3<point_type> xAxis = -glm::row(tempView, 0);
			glm::tvec3<point_type> yAxis = -glm::row(tempView, 1);

			// do the offset kind of dependent on the amount of mouse move
			point_type dx = static_cast<point_type>(vps.parentMouseState()->dx);
			point_type dy = static_cast<point_type>(vps.parentMouseState()->dy);
			// point_type dx = static_cast<point_type>((xpos - vps.lastMousePosX) / static_cast<point_type>(vps.height));
			// point_type dy = static_cast<point_type>((ypos - vps.lastMousePosY) / static_cast<point_type>(vps.width));

			const point_type acc = static_cast<point_type>(
				5 * std::sqrt(pss.wPos.x * pss.wPos.x + pss.wPos.y * pss.wPos.y + pss.wPos.z * pss.wPos.z) * 10e-2);

			glm::tvec3<point_type> offset = acc * (dx * xAxis + dy * yAxis);
			pss.wPos = pss.wPos + offset;
			pss.wLook = pss.wLook + offset;

			// write back into the view matrix
			glm::vec3 newTranslation = -(tempView * pss.wPos);
			pss.view = glm::tmat4x4<point_type>(
				glm::column(pss.view, 0),			/*new x axis*/
				glm::column(pss.view, 1),			/*new y axis*/
				glm::column(pss.view, 2),			/*new z axis*/
				glm::tvec4<point_type>(newTranslation, 1.0)	/*new position of the camera*/
			);
		}
    };
}