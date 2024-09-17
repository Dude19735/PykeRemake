#pragma once

#include <vector>
#include <array>

#include "../Defines.h"

#include "../renderer/I_Renderer.hpp"
#include "Vk_ViewerSteering_CameraCentric.hpp"
#include "Vk_ViewerSteering_ObjectCentric.hpp"
#include "Vk_CameraLib.hpp"

// namespace py = pybind11;

namespace VK5 {
    struct Vk_CameraMisc {
        TSteeringGroup SteeringGroup;
        Vk_RenderType RenderType;
        Vk_SteeringType SteeringType;
    };

    /**
     * Convenience modifier: modify the SteeringGroup of a Vk_CameraMisc in place
     * Make initializing layouts nicer
     * ===============================
     * This is probably the most common case: use one Vk_CameraMisc and initialize one whole grid.
     * Instead of needing space for a new Vk_CameraMisc for each new add(), modify one single
     * Vk_CameraMisc in a chain:
     * 
     *   VK5::Vk_LayoutGrid grid(2,2, 0, 0);
     *   grid.add(0,0,p,m).add(0,1,p,m)
     *       .add(1,0,p,m).add(1,1,p,VK5::mSG(m,2));
     */
    Vk_CameraMisc& mSG(Vk_CameraMisc& cameraMisc, TSteeringGroup newSteeringGroup){
        cameraMisc.SteeringGroup = newSteeringGroup;
        return cameraMisc;
    }
    /**
     * Convenience modifier: modify the RenderType of a Vk_CameraMisc in place
     * Make initializing layouts nicer
     * ===============================
     *   VK5::Vk_LayoutGrid grid(2,2, 0, 0);
     *   grid.add(0,0,p,m)                                          
     *       .add(0,1,p,VK5::mRT(m,VK5::Vk_RenderType::Rasterizer_IM))
     *       .add(1,0,p,mST(m,VK5::Vk_SteeringType::CAMERA_CENTRIC))
     *       .add(1,1,p,VK5::mSG(m,2));
     */
    Vk_CameraMisc& mRT(Vk_CameraMisc& cameraMisc, Vk_RenderType newRenderType){
        cameraMisc.RenderType = newRenderType;
        return cameraMisc;
    }
    /**
     * Convenience modifier: modify the SteeringType of a Vk_CameraMisc in place
     * Make initializing layouts nicer
     * ===============================
     *   VK5::Vk_LayoutGrid grid(2,2, 0, 0);
     *   grid.add(0,0,p,m)
     *       .add(0,1,p,VK5::mRT(m,VK5::Vk_RenderType::Rasterizer_IM))
     *       .add(1,0,p,mST(m,VK5::Vk_SteeringType::CAMERA_CENTRIC))
     *       .add(1,1,p,VK5::mSG(m,2));
     */
    Vk_CameraMisc& mST(Vk_CameraMisc& cameraMisc, Vk_SteeringType newSteeringType){
        cameraMisc.SteeringType = newSteeringType;
        return cameraMisc;
    }

    struct Vk_CameraInit {
        Vk_CameraMisc Misc;
        LWWS::LWWS_Viewport Viewport;
        Vk_CameraPinhole Pinhole;
    };

    class Vk_Camera {
    public:
        Vk_Camera(const Vk_CameraInit& init)
        : 
        _misc(init.Misc),
        _state(Vk_CameraState{
            .viewport=init.Viewport,
            .pinhole=Vk_PinholeState{
                .wPos=init.Pinhole.wPos,
                .wLook=init.Pinhole.wLook,
                .wUp=init.Pinhole.wUp,
                .fow=init.Pinhole.fow,
                .wNear=init.Pinhole.wNear,
                .wFar=init.Pinhole.wFar,
                .view=glm::mat4(1.0f),
                .perspective=glm::mat4(1.0f),
                .xAxis=glm::vec3(0.0f, 0.0f, 0.0f),
                .yAxis=glm::vec3(0.0f, 0.0f, 0.0f),
                .zAxis=glm::vec3(0.0f, 0.0f, 0.0f),
                .aspect=1.0f
            }
        }),
        _renderer(nullptr),
        _steering(setSteering(init.Misc))
        {
			calculateTransform();
            if(_steering==nullptr) UT::Ut_Logger::RuntimeError(typeid(this), "Unsuported steering type");
		}

		void calculateTransform() {
            Vk_CameraLib::calculateTransform(_state.viewport.width(), _state.viewport.height(), _state.pinhole);
		}

        bool contains(int posw, int posh){
            return _state.viewport.contains(posw, posh);
        }

        Vk_CameraMisc* misc() { return &_misc; }
        Vk_CameraState* state() { return &_state; }
        
        const Vk_CameraLib::Vk_CameraCoords cameraCoords() const {
            return Vk_CameraLib::Vk_CameraCoords {
                .wPos = std::array<point_type, 3> {_state.pinhole.wPos.x, _state.pinhole.wPos.y, _state.pinhole.wPos.z},
                .wLook = std::array<point_type, 3> {_state.pinhole.wLook.x, _state.pinhole.wLook.y, _state.pinhole.wLook.z},
                .wUp = std::array<point_type, 3> {_state.pinhole.wUp.x, _state.pinhole.wUp.y, _state.pinhole.wUp.z},
                .xAxis = std::array<point_type, 3> {_state.pinhole.xAxis.x, _state.pinhole.xAxis.y, _state.pinhole.xAxis.z},
                .yAxis = std::array<point_type, 3> {_state.pinhole.yAxis.x, _state.pinhole.yAxis.y, _state.pinhole.yAxis.z},
                .zAxis = std::array<point_type, 3> {_state.pinhole.zAxis.x, _state.pinhole.zAxis.y, _state.pinhole.zAxis.z}
            };
        }

        /* == Callbacks == */
        void onMouseAction(int px, int py, int dx, int dy, float dz, const std::set<int>& pressedKeys, LWWS::MouseButton mouseButton, LWWS::ButtonOp op, LWWS::MouseAction mouseAction, void* aptr) {
			_steering->onMouseAction(_state, px, py, dx, dy, dz, pressedKeys, mouseButton, op, mouseAction, aptr);
            /**
             * TODO: trigger redraw
             */
		}

		void onKeyAction(int k, LWWS::ButtonOp op, const std::set<int>& otherPressedKeys, void* aptr) {
			_steering->onKeyAction(_state, k, op, otherPressedKeys, aptr);
            /**
             * TODO: trigger redraw
             */
		}

        // void onMouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
		// 	_steering->onMouseMove(window, this, xpos, ypos);
		// }

		// void onMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
		// 	_steering->onMouseScroll(window, this, yoffset, yoffset);
		// }

		// void onKeyPressedCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		// 	_steering->onKeyPressed(window, this, key, scancode, action, mods);
		// }

    private:
        Vk_CameraMisc _misc;
        Vk_CameraState _state;

        std::unique_ptr<I_Renderer> _renderer;
        std::unique_ptr<I_ViewerSteering> _steering;

        std::unique_ptr<I_ViewerSteering> setSteering(const Vk_CameraMisc& misc){
			if(misc.SteeringType == Vk_SteeringType::CAMERA_CENTRIC){
				return std::make_unique<Vk_ViewerSteering_CameraCentric>();
			}
			else if(misc.SteeringType == Vk_SteeringType::OBJECT_CENTRIC){
				return std::make_unique<Vk_ViewerSteering_ObjectCentric>();
			}

            return nullptr;
		}
    };
}