#include <functional>
#include <memory>
#include <map>
#include <iostream>
#include <shared_mutex>
#include <set>
#include <random>

#include "../../utils/Ut_Colors.hpp"
#include "../../utils/Ut_Utils.hpp"
#include "../../utils/Ut_Logger.hpp"
#include "lwws_viewport.hpp"
#include "lwws_func.hpp"
#ifdef PLATFORM_WINDOWS
    #include "lwws_key_win.hpp"
    typedef LWWS::LWWS_Key_Win LWWS_Key_P;
#else
    #include "lwws_key_x11.hpp"
    typedef LWWS::LWWS_Key_X11 LWWS_Key_P;
#endif


namespace LWWS {
    class LWWS_Window {
    protected:
        LWWS_MouseState _mouseState;
        LWWS_WindowState _windowState;

        MouseButton _currentMouseButtonPressed = MouseButton::NoButton;
        std::set<int> _currentIntPressed = {};
        std::unordered_map<TViewportId, LWWS_Viewport> _viewports;
        UT::Ut_RGBColor _bgColor;
        std::random_device _randomDevice;
        std::uniform_int_distribution<std::mt19937::result_type> _dist01;
        std::mt19937 _randomGenerator;

    private:
        std::shared_mutex _sizeMutex;
        void* _aptr = nullptr;
        // std::shared_ptr<LWWS_Func> _on_Char_Callback = nullptr;              // char, Up/Down
        // std::shared_ptr<LWWS_Func> _on_SpecialKey_Callback = nullptr;        // SpecialKey, Up/Down
        std::shared_ptr<LWWS_Func> _on_Destructor_Callback = nullptr;        // void
        std::shared_ptr<LWWS_Func> _on_IntKey_Callback = nullptr;            // to integer formated key
        std::shared_ptr<LWWS_Func> _on_WindowState_Callback = nullptr;       // int x, int y, Activate/Deactivate/Resize/Max/Min/Moved
        std::shared_ptr<LWWS_Func> _on_MouseAction_Callback = nullptr;       // int mx, int my, int dz, MouseButton, ButtonOp, MouseAction

    public:
        LWWS_Window(
            int width, 
            int height, 
            const UT::Ut_RGBColor& bgColor,
            const std::unordered_map<TViewportId, LWWS_Viewport>& viewports,
            bool disableMousePointerOnHover, 
            int hoverTimeoutMS,
            bool bindSamples
        ) 
        : 
        _mouseState(LWWS_MouseState{
            .leftWindow=false,
            .transitionedAtX=-1,
            .transitionedAtY=-1,
            .posX=-1,
            .posY=-1,
            .dx=0,
            .dy=0,
            .hoverTimeoutMS=hoverTimeoutMS,
            .disableMousePointerOnHover=disableMousePointerOnHover,
            .isHovering=false,
        }),
        _windowState({
            .active=true,
            .initWidth=width, 
            .initHeight=height, 
            .width=width, 
            .height=height, 
            .posX=-1,
            .posY=-1,
            .minimized=false,
            .initialized=false,
            .shouldClose=false,
        }),
        _currentMouseButtonPressed(MouseButton::NoButton),
        _currentIntPressed({}),
        _viewports(viewports),
        _bgColor(bgColor),
        _dist01(std::uniform_int_distribution<std::mt19937::result_type>(0,1)),
        _randomGenerator(std::mt19937(_randomDevice()))
        {
            if(_viewports.size() < 1){
                std::runtime_error("LWWS window requires at least one viewport!");
            }

            if(bindSamples){
                bind_Destructor_Callback(this, &LWWS_Window::sample_onDestructorCallback);
                bind_IntKey_Callback(this, &LWWS_Window::sample_onIntKeyCallback);
                bind_WindowState_Callback(this, &LWWS_Window::sample_onWindowStateCallback);
                bind_MouseAction_Callback(this, &LWWS_Window::sample_onMouseActionCallback);
            }
        }

