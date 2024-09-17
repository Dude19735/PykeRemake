#pragma once
#include <stdexcept>
#include <condition_variable>
#include "lwws_window.hpp"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/Xatom.h>
#include <cstring>

namespace LWWS {
    // class RedrawQueue {
    //     std::queue<int> _queue;
    //     std::mutex _mutex;
    // public:
    //     RedrawQueue(){}
    //     void pushRedrawEvent(){
    //         auto lock = std::lock_guard<std::mutex>(_mutex);
    //         _queue.push(1);
    //     }

    //     bool hasRedrawEvent(){
    //         auto lock = std::lock_guard<std::mutex>(_mutex);
    //         if(_queue.size() == 0) return false;
    //         _queue.pop();
    //         return true;
    //     }
        
    // };

    class LWWS_Window_X11: public LWWS_Window{
        Display* _display;
        Window _window;
        std::set<Window> _x11viewportsS;
        std::unordered_map<TViewportId, Window> _x11viewportsMap;
        Screen* _screen;
        int _screenId;
        XEvent _ev;
        // GC _gc;
        // GC _subGc;

        long unsigned int _cFocused;
        long unsigned int _cMaxhorz;
        long unsigned int _cMaxvert;
        long unsigned int _cHidden;
        int _winstate = 0; // 0 = normal, 1 = maximized, 2 = minimized

        KeySym _key;
        char _text[255];
        bool _flush;

    public:
        LWWS_Window_X11(
            std::string windowTitle,
            int width,
            int height,
            const UT::Ut_RGBColor& bgColor,
            const std::unordered_map<TViewportId, LWWS_Viewport>& viewports,
            bool resizable,
            bool disableMousePointerOnHover=false,
            int hoverTimeoutMS=500,
            bool bindSamples=false
        ) 
        : 
        LWWS_Window(width, height, bgColor, viewports, disableMousePointerOnHover, hoverTimeoutMS, bindSamples),
        _display(nullptr),
        _window(0),
        _x11viewportsS({}),
        _x11viewportsMap({}),
        _screen(nullptr),
        _screenId(0),
        _ev({}),
        _cFocused(0),
        _cMaxhorz(0),
        _cMaxvert(0),
        _cHidden(0),
        _winstate(0),
        _key(0),
        _flush(true)
        {
            /* use the information from the environment variable DISPLAY 
            to create the X connection:
            */	
            _display = XOpenDisplay((char *)0);
            _screenId = DefaultScreen(_display);
            unsigned long white = WhitePixel(_display,_screenId);  /* get color white */
            unsigned long black = BlackPixel(_display,_screenId);  /* get color white */

            _window = XCreateSimpleWindow(_display,DefaultRootWindow(_display),0,0,_windowState.width, _windowState.height, 0, white, createX11Color(UT::Ut_ColorUtils::rgb2str(_bgColor)));
            XSetStandardProperties(_display, _window, windowTitle.c_str(), "", None, NULL, 0, NULL);
            // std::cout << "Parent window: " << _window << std::endl;
            for(auto& v : _viewports){
                createX11Window(v.first);
            }

            Atom wm_delete = XInternAtom(_display, "WM_DELETE_WINDOW", 1);	
            XSetWMProtocols(_display, _window, &wm_delete, 1 );

            _cFocused = XInternAtom(_display, "_NET_WM_STATE_FOCUSED", 1);
            _cMaxhorz = XInternAtom(_display, "_NET_WM_STATE_MAXIMIZED_HORZ", 1);
            _cMaxvert = XInternAtom(_display, "_NET_WM_STATE_MAXIMIZED_VERT", 1);
            _cHidden =  XInternAtom(_display, "_NET_WM_STATE_HIDDEN", 1);

            XSelectInput(_display, _window, 
                ExposureMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask |
                PointerMotionMask | FocusChangeMask | StructureNotifyMask | PropertyChangeMask
            );

            // create the Graphics Context
            // _gc=XCreateGC(_display, _window, 0,0);
            // _subGc=XCreateGC(_display, _subWindow, 0, 0);

            if(!resizable){
                auto sh = XAllocSizeHints();
                sh->flags = PMinSize | PMaxSize;
                sh->min_width = sh->max_width = 100;
                sh->min_height = sh->max_height = 100;
                //XSetWMNormalHints(d, w, sh);
                XSetWMSizeHints(_display, _window, sh, XA_WM_NORMAL_HINTS);
                XFree(sh);
            }

            // Window stackPosition[2] = {_window, _subWindow};
            // XRestackWindows(_display, stackPosition, 2);
        }

        ~LWWS_Window_X11(){
            // XFreeGC(_display, _gc);
            // XFreeGC(_display, _subGc);
            // XDestroyWindow(_display, _subWindow);
            XDestroyWindow(_display, _window);
	        XCloseDisplay(_display);
        }

