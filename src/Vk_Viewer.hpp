#pragma once

#include <unordered_map>
#include <thread>
#include <shared_mutex>
#include <condition_variable>

#include "Defines.h"
#include "./utils/Ut_Utils.hpp"
#include "./cameras/Vk_Camera.hpp"
#include "./cameras/I_Layout.hpp"
#include "./tasks/Vk_Actions.hpp"
#include "./application/Vk_Device.hpp"

namespace VK5 {
    class Vk_Viewer{
        Vk_Device* _device;
        std::unordered_map<LWWS::TViewportId, Vk_Camera> _cameras;
        std::string _name;
        int _width, _height;
        bool _resizable;
        UT::Ut_RGBColor _bgColor;
        std::string _screenshotPath;
        bool _disableMouseOnHover;
        int _mouseHoverTimeout;
        int _freshPoolSize;
        bool _bindLWWSSampleCallbacks;
        bool _running;
        std::thread _eventThread;
        std::shared_mutex _mutex;
        std::thread _sleepThread;
        std::condition_variable_any _sleepCondition;
        LWWS::LWWS_Window* _lwws_window;
        LWWS::TViewportId _lastSteeredViewport;

    public:
        // public components
        Vk_Actions Actions;

        Vk_Viewer(
            std::string name, int width, int height, 
            bool resizable=true, const UT::Ut_RGBColor& bgColor=UT::RGB::White, 
            std::string screenshotPath="./", bool disableMouseOnHover=true,
            int mouseHoverTimeout=500, int freshPoolSize=100, bool bindLWWSSampleCallbacks=false
        )
        :
        _device(nullptr),
        _name(name), _width(width), _height(height), _resizable(resizable), _bgColor(bgColor),
        _screenshotPath(screenshotPath), _disableMouseOnHover(disableMouseOnHover), _mouseHoverTimeout(mouseHoverTimeout),
        _freshPoolSize(freshPoolSize), _bindLWWSSampleCallbacks(bindLWWSSampleCallbacks), _running(false),
        _lwws_window(nullptr), _lastSteeredViewport(-1)
        {

        }

        ~Vk_Viewer(){
            if(_eventThread.joinable()){
                _eventThread.join();
            }
        }

        void addCameras(std::vector<Vk_CameraInit> cameras){
            for(const auto& c : cameras){
                if(_cameras.contains(c.Viewport.viewportId())){
                    UT::Ut_Logger::RuntimeError(typeid(this), "Viewer already contains viewportIds {0}. Given viewportId is {1}!", UT::Ut_Utils::toStr_unorderedMapKeys(_cameras), c.Viewport.viewportId());
                }
                _cameras.insert({c.Viewport.viewportId(), Vk_Camera(c)});
            }
        }

        void addLayout(const I_Layout& layout){
            _cameras = layout.layout(_width, _height);
        }

        void runAsync(){
            _eventThread = std::thread(&Vk_Viewer::_eventLoop, this);
            while(!isRunning());
        }

        bool isRunning(){
            auto lock = std::shared_lock<std::shared_mutex>(_mutex);
            return _running;
        }

        void close(){
            auto lock = std::lock_guard<std::shared_mutex>(_mutex);
            _running = false;
            if(_lwws_window != nullptr) _lwws_window->emit_windowEvent_Empty();
        }

        void sleepResponsively(std::chrono::milliseconds ms){
            _sleepThread = std::thread(&Vk_Viewer::_wait, this, ms);
            _sleepThread.join();
        }

        Vk_Device* const vk_device() const { return _device; }

    private:
        void _wait(std::chrono::milliseconds ms){
            auto lock = std::unique_lock<std::shared_mutex>(_mutex);
            _sleepCondition.wait_for(lock, ms);
        }

        void _eventLoop(){
#if defined(PLATFORM_LINUX)
            auto lwws_window = LWWS::LWWS_Window_X11(_name, _width, _height, _bgColor, _resizable, _disableMouseOnHover, _mouseHoverTimeout, _bindLWWSSampleCallbacks);
#elif defined(PLATFORM_WINDOWS)
            UT::Ut_Logger::RuntimeError(typeid(this), "Not implemented!");
#else
            UT::Ut_Logger::RuntimeError(typeid(this), "Not implemented!");
#endif
            for(auto& c : _cameras){
                lwws_window.addViewport(c.second.state()->viewport, false);
            }

            lwws_window.bind_IntKey_Callback(this, &Vk_Viewer::_onKey);
			lwws_window.bind_MouseAction_Callback(this, &Vk_Viewer::_onMouseAction);
			lwws_window.bind_WindowState_Callback(this, &Vk_Viewer::_onWindowAction);

            _lwws_window = &lwws_window;
            lwws_window.windowEvents_Init();
            lwws_window.emit_windowEvent_Paint();

            {
                auto lock = std::lock_guard<std::shared_mutex>(_mutex);
                _running = true;
            }

            while(lwws_window.windowEvents_Exist()){
                {
                    auto lock = std::shared_lock<std::shared_mutex>(_mutex);
                    if(!_running) break;
                }

                lwws_window.windowEvents_Pump();

                if(lwws_window.windowShouldClose()){
                    break;
                }
            }

            {
                auto lock = std::lock_guard<std::shared_mutex>(_mutex);
                _running = false;
            }
            _lwws_window = nullptr;
            _sleepCondition.notify_all();
        }