        virtual ~LWWS_Window(){}

        /**
         * Loopers
         */
        virtual inline void windowEvents_Init() = 0;
        virtual inline bool windowEvents_Exist() = 0;
        virtual inline void windowEvents_Pump() = 0;
        /*
        * Paint events semantics
        *  - emit_windowEvent_Paint() will initiate a Paint event for all viewports
        *  - emit_windowEvent_Paint(id) will initiate a Paint event for viewport with the passed id 
        *  - the viewport id has to be the same as the viewportId of the associated viewport
        */
        virtual inline void emit_windowEvent_Paint() = 0;
        virtual inline void emit_windowEvent_Paint(int id) = 0;
        virtual inline void emit_windowEvent_Empty() = 0;
        inline bool windowShouldClose() { return _windowState.shouldClose; }

        /**
         * Getter/Setter
         */
        int hoverTimeoutMS() { return _mouseState.hoverTimeoutMS; }

        size_t canvasSize(/*out*/int& width, /*out*/int& height){
            std::shared_lock<std::shared_mutex> lock(_sizeMutex);
            width = _windowState.width;
            height = _windowState.height;
            return width*height*4;
        }

        size_t canvasInitSize(/*out*/int& initWidth, /*out*/int& initHeight){
            initWidth = _windowState.initWidth;
            initHeight = _windowState.initHeight;
            return initWidth*initHeight*4;
        }

        const std::unordered_map<TViewportId, LWWS_Viewport>& viewports() {
            return _viewports;
        }

        virtual bool frameSize(/*out*/int& width, /*out*/int& height) = 0;

        virtual void viewportSize(TViewportId viewportId, /*out*/int& width, /*out*/int& height) {
            if(!_viewports.contains(viewportId)){
                throw std::runtime_error(genViewportErrMsg(viewportId));
            }

            width = _viewports.at(viewportId).width();
            height = _viewports.at(viewportId).height();
        }

        bool hasViewport(TViewportId viewportId){
            return _viewports.contains(viewportId);
        }

        virtual void addViewport(const std::unordered_map<TViewportId, LWWS_Viewport>& viewports, bool emitPaint=true){
            for(const auto& vp : viewports){
                if(_viewports.contains(vp.first)){
                    UT::Ut_Logger::RuntimeError(typeid(this), "ViewportIds [{0}] area already in use. {1} given.", UT::Ut_Utils::toStr_unorderedMapKeys(_viewports), vp.first);
                }
                _viewports.insert({vp.first, vp.second});
                _viewports.at(vp.first).setParentState(&_mouseState, &_windowState);
            }
            if(emitPaint) emit_windowEvent_Paint();
        }

        virtual void removeViewport(const std::vector<TViewportId>& viewportIds, bool emitPaint=true){
            for(const auto& vpid : viewportIds){
                if(!_viewports.contains(vpid)){
                    UT::Ut_Logger::RuntimeError(typeid(this), "ViewportIds [{0}] area in use. {1} not found in list.", UT::Ut_Utils::toStr_unorderedMapKeys(_viewports), vpid);
                }
                _viewports.erase(vpid);
            }
            if(emitPaint) emit_windowEvent_Paint();
        }

        /**
         * Binder
         */
        void bind_Destructor_Callback(t_void_func_null f){
            _on_Destructor_Callback = std::static_pointer_cast<LWWS_Func>(std::make_shared<LWWS_VoidFuncNull>(f));
        }

        void bind_IntKey_Callback(t_int_func_null f){
            _on_IntKey_Callback = std::static_pointer_cast<LWWS_Func>(std::make_shared<LWWS_IntFuncNull>(f));
        }

        void bind_WindowState_Callback(t_windowstate_func_null f){
            _on_WindowState_Callback = std::static_pointer_cast<LWWS_Func>(std::make_shared<LWWS_WindowStateFuncNull>(f));
        }