        // void getX11XcbWindowDescriptors(Display*& display, LWWS::TViewportId viewportId, uint32_t& screenId){
        //     display = _display;
        //     screenId = static_cast<uint32_t>(_screenId);
        // }

        void getX11XlibWindowDescriptors(Display*& display, LWWS::TViewportId viewportId, Window& window){
            display = _display;
            window = _window;
        }

        inline void windowEvents_Init() {
            XClearWindow(_display, _window);
	        XMapRaised(_display, _window);
            for(const auto& w : _x11viewportsMap){
                XClearWindow(_display, w.second);
	            XMapRaised(_display, w.second);
            }

        }

        inline bool windowEvents_Exist() {
            if(_windowState.shouldClose) return false;
            return true;
        }

        inline void windowEvents_Pump(){
            // std::unique_lock<std::mutex> lock(_mutex);
            // _condition.wait(lock, [this]() {
            //     // continue if we have pending events
            //     return XPending(_display)>0 || _redraw;
            // });
            // if(_redraw) {
            //     queueRedraw();
            //     _redraw = false;
            // }

            windowEventPump();
        }

        XExposeEvent createXExposeEvent(Window window, int posx, int posy, int width, int height){
            XExposeEvent pEv;
            memset(&pEv, 0, sizeof(pEv));
            pEv = {
                .type = Expose,
                .serial = 0,
                .send_event = false,
                .display = _display,
                .window = window,
                .x = posx,
                .y = posy,
                .width = width,
                .height = height,
                .count = 0
            };

            return pEv;
        }

        XClientMessageEvent createXClientMessageEvent(Window window, std::string msg="empty"){
            if(msg.size() > 19) msg = msg.substr(0,19);
            if(msg.size() == 0) msg="empty";

            XClientMessageEvent pEv;
            memset(&pEv, 0, sizeof(pEv));
            pEv = {
                .type = ClientMessage,
                .serial = 0,
                .send_event = false,
                .display = _display,
                .window = window,
                .message_type = XA_NOTICE,
                .format = 8,
                .data = msg.c_str()[0]
            };

            return pEv;
        }

        inline void emit_windowEvent_Empty() {
            auto pEv = createXClientMessageEvent(_window);
            XSendEvent(_display, _window, true, ExposureMask, reinterpret_cast<XEvent*>(&pEv));
            XFlush(_display);
        }

        inline void emit_windowEvent_Paint(){
            int s = static_cast<int>(_x11viewportsMap.size());
            _flush = false; // make sure, individual emit_windowEvent_Paint(i) don't flush the display every time
            for(int i=0; i<s; ++i){
                emit_windowEvent_Paint(i);
            }
            XFlush(_display);
            _flush = true;
        }

        inline void emit_windowEvent_Paint(TViewportId id){
            if(!_viewports.contains(id)) {
                throw std::runtime_error(genViewportErrMsg(id));
            }

            const auto& vp = _viewports.at(id);
            const auto& x11vp = _x11viewportsMap.at(id);
            // std::cout << vp.width() << std::endl;
            XResizeWindow(_display, x11vp, static_cast<unsigned int>(vp.width()), static_cast<unsigned int>(vp.height()));
            XMoveWindow(_display, x11vp, vp.posW(), vp.posH());
            auto pEv = createXExposeEvent(x11vp, vp.posW(), vp.posH(), vp.width(), vp.height());
            XSendEvent(_display, x11vp, true, ExposureMask, reinterpret_cast<XEvent*>(&pEv));
            if(_flush) XFlush(_display);
        }

        bool frameSize(int& width, int& height){
            Window root;
            int x, y;
            unsigned int w, h, border_width, depth;
            int status = XGetGeometry(_display, _window, &root, &x, &y, &w, &h, &border_width, &depth);
            if(status == 0) return false;
            width = static_cast<int>(w);
            height = static_cast<int>(h);
            return true;
        }

        void addViewport(const std::unordered_map<TViewportId, LWWS_Viewport>& viewports, bool emitPaint=true) {
            LWWS_Window::addViewport(viewports, false);

            for(auto& vp : viewports){
                auto newWin = createX11Window(vp.first);
                XClearWindow(_display, newWin);
                XMapRaised(_display, newWin);
            }
            if(emitPaint) emit_windowEvent_Paint();
        }

        void removeViewport(const std::vector<TViewportId>& viewportIds, bool emitPaint=true){
            LWWS_Window::removeViewport(viewportIds, false);

            for(const auto& vpid : viewportIds){
                Window win = _x11viewportsMap.at(vpid);
                XDestroyWindow(_display, win);
                _x11viewportsS.erase(win);
                _x11viewportsMap.erase(vpid);
            }
            if(emitPaint) emit_windowEvent_Paint();
        }