        void _screenshot(const std::string& filename){
        }

        void _onWindowAction(int w, int h, int px, int py, const std::set<int>& pressedKeys, LWWS::WindowAction windowAction, void* aptr){
            UT::Ut_Logger::HighlightedGreen("...Window Action...");
		}

        void _onMouseAction(int px, int py, int dx, int dy, float dz, const std::set<int>& pressedKeys, LWWS::MouseButton mouseButton, LWWS::ButtonOp op, LWWS::MouseAction mouseAction, void* aptr){
			// std::cout << px << " " << py << " " << dx << " " << dy << " " << dz << " " << LWWS::LWWS_Key::IntKey2String(pressedKeys) << " " << LWWS::MouseButton2String(mouseButton) << " " << LWWS::ButtonOp2String(op) << " " << LWWS::MouseAction2String(mouseAction) << std::endl;
            // figure out which camera
            TSteeringGroup lastSteeredViewport = _lastSteeredViewport;
            LWWS::TViewportId viewportId = -1;
            {
                TSteeringGroup steeringGroup = -1;
                for(auto& c : _cameras){
                    if(c.second.contains(px, py)){
                        steeringGroup = c.second.misc()->SteeringGroup;
                        viewportId = c.second.state()->viewport.viewportId();
                        break;
                    }
                }
                if(steeringGroup < 0 && lastSteeredViewport < 0) return;
            }

			if(mouseAction == LWWS::MouseAction::MouseScroll){
				auto lctrl = LWWS::LWWS_Key::KeyToInt(LWWS::LWWS_Key::Special::LControl);
				auto rctrl = LWWS::LWWS_Key::KeyToInt(LWWS::LWWS_Key::Special::RControl);
				auto lshift = LWWS::LWWS_Key::KeyToInt(LWWS::LWWS_Key::Special::LShift);
				auto rshift = LWWS::LWWS_Key::KeyToInt(LWWS::LWWS_Key::Special::RShift);

				bool ctrlPressed = pressedKeys.find(lctrl) != pressedKeys.end() || pressedKeys.find(rctrl) != pressedKeys.end();
				bool shiftPressed = pressedKeys.find(lshift) != pressedKeys.end() || pressedKeys.find(rshift) != pressedKeys.end();

				if(!ctrlPressed && !shiftPressed) dz *= 0.05f;
				else if(!ctrlPressed) dz *= 0.20f;
				else if(!shiftPressed) dz *= 0.5f;

                _cameras.at(viewportId).onMouseAction(px, py, dx, dy, dz, pressedKeys, mouseButton, op, mouseAction, aptr);
                UT::Ut_Logger::HighlightedYellow("   zoom viewport {0}", viewportId);
			}
			else if(mouseAction == LWWS::MouseAction::MouseButton && mouseButton == LWWS::MouseButton::Left && op == LWWS::ButtonOp::Down){
				_lastSteeredViewport = viewportId;
			}
			else if(mouseAction == LWWS::MouseAction::MouseMove && mouseButton == LWWS::MouseButton::Left && op == LWWS::ButtonOp::SteadyPress){
                TSteeringGroup steeringGroup = _cameras.at(_lastSteeredViewport).misc()->SteeringGroup;

				for(auto& c : _cameras){
                    if(c.second.misc()->SteeringGroup == steeringGroup)
                        c.second.onMouseAction(px, py, dx, dy, dz, pressedKeys, mouseButton, op, mouseAction, aptr);
                }
			}
			else if(mouseAction == LWWS::MouseAction::MouseButton && mouseButton == LWWS::MouseButton::Left && op == LWWS::ButtonOp::Up){
				_lastSteeredViewport = -1;
			}

            // the viewport should take over here, but one never knows...
			// for(const auto& c : vk_cameras()){
			// 	c.second->vk_lastMousePosition(px, py);
			// }
		}

        void _onKey(int k, LWWS::ButtonOp op, const std::set<int>& otherPressedKeys, void* aptr){
			int lctrl = LWWS::LWWS_Key::KeyToInt(LWWS::LWWS_Key::Special::LControl);
			int rctrl = LWWS::LWWS_Key::KeyToInt(LWWS::LWWS_Key::Special::RControl);
			if (op == LWWS::ButtonOp::Down) {
				// int lshift = LWWS::LWWS_Key::KeyToInt(LWWS::LWWS_Key::Special::LShift);
				// int rshift = LWWS::LWWS_Key::KeyToInt(LWWS::LWWS_Key::Special::RShift);

				bool ctrlPressed = otherPressedKeys.contains(lctrl) || otherPressedKeys.contains(rctrl);
				int sKey = LWWS::LWWS_Key::KeyToInt('s');
				bool sPressed = otherPressedKeys.contains(sKey);

				if (ctrlPressed && sPressed) {
					long long timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
					std::string filename = std::string("screenshot_") + std::to_string(timestamp) + std::string(".jpeg");
					UT::Ut_Logger::Message(typeid(NoneObj), "Take screenshot: {0}", filename);
					_screenshot(filename);
				}

				// todo: do some translating
				Actions.execAction(k);
			}
		}
    };
}
