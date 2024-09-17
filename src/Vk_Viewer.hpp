#pragma once

#include <unordered_map>
#include <thread>
#include <shared_mutex>
#include <condition_variable>

#include "Defines.h"
#include "./utils/Ut_Utils.hpp"
#include "./cameras/Vk_Camera.hpp"
#include "./cameras/I_Layout.hpp"
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
    public:
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
        _lwws_window(nullptr)
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
            _eventThread = std::thread(&Vk_Viewer::_run, this);
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

        void _run(){
#if defined(PLATFORM_LINUX)
            auto lwws_window = LWWS::LWWS_Window_X11(_name, _width, _height, _bgColor, {}, _resizable, _disableMouseOnHover, _mouseHoverTimeout, _bindLWWSSampleCallbacks);
#elif defined(PLATFORM_WINDOWS)
            UT::Ut_Logger::RuntimeError(typeid(this), "Not implemented!");
#else
            UT::Ut_Logger::RuntimeError(typeid(this), "Not implemented!");
#endif
            for(auto& c : _cameras){
                lwws_window.addViewport({{c.first, c.second.state()->viewport}}, false);
            }

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
    };
}