    private:
        Window createX11Window(TViewportId viewportId){
            auto& vp = _viewports.at(viewportId);
            /* Display, Window, x, y, width, height, border_width, border_color, bg_color */
            auto newWin = XCreateSimpleWindow(
                _display, _window, vp.posW(), vp.posH(), vp.width(), vp.height(), vp.borderWidth(), 
                createX11Color(UT::Ut_ColorUtils::rgb2str(vp.borderColor())), 
                createX11Color(UT::Ut_ColorUtils::rgb2str(vp.bgColor()))
            );
            // std::cout << "Child window: " << newWin << std::endl;
            XRaiseWindow(_display, newWin);
            _x11viewportsS.insert(newWin);
            _x11viewportsMap.insert({viewportId, newWin});

            return newWin;
        }

        unsigned long createX11Color(const std::string& pattern){
            if(pattern.at(0) != '#' || (pattern.size() != 7 && pattern.size() != 13)){
                return 0;
            }

            XColor color;
            Colormap colormap;

            colormap = DefaultColormap(_display, 0);
            XParseColor(_display, colormap, pattern.c_str(), &color);
            XAllocColor(_display, colormap, &color);

            return color.pixel;
        }

        void windowEventPump(){
            XNextEvent(_display, &_ev);
            switch(_ev.type){
                case Expose:{
                    if(!_x11viewportsS.contains(_ev.xexpose.window)) break;
                    // std::cout << "paint event: " << _ev.xexpose.window << " " << _ev.xexpose.x << " " << _ev.xexpose.y << " " << _ev.xexpose.width << " " << _ev.xexpose.height << std::endl;

                    // std::cout << " ======================> " << std::endl;
                    if(_ev.xexpose.count==0) {
                        // std::cout << "..... paint event" << std::endl;
                        wndPaint(this);
                    }
                    break;
                } 
                case ClientMessage:{
                    if(_x11viewportsS.contains(_ev.xclient.window)) break;

                    std::string xx = XGetAtomName(_display, _ev.xclient.message_type);
                    if (xx.compare("WM_PROTOCOLS") == 0) {
                        wndCloseOperations(this);
                    }
                    break;
                }
                case KeyPress:{
                    if(_x11viewportsS.contains(_ev.xkey.window)) break;

                    unsigned int kk = _ev.xkey.keycode;
                    if(LWWS_Key_X11::KeyFilter(kk)) break;

                    auto special = LWWS_Key_X11::ToSpecialKey(kk);
                    if(special != LWWS_Key::Special::RandomKey){
                        wndSpecialKeyPressed(this, special, ButtonOp::Down, {});
                        break;
                    }

                    // _text[0] can be a char that corresponds to what the user pressed
                    // but it can also be some control sequence like dec(19) in the case of ctrl+s.
                    // _key seems to always be the underlying char => use that one
                    _ev.xkey.keycode = kk;
                    if(XLookupString(&_ev.xkey, _text, 255, &_key, 0)==1) {
                        wndCharPressed(this, static_cast<char>(_key) /*_text[0]*/, ButtonOp::Down);
                        break;
                    }

                    // some unsupported key pressed
                    break;
                }
                case KeyRelease:{
                    if(_x11viewportsS.contains(_ev.xkey.window)) break;

                    unsigned int kk = _ev.xkey.keycode;
                    if(LWWS_Key_X11::KeyFilter(kk)) break;

                    auto special = LWWS_Key_X11::ToSpecialKey(kk);
                    if(special != LWWS_Key::Special::RandomKey){
                        wndSpecialKeyPressed(this, special, ButtonOp::Up, {});
                        break;
                    }

                    if(XLookupString(&_ev.xkey, _text, 255, &_key,0)){
                        XEvent ev;
                        if(XPending(_display) && XPeekEvent(_display, &ev)){
                            if(ev.type == KeyPress && ev.xkey.keycode == kk) break;
                        }

                        // _text[0] can be a char that corresponds to what the user pressed
                        // but it can also be some control sequence like dec(19) in the case of ctrl+s.
                        // _key seems to always be the underlying char => use that one
                        wndCharPressed(this, _key /*_text[0]*/, ButtonOp::Up);
                        break;
                    }

                    // some unsupported key pressed
                    break;
                }
                case ConfigureNotify:{
                    if(_x11viewportsS.contains(_ev.xconfigure.window)) break;

                    auto x = _ev.xconfigure.x;
                    auto y = _ev.xconfigure.y;
                    auto width = _ev.xconfigure.width;
                    auto height = _ev.xconfigure.height;
                    if(x != _windowState.posX || y != _windowState.posY) wndMoved(this, x, y);
                    if(width != _windowState.width || height != _windowState.height) {
                        wndResize(this, width, height, false);
                        // emit_windowEvent_Paint();
                    }
                    break;
                }
                case ButtonPress:{
                    if(_x11viewportsS.contains(_ev.xbutton.window)) break;

                    if(_ev.xbutton.button == 1) wndMousePressed(this, MouseButton::Left, ButtonOp::Down);
                    else if(_ev.xbutton.button == 2) wndMousePressed(this, MouseButton::Middle, ButtonOp::Down);
                    else if(_ev.xbutton.button == 3) wndMousePressed(this, MouseButton::Right, ButtonOp::Down);
                    else if(_ev.xbutton.button == 4) wndMouseScroll(this, 1.0); // scroll up
                    else if(_ev.xbutton.button == 5) wndMouseScroll(this, -1.0); // scroll down
                    break;
                }
                case ButtonRelease:{
                    if(_x11viewportsS.contains(_ev.xbutton.window)) break;

                    if(_ev.xbutton.button == 1) wndMousePressed(this, MouseButton::Left, ButtonOp::Up);
                    else if(_ev.xbutton.button == 2) wndMousePressed(this, MouseButton::Middle, ButtonOp::Up);
                    else if(_ev.xbutton.button == 3) wndMousePressed(this, MouseButton::Right, ButtonOp::Up);
                    /**
                     * ignore these two ON PURPOSE => no need for this
                     * else if(_ev.xbutton.button == 4) wndMouseScroll(this, 120); // scroll up
                     * else if(_ev.xbutton.button == 5) wndMouseScroll(this, -120); // scroll down
                     */
                    break;
                }
                case MotionNotify:{
                    if(_x11viewportsS.contains(_ev.xmotion.window)) break;

                    wndMouseMoved(this, _ev.xbutton.x, _ev.xbutton.y);
                    break;
                }
                case FocusIn:{
                    if(_x11viewportsS.contains(_ev.xfocus.window)) break;

                    wndSetActive(this, true);
                    break;
                }
                case FocusOut:{
                    if(_x11viewportsS.contains(_ev.xfocus.window)) break;

                    wndSetActive(this, false);
                    break;
                }
                case PropertyNotify:{
                    if(_x11viewportsS.contains(_ev.xproperty.window)) break;

                    /**
                     * Needs to be after focus in and out
                     */
                    auto event = propertyNotifyEvent();
                    int width = 0;
                    int height = 0;
                    if(!frameSize(width, height)){
                        width = _windowState.width;
                        height = _windowState.height;
                    }

                    if(event == WindowAction::Minimized) {
                        wndResize(this, width, height, true);
                        // emit_windowEvent_Paint();
                    }
                    // the other return value here is 'Maximized'
                    else if(event == WindowAction::Maximized) {
                        wndResize(this, width, height, false);
                        // emit_windowEvent_Paint();
                    }

                    break;
                }
            }
        }