        void bind_MouseAction_Callback(t_mouseaction_func_null f){
            _on_MouseAction_Callback = std::static_pointer_cast<LWWS_Func>(std::make_shared<LWWS_MouseActionFuncNull>(f));
        }

        template<class ObjType>
        void bind_Destructor_Callback(ObjType* obj, t_void_func<ObjType> f){
            _on_Destructor_Callback = std::static_pointer_cast<LWWS_Func>(std::make_shared<LWWS_VoidFunc<ObjType>>(obj, f));
        }

        template<class ObjType>
        void bind_IntKey_Callback(ObjType* obj, t_int_func<ObjType> f){
            _on_IntKey_Callback = std::static_pointer_cast<LWWS_Func>(std::make_shared<LWWS_IntFunc<ObjType>>(obj, f));
        }

        template<class ObjType>
        void bind_WindowState_Callback(ObjType* obj, t_windowstate_func<ObjType> f){
            _on_WindowState_Callback = std::static_pointer_cast<LWWS_Func>(std::make_shared<LWWS_WindowStateFunc<ObjType>>(obj, f));
        }

        template<class ObjType>
        void bind_MouseAction_Callback(ObjType* obj, t_mouseaction_func<ObjType> f){
            _on_MouseAction_Callback = std::static_pointer_cast<LWWS_Func>(std::make_shared<LWWS_MouseActionFunc<ObjType>>(obj, f));
        }

        std::string genViewportErrMsg(TViewportId id){
            std::stringstream errmsg;
            errmsg << "Viewport with id " << id << " does not exist! Candidates are [";
            for(const auto& vp : _viewports) errmsg << vp.first << ",";
            errmsg << "]";
            return errmsg.str();
        }

    private:

        /**
         * Sample callbacks
         */
        void sample_onDestructorCallback(void* aptr){
            std::cout << "Destructor event called" << std::endl;
        }

        void sample_onIntKeyCallback(int k, ButtonOp op, const std::set<int>& otherPressedKeys, void* aptr){
            std::cout << "IntKey\n" << 
                               "\tkey:   " << LWWS_Key::IntKey2String(k) << "\n" <<
                               "\tOp:    " << ButtonOp2String(op) << "\n" <<
                               "\tOther: " << LWWS_Key::IntKey2String(otherPressedKeys) << std::endl;
        }

        void sample_onWindowStateCallback(int x, int y, int px, int py, const std::set<int>& pressedKeys, WindowAction windowAction, void* aptr){
            std::cout << "Window state\n" << 
                                     "\tx:        " << x << "\n" <<
                                     "\ty:        " << y << "\n" <<
                                    "\tpx:        " << px << "\n" <<
                                    "\tpy:        " << py << "\n" <<
                                    "\tkey:       " << LWWS_Key::IntKey2String(pressedKeys) << "\n" <<
                                    "\tto state:  " << WindowAction2String(windowAction) << std::endl;
        }

        void sample_onMouseActionCallback(int px, int py, int dx, int dy, float dz, const std::set<int>& pressedKeys, MouseButton mouseButton, ButtonOp op, MouseAction mouseAction, void* aptr){
            std::cout << "Mouse action\n" << 
                                     "\tpx:      " << px << "\n" <<
                                     "\tpy:      " << py << "\n" <<
                                     "\tdx:      " << dx << "\n" <<
                                     "\tdy:      " << dy << "\n" <<
                                     "\tdz:      " << dz << "\n" <<
                                     "\tkey:     " << LWWS_Key::IntKey2String(pressedKeys) << "\n" <<
                                     "\tOp:      " << ButtonOp2String(op) << "\n" <<
                                     "\tbutton:  " << MouseButton2String(mouseButton) << "\n" <<
                                     "\taction:  " << MouseAction2String(mouseAction) << std::endl;
        }

    protected:
        int choose01(){
            return static_cast<int>(_dist01(_randomGenerator));
        }

        /**
         * State methods
         */
        static inline bool wndSetAPtr(LWWS_Window* window, void*& aptr){
            if(window == nullptr) return false;
            window->_aptr = aptr;
            return true;
        }

