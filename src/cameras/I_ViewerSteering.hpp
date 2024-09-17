#pragma once

#include <assert.h>

#include "../Defines.h"

// namespace py = pybind11;

namespace VK5 {

	class I_ViewerSteering {
	public:
		virtual Vk_SteeringType steeringType() const = 0;
		virtual void onMouseAction(Vk_CameraState& cameraState, int px, int py, int dx, int dy, float dz, const std::set<int>& pressedKeys, LWWS::MouseButton mouseButton, LWWS::ButtonOp op, LWWS::MouseAction mouseAction, void* aptr) = 0;
		virtual void onKeyAction(Vk_CameraState& cameraState, int k, LWWS::ButtonOp op, const std::set<int>& otherPressedKeys, void* aptr) = 0;

	private:
		virtual void rotation(Vk_CameraState& cameraState, double dx, double dy) = 0;
		virtual void zoom(Vk_CameraState& cameraState, double dz) = 0;
		virtual void pan(Vk_CameraState& cameraState, double xpos, double ypos) = 0;
	};
}