        WindowAction propertyNotifyEvent(){
            /**
             * Source: https://stackoverflow.com/questions/69249370/how-do-i-capture-minimize-and-maximize-events-in-xwindows
             * Thank You Scott Franco!
             */
            std::string xx = XGetAtomName(_display, _ev.xproperty.atom);
            if(xx.compare("_NET_WM_STATE")==0){
                unsigned long after = 1L;
                int focused = 0;
                int maxhorz = 0;
                int maxvert = 0;
                int hidden = 0;
                
                do{
                    Atom type;		/* actual_type_return */
                    int format;		/* actual_format_return */
                    unsigned long length;	/* nitems_return */
                    unsigned char* dp;	/* prop_return */
                    int status = XGetWindowProperty(_display, _window, _ev.xproperty.atom,
                                                    0L, after, 0,
                                                    4/*XA_ATOM*/, 
                                                    &type, &format, &length, &after, &dp);
                                                
                    if (status == Success && type == 4/*XA_ATOM*/ && dp && format == 32 && length) {

                        for (unsigned int i = 0; i < length; i++) {

                            auto prop = ((Atom*)dp)[i];

                            if (prop == _cFocused) focused = 1;
                            if (prop == _cMaxhorz) maxhorz = 1;
                            if (prop == _cMaxvert) maxvert = 1;
                            if (prop == _cHidden) hidden = 1;

                        }

                    }
                } while(after);
                int lws = 0;
                if (hidden) {
                    lws = _winstate;
                    _winstate = 2;
                    if (lws != _winstate) return WindowAction::Minimized;
                } 
                else if (maxhorz || maxvert) {
                    lws = _winstate;
                    _winstate = 1;
                    if (lws != _winstate) return WindowAction::Maximized;
                } 
                else if (focused) {
                    lws = _winstate;
                    _winstate = 0;
                    // this one should be a window, but Windows API doesn't really support that
                    if (lws != _winstate) return WindowAction::Maximized;

                }
            }

            return WindowAction::NoAction;
        }
    };
}