        static inline void wndPaint(LWWS_Window* window){
            if(window->_on_WindowState_Callback) (*window->_on_WindowState_Callback)(
                                                    window->_windowState.width, window->_windowState.height,
                                                    window->_windowState.posX, window->_windowState.posY,
                                                    window->_currentIntPressed,
                                                    WindowAction::Paint,
                                                    window->_aptr
                                                );
        }

        static inline void wndEnableHover(LWWS_Window* window){
            if(window == nullptr) return;
            if(window->_mouseState.isHovering) return;
            if(window->_mouseState.leftWindow) return;
            if(!window->_windowState.active) return;
            
            window->_mouseState.isHovering = true;
            if(window->_mouseState.disableMousePointerOnHover) LWWS_Key_P::LWWS_ShowCursor(false);
            if(window->_on_MouseAction_Callback) (*window->_on_MouseAction_Callback)(
                                                    window->_mouseState.posX, window->_mouseState.posY, 
                                                    0, 0, 0,
                                                    window->_currentIntPressed,
                                                    window->_currentMouseButtonPressed,
                                                    (window->_currentMouseButtonPressed != MouseButton::NoButton ? ButtonOp::SteadyPress : ButtonOp::NoOp),
                                                    MouseAction::HoverStart,
                                                    window->_aptr
                                                );
        }

        static inline void wndDisableHover(LWWS_Window* window){
            if(window == nullptr) return;
            if(!window->_mouseState.isHovering) return;

            window->_mouseState.isHovering = false;
            if(window->_mouseState.disableMousePointerOnHover) LWWS_Key_P::LWWS_ShowCursor(true);
            if(window->_on_MouseAction_Callback) (*window->_on_MouseAction_Callback)(
                                                    window->_mouseState.posX, window->_mouseState.posY, 
                                                    0, 0, 0,
                                                    window->_currentIntPressed,
                                                    window->_currentMouseButtonPressed,
                                                    (window->_currentMouseButtonPressed != MouseButton::NoButton ? ButtonOp::SteadyPress : ButtonOp::NoOp),
                                                    MouseAction::HoverStop,
                                                    window->_aptr
                                                );
        }

        static inline bool wndMouseMoved(LWWS_Window* window, int xPos, int yPos, bool sendDisableHoverMsg=true){
            if(window == nullptr) return false;
            if(window->_mouseState.leftWindow) return false;
            if(xPos == window->_mouseState.posX && yPos == window->_mouseState.posY) return false;
            int dx = window->_mouseState.posX - xPos;
            int dy = window->_mouseState.posY - yPos;
            window->_mouseState.posX = xPos;
            window->_mouseState.posY = yPos;
            window->_mouseState.dx = dx;
            window->_mouseState.dy = dy;

            if(sendDisableHoverMsg) wndDisableHover(window);

            if(window->_on_MouseAction_Callback) (*window->_on_MouseAction_Callback)(
                                                    window->_mouseState.posX, window->_mouseState.posY, 
                                                    dx, dy, 0,
                                                    window->_currentIntPressed,
                                                    window->_currentMouseButtonPressed,
                                                    (window->_currentMouseButtonPressed != MouseButton::NoButton ? ButtonOp::SteadyPress : ButtonOp::NoOp),
                                                    MouseAction::MouseMove,
                                                    window->_aptr
                                                );

            return true;
        }

