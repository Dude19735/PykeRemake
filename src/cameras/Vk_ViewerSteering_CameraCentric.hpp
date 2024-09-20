#pragma once

#include "../Defines.h"

// namespace py = pybind11;

#include "I_ViewerSteering.hpp"
#include "Vk_ViewerSteeringLib.hpp"

namespace VK5 {

	class Vk_ViewerSteering_CameraCentric : public I_ViewerSteering {
	public:

		virtual Vk_SteeringType steeringType() const {
			return Vk_SteeringType::CameraCentric;
		}

		virtual void onMouseAction(Vk_CameraState& cameraState, int px, int py, int dx, int dy, float dz, const std::set<int>& pressedKeys, LWWS::MouseButton mouseButton, LWWS::ButtonOp op, LWWS::MouseAction mouseAction, void* aptr) {
			if(mouseAction == LWWS::MouseAction::MouseMove){
				// const auto& cameras = viewer->vk_cameras();
				if (mouseButton == LWWS::MouseButton::Left && (op == LWWS::ButtonOp::Down || op == LWWS::ButtonOp::SteadyPress)) {
					auto ctrl = LWWS::LWWS_Key::KeyToInt(LWWS::LWWS_Key::Special::LControl);
					if(pressedKeys.find(ctrl) != pressedKeys.end()){
						pan(cameraState, px, py);
					}
					else {
						rotation(cameraState, px, py);
					}
				}
			}
			else if(mouseAction == LWWS::MouseAction::MouseScroll){
				zoom(cameraState, dz);
			}

			// camera->vk_lastMousePosition(xpos, ypos);
		}
		
		virtual void onKeyAction(Vk_CameraState& cameraState, int k, LWWS::ButtonOp op, const std::set<int>& otherPressedKeys, void* aptr) {}

	private:
		void rotation(Vk_CameraState& cameraState, double xpos, double ypos) {
			Vk_ViewerSteeringLib::rotation_CameraCentric(cameraState, xpos, ypos);
		}

		virtual void zoom(Vk_CameraState& cameraState, double dz) {
			Vk_ViewerSteeringLib::zoom(cameraState, dz);
		}

		virtual void pan(Vk_CameraState& cameraState, double xpos, double ypos) {
			Vk_ViewerSteeringLib::pan(cameraState, xpos, ypos);
		}
	};
}