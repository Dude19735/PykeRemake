#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <tchar.h>
#include <cstdlib>

#include "lwws_window.hpp"

namespace LWWS {

    static int WNDOWS_WINDOW_API_FORCE_CORRECT_INIT_WIDTH;
    static int WINDOWS_WINDOW_API_FORCE_CORRECT_INIT_HEIGHT;
    static bool MOUSE_POINTER_INITIALLY_INSIDE;
    static bool VALID;

    class LWWS_Window_Win: public LWWS_Window {
        std::string _szWindowClass = "";
        std::string _szWindowTitle = "";
        HINSTANCE _hInst = nullptr;
        HWND _hWnd = nullptr;

        MSG msg;
    public:

        LWWS_Window_Win(
            std::string szWindowTitle, 
            int width, 
            int height, 
            bool resizable,
            bool disableMousePointerOnHover=false,
            int hoverTimeoutMS=500,
            std::string szWindowClass="SomeClass",
            bool bindSamples=false
        ) 
        : 
        _szWindowClass(szWindowClass + std::to_string(std::rand()) + std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()) + std::to_string(std::rand())), 
        _szWindowTitle(szWindowTitle), 
        LWWS_Window(width, height, disableMousePointerOnHover, hoverTimeoutMS, bindSamples)
        {
            VALID = false;
            WNDOWS_WINDOW_API_FORCE_CORRECT_INIT_WIDTH = width;
            WINDOWS_WINDOW_API_FORCE_CORRECT_INIT_HEIGHT = height;
            WNDCLASSEX wcex;
            HINSTANCE hInstance = GetModuleHandle(nullptr);

            wcex.cbSize = sizeof(WNDCLASSEX);
            wcex.style          = CS_HREDRAW | CS_VREDRAW;
            wcex.lpfnWndProc    = WndProc;
            wcex.cbClsExtra     = 0;
            wcex.cbWndExtra     = 0;
            wcex.hInstance      = hInstance;
            wcex.hIcon          = LoadIcon(wcex.hInstance, IDI_APPLICATION);
            wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
            wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
            wcex.lpszMenuName   = NULL;
            wcex.lpszClassName  = _szWindowClass.c_str();
            wcex.hIconSm        = LoadIcon(wcex.hInstance, IDI_APPLICATION);
            wcex.cbWndExtra     = sizeof(this);

            bool res = RegisterClassEx(&wcex);
            if (!res)
            {
                throw std::runtime_error("Call to RegisterClassEx failed! Unable to open Windows window");
            }

            // Store instance handle in our global variable
            _hInst = hInstance;

            auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
            if(resizable) style  = style | WS_SIZEBOX;

            _hWnd = CreateWindowEx(
                /* [in, optional] lpClassName */
                WS_EX_OVERLAPPEDWINDOW,
                /* [in, optional] lpWindowName, // name of the application (not visible) */
                _szWindowClass.c_str(),
                /* text in the head bar of the windopw */
                _szWindowTitle.c_str(),
                /**
                 * All window styles, can be combined with |
                 *  => see for full list: https://learn.microsoft.com/en-us/windows/win32/winmsg/window-styles
                 */
                style,
                /* Default x and y position of the window */
                CW_USEDEFAULT, CW_USEDEFAULT,
                /* Initial Width and Height of the window */
                width, height,
                /**
                 * [in, optional] hWndParent, handle to the owner of the window (can be nullptr)
                 * For a popup window, use HWND_MESSAGE or some handle to some other window
                 */
                nullptr,
                /**
                 * [in, optional] hMenu
                 * Specifies some kind of menu to be used with this window
                 */
                nullptr,
                /**
                 * [in, optional] hInstance
                 * Handle to some module that is associated with this window
                 */
                hInstance,
                /**
                 * [in, optional] lpParam
                 * See for additional info: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowa
                 * Generally, nothing needs to be passed here
                 */
                nullptr
            );

            SetWindowLongPtr(_hWnd, 0, reinterpret_cast<LONG_PTR>(this));
        }

        ~LWWS_Window_Win(){
            DestroyWindow(_hWnd);
            UnregisterClass(_szWindowClass.c_str(), _hInst);
        }

        void getWin32WindowDescriptors(HWND& hWnd, HINSTANCE& hInst){
            hWnd = _hWnd;
            hInst = _hInst;
        }