        static inline void wndMouseLeftWindow(LWWS_Window* window, bool mouseLeftWindow){
            if(window == nullptr) return;
            if(window->_mouseState.leftWindow == mouseLeftWindow) return;
            if(mouseLeftWindow){
                window->_currentMouseButtonPressed = MouseButton::NoButton;
            }
            window->_mouseState.leftWindow = mouseLeftWindow;
            window->_mouseState.transitionedAtX = window->_mouseState.posX;
            window->_mouseState.transitionedAtY = window->_mouseState.posY;
            if(window->_on_MouseAction_Callback) (*window->_on_MouseAction_Callback)(
                                                    window->_mouseState.posX, window->_mouseState.posY, 
                                                    0, 0, 0,
                                                    window->_currentIntPressed,
                                                    window->_currentMouseButtonPressed,
                                                    (window->_currentMouseButtonPressed != MouseButton::NoButton ? ButtonOp::SteadyPress : ButtonOp::NoOp),
                                                    mouseLeftWindow ? MouseAction::MouseLeave : MouseAction::MouseEnter,
                                                    window->_aptr
                                                );
        }

        static inline void wndSetActive(LWWS_Window* window, bool windowActive){
            if(window == nullptr) return;
            if(window->_windowState.active == windowActive) return;
            window->_windowState.active = windowActive;
            if(window->_on_WindowState_Callback) (*window->_on_WindowState_Callback)(
                                                    window->_windowState.width, window->_windowState.height,
                                                    window->_windowState.posX, window->_windowState.posY,
                                                    window->_currentIntPressed,
                                                    windowActive ? WindowAction::Activated : WindowAction::Deactivated,
                                                    window->_aptr
                                                );
            if(windowActive == false){
                window->_currentIntPressed.clear();
            }
        }

        static inline void wndMouseScroll(LWWS_Window* window, double mouseDz){
            if(window == nullptr) return;
            if(window->_on_MouseAction_Callback) (*window->_on_MouseAction_Callback)(
                                                    window->_mouseState.posX, window->_mouseState.posY, 
                                                    0, 0, mouseDz,
                                                    window->_currentIntPressed,
                                                    window->_currentMouseButtonPressed,
                                                    ButtonOp::NoOp,
                                                    MouseAction::MouseScroll,
                                                    window->_aptr
                                                );
        }

        static inline void wndResize(LWWS_Window* window, int width, int height, bool windowMinimized){
            if(window == nullptr) return;

            bool iconify = false;
            if((window->_windowState.minimized && !windowMinimized) || (!window->_windowState.minimized && windowMinimized)){
                iconify = true;
            }
            window->_windowState.minimized = windowMinimized;
            auto action = WindowAction::SteadyPress;
            if(iconify && windowMinimized) action = WindowAction::Minimized;
            else if(iconify && !windowMinimized) action = WindowAction::Maximized;

            else if(!windowMinimized) {
                if(width != window->_windowState.width || height != window->_windowState.height){
                    std::lock_guard<std::shared_mutex> lock(window->_sizeMutex);
                    window->_windowState.width = width; // if the window is minimized, this one becomes 0
                    window->_windowState.height = height;
                    action = WindowAction::Resized;
                }
            }

            // int c = window->choose01();
            for(auto& v : window->_viewports){
                v.second.resize();
                window->emit_windowEvent_Paint(v.first);
            }

            if(action != WindowAction::SteadyPress && window->_on_WindowState_Callback) (*window->_on_WindowState_Callback)(
                                                    window->_windowState.width, window->_windowState.height,
                                                    window->_windowState.posX, window->_windowState.posY,
                                                    window->_currentIntPressed,
                                                    action,
                                                    window->_aptr
                                                );
        }

        static inline bool wndMoved(LWWS_Window* window, int xPos, int yPos){
            if(window == nullptr) return false;
            if(window->_windowState.posX == xPos && window->_windowState.posY == yPos) return false;
            {
                window->_windowState.posX = xPos;
                window->_windowState.posY = yPos;
            }

            if(window->_on_WindowState_Callback) (*window->_on_WindowState_Callback)(
                                                    window->_windowState.width, window->_windowState.height,
                                                    window->_windowState.posX, window->_windowState.posY,
                                                    window->_currentIntPressed,
                                                    WindowAction::Moved,
                                                    window->_aptr
                                                );
            return true;
        }