        inline void windowEvents_Init() {
            ShowWindow(_hWnd, SW_SHOW);
            UpdateWindow(_hWnd);
            VALID = true;
        }

        inline bool windowEvents_Exist() {
            if(_windowShouldClose) return false;
            return GetMessage(&msg, NULL, 0, 0);
        }

        inline void windowEvents_Pump(){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        inline void emit_windowEvent_Paint(){
            MSG redrmsg;
            redrmsg.hwnd = _hWnd;
            redrmsg.message = WM_PAINT;
            redrmsg.wParam = 55;
            DispatchMessage(&redrmsg);
        }

        bool frameSize(int& width, int& height){
            RECT rect;
            width = 0;
            height = 0;
            auto res = GetWindowRect(_hWnd, &rect);
            if(res){
                width = rect.right - rect.left;
                height = rect.bottom - rect.top;
            }
            return res;
        }

        // must be thread safe (use _streamline_mutex)
        static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
        {
            LONG_PTR wind = GetWindowLongPtr(hWnd, 0);
            LWWS_Window_Win* window = wind != 0 ? reinterpret_cast<LWWS_Window_Win*>(wind) : nullptr;

            switch (message)
            {
            case WM_MOUSEMOVE:{
                int xPos = GET_X_LPARAM(lParam);
                int yPos = GET_Y_LPARAM(lParam);

                wndMouseLeftWindow(window, false);
                wndMouseMoved(window, xPos, yPos);

                TRACKMOUSEEVENT tmeHover;
                tmeHover.cbSize = sizeof(TRACKMOUSEEVENT);
                tmeHover.dwFlags = TME_HOVER;
                tmeHover.dwHoverTime = winHoverTimeoutMS(window);
                tmeHover.hwndTrack = hWnd;
                TrackMouseEvent(&tmeHover);

                TRACKMOUSEEVENT tmeLeave;
                tmeLeave.cbSize = sizeof(TRACKMOUSEEVENT);
                tmeLeave.dwFlags = TME_LEAVE;
                tmeLeave.dwHoverTime = 100;
                tmeLeave.hwndTrack = hWnd;
                TrackMouseEvent(&tmeLeave);

                break;
            }
            case WM_MOUSEHOVER:
                wndEnableHover(window);
                break;
            case WM_MOUSELEAVE:
                wndDisableHover(window);
                wndMouseLeftWindow(window, true);
                break;
            case WM_MOUSEACTIVATE:
                if(!VALID) break;
                // Sent when the cursor is in an inactive window and the user presses a mouse button
                SetActiveWindow(hWnd);
                break;
            case WM_ACTIVATE:
                if(!VALID) break;
                if(LOWORD(wParam)==WA_INACTIVE){
                    wndSetActive(window, false);
                }
                else if(LOWORD(wParam)==WA_CLICKACTIVE || LOWORD(wParam)==WA_ACTIVE){
                    wndSetActive(window, true);
                }
            case WM_LBUTTONDOWN:{
                wndDisableHover(window);
                wndMousePressed(window, MouseButton::Left, ButtonOp::Down);
                break;
            }
            case WM_LBUTTONUP:
                wndDisableHover(window);
                wndMousePressed(window, MouseButton::Left, ButtonOp::Up);
                break;
            case WM_MBUTTONDOWN:
                wndDisableHover(window);
                wndMousePressed(window, MouseButton::Middle, ButtonOp::Down);
                break;
            case WM_MBUTTONUP:
                wndDisableHover(window);
                wndMousePressed(window, MouseButton::Middle, ButtonOp::Up);
                break;
            case WM_RBUTTONDOWN:
                wndDisableHover(window);
                wndMousePressed(window, MouseButton::Right, ButtonOp::Down);
                break;
            case WM_RBUTTONUP:
                wndDisableHover(window);
                wndMousePressed(window, MouseButton::Right, ButtonOp::Up);
                break;
            case WM_MOUSEWHEEL:{
                /**
                 * Check for more info: https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-mousehwheel
                 * Some keys (mouse keys, ctrl button etc can be down at the same time)
                 */
                float zDelta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam) / 120.0f);
                wndMouseScroll(window, zDelta);
                break;
            }
            case WM_KEYUP:{
                LWWS_Key::Special special = LWWS_Key_Win::ToSpecialKey(wParam, lParam);
                if(special == LWWS_Key::Special::IntentionalSkip) break; // don't report this
                std::set<int> stillPressed = LWWS_Key_Win::getStillPressed(window->_currentIntPressed);
                wndSpecialKeyPressed(window, special, ButtonOp::Up, stillPressed);
                break;
            }
            case WM_KEYDOWN:{
                LWWS_Key::Special special = LWWS_Key_Win::ToSpecialKey(wParam, lParam);
                if(special == LWWS_Key::Special::IntentionalSkip) break; // don't report this
                wndSpecialKeyPressed(window, special, ButtonOp::Down, {});
                break;
            }
            case WM_CHAR:{
                char c = LWWS_Key_Win::ToChar(wParam);
                if(LWWS_Key_Win::CharFilter(c)) break;
                wndCharPressed(window, c, ButtonOp::Down);
                break;
            }
            case WM_SYSKEYDOWN:{
                LWWS_Key::Special special = LWWS_Key_Win::ToSpecialKey(wParam, lParam);
                if(special == LWWS_Key::Special::IntentionalSkip) break; // don't report this
                wndSpecialKeyPressed(window, special, ButtonOp::Down, {});
                break;
            }
            case WM_SYSKEYUP:{
                LWWS_Key::Special special = LWWS_Key_Win::ToSpecialKey(wParam, lParam);
                if(special == LWWS_Key::Special::IntentionalSkip) break; // don't report this
                std::set<int> stillPressed = LWWS_Key_Win::getStillPressed(window->_currentIntPressed);
                wndSpecialKeyPressed(window, special, ButtonOp::Up, stillPressed);
                break;
            }
            case WM_CREATE:{
                if(IsWindow(hWnd)) {
                    DWORD dwStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
                    DWORD dwExStyle = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
                    HMENU menu = GetMenu(hWnd);
                    RECT rc = {0, 0, WNDOWS_WINDOW_API_FORCE_CORRECT_INIT_WIDTH, WINDOWS_WINDOW_API_FORCE_CORRECT_INIT_HEIGHT};
                    AdjustWindowRectEx(&rc, dwStyle, menu ? TRUE : FALSE, dwExStyle);
                    SetWindowPos(hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE);

                    POINT p;
                    if (GetCursorPos(&p))
                    {
                        //cursor position now in p.x and p.y
                        if (ScreenToClient(hWnd, &p))
                        {
                            if(p.x > 0 && p.x < WNDOWS_WINDOW_API_FORCE_CORRECT_INIT_WIDTH && p.y > 0 && p.y < WINDOWS_WINDOW_API_FORCE_CORRECT_INIT_HEIGHT){
                                // we are inside the window rect
                                MOUSE_POINTER_INITIALLY_INSIDE = true;
                            }
                            else{
                                // we are initially outside
                                MOUSE_POINTER_INITIALLY_INSIDE = false;
                            }
                            //p.x and p.y are now relative to hwnd's client area
                        }
                    }
                }
                break;
            }
            case WM_SIZE:{
                if(window == nullptr) break;
                // if(window != nullptr && window->_painter != nullptr && !window->_painter->ready()) window->_painter->invalidate();
                const int width = LOWORD(lParam); // 0 if minimized, width if maximized
                const int height = HIWORD(lParam); // 0 if minimized, height if maximized
                const bool windowMinimized = wParam == SIZE_MINIMIZED; // true if minimized

                wndResize(window, width, height, windowMinimized);
                // window->createBitmap(width, height);
                break;
            }
            case WM_MOVE:{
                if(!VALID) break;
                const int xPos = static_cast<int>(LOWORD(lParam));   // horizontal position 
                const int yPos = static_cast<int>(HIWORD(lParam));   // vertical position 
                wndMoved(window, xPos, yPos);
                break;
            }
            case WM_PAINT:{
                if(window == nullptr) break;
                if(!VALID) break;
                if(wndRequiresInit(window)) wndInit(window, MOUSE_POINTER_INITIALLY_INSIDE);
                wndPaint(window);
                break;
            }
            case WM_DESTROY:
                VALID = false;
                wndDisableHover(window);
                wndCloseOperations(window);
                PostQuitMessage(0);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
                break;
            }
            return DefWindowProc(hWnd, message, wParam, lParam);
        }

    private:
        static DWORD winHoverTimeoutMS(LWWS_Window_Win* window){
            if(window == nullptr) return 0;
            return static_cast<DWORD>(window->hoverTimeoutMS());
        }
    };
}