        static inline void wndCloseOperations(LWWS_Window* window){
            if(window == nullptr) return;
            window->_windowState.shouldClose = true;
            if(window->_on_WindowState_Callback) (*window->_on_WindowState_Callback)(
                                                    window->_windowState.width, window->_windowState.height,
                                                    window->_windowState.posX, window->_windowState.posY,
                                                    window->_currentIntPressed,
                                                    WindowAction::Closed,
                                                    window->_aptr
                                                );
        }

        static inline void wndMousePressed(LWWS_Window* window, MouseButton button, ButtonOp op){
            if(window == nullptr) return;
            if(window->_mouseState.leftWindow) return;

            if(op == ButtonOp::Down) window->_currentMouseButtonPressed = button;
            else if(op == ButtonOp::Up) window->_currentMouseButtonPressed = MouseButton::NoButton;

            if(window->_on_MouseAction_Callback) (*window->_on_MouseAction_Callback)(
                                                    window->_mouseState.posX, window->_mouseState.posY, 
                                                    0, 0, 0,
                                                    window->_currentIntPressed,
                                                    button, op,
                                                    MouseAction::MouseButton,
                                                    window->_aptr
                                                );
        }

        static inline void wndSpecialKeyPressed(LWWS_Window* window, LWWS_Key::Special key, ButtonOp op, const std::set<int>& stillPressedKeys){
            int kk = LWWS_Key::KeyToInt(key);
            if(key == LWWS_Key::Special::RandomKey && !window->_currentIntPressed.empty() && op == ButtonOp::Up){
                for(const auto& pk : window->_currentIntPressed){
                    if(stillPressedKeys.find(pk) == stillPressedKeys.end()){
                        int temp = pk; // get the value before we erase pk
                        window->_currentIntPressed.erase(pk);
                        if(window->_on_IntKey_Callback) (*window->_on_IntKey_Callback)(temp, op, window->_currentIntPressed, window->_aptr);
                        break;
                    }
                }
                return;
            }
            if(key == LWWS_Key::Special::RandomKey) return;

            if(op == ButtonOp::Up) {
                window->_currentIntPressed.erase(kk);
            } 
            else if(window->_currentIntPressed.find(kk) != window->_currentIntPressed.end()){
                op = ButtonOp::SteadyPress;
            }
            else{
                window->_currentIntPressed.insert(kk);
            }

            // callback propagation
            if(window->_on_IntKey_Callback) (*window->_on_IntKey_Callback)(kk, op, window->_currentIntPressed, window->_aptr);
        }

        static inline void wndCharPressed(LWWS_Window* window, char key, ButtonOp op){
            int kk = LWWS::LWWS_Key::KeyToInt(key);
            if(op == ButtonOp::Down){
                if(window->_currentIntPressed.find(kk) != window->_currentIntPressed.end()){
                    op = ButtonOp::SteadyPress;
                }
                else{
                    window->_currentIntPressed.insert(kk);
                }
            }
            else if(op == ButtonOp::Up){
                if(window->_currentIntPressed.find(kk) != window->_currentIntPressed.end()){
                    window->_currentIntPressed.erase(kk);
                }
            }

            // callback propagation
            if(window->_on_IntKey_Callback) (*window->_on_IntKey_Callback)(kk, op, window->_currentIntPressed, window->_aptr);
        }

        static inline bool wndRequiresInit(LWWS_Window* window){
            return !window->_windowState.initialized;
        }

        static inline void wndInit(LWWS_Window* window, bool mouseInRectAtInit){
            if(window == nullptr) return;
            if(!mouseInRectAtInit){
                window->_mouseState.leftWindow = true;
            }
            window->_windowState.initialized = true;

            if(window->_on_WindowState_Callback) (*window->_on_WindowState_Callback)(
                                                    window->_windowState.width, window->_windowState.height,
                                                    window->_windowState.posX, window->_windowState.posY,
                                                    window->_currentIntPressed,
                                                    WindowAction::Created,
                                                    window->_aptr
                                                );
        }
    };
}
