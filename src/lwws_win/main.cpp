#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <iostream>
#include <thread>
#include <string>
#include <typeinfo>
#include <typeindex>

#include "include/lwws_window_x11.hpp"
LWWS::LWWS_Window_X11* window = nullptr;

void run(){
    std::unordered_map<LWWS::TViewportId, LWWS::LWWS_Viewport> viewports = {
        {0, LWWS::LWWS_Viewport(0,   0,   0, 320,100, {1, UT::RGB::Red, UT::RGB::Yellow})},
        {1, LWWS::LWWS_Viewport(1, 180, 80, 150,100, {1, UT::RGB::Green, UT::RGB::Magenta})},
        {2, LWWS::LWWS_Viewport(2, 350, 260, 150,100, {1, UT::RGB::Blue, UT::RGB::Cyan})}
    };

    auto lwws_window = LWWS::LWWS_Window_X11("hello", 640, 480, UT::RGB::Gray, viewports, true, false, 500, true);
    window = &lwws_window;

    window->windowEvents_Init();
    window->emit_windowEvent_Paint();

    while(window->windowEvents_Exist()){
        window->windowEvents_Pump();

        if(window->windowShouldClose()){
            break;
        }
    }
}

int main(int argc, char** argv) {
    // std::unordered_map<LWWS::TViewportId, LWWS::LWWS_Viewport> viewports = {
    //     {0, LWWS::LWWS_Viewport(0,   0,   0, 320,100, 1, UT::RGB::Red, UT::RGB::Yellow)},
    //     {1, LWWS::LWWS_Viewport(1, 180, 80, 150,100, 1, UT::RGB::Green, UT::RGB::Magenta)},
    //     {2, LWWS::LWWS_Viewport(2, 350, 260, 150,100, 1, UT::RGB::Blue, UT::RGB::Cyan)}
    // };

    // auto lwws_window = LWWS::LWWS_Window_X11("hello",  640,480, UT::RGB::Gray,  viewports,  true, false, 500, true);
    // auto window = &lwws_window;

    // window->windowEvents_Init();
    // window->emit_windowEvent_Paint();

    // while(window->windowEvents_Exist()){
    //     window->windowEvents_Pump();

    //     if(window->windowShouldClose()){
    //         goto terminate;
    //     }
    // }

    // terminate:
    // std::cout << "terminated" << std::endl;

    int counter = 0;
    std::thread t = std::thread(run);
    while(window == nullptr);
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        counter++;
        std::cout << counter << std::endl;
        if(counter == 5){
            std::cout << "add new window..." << std::endl;
            window->addViewport({{3, LWWS::LWWS_Viewport(3, 0, 260, 150,100, {1, UT::RGB::Navy, UT::RGB::Maroon})}});
        }
        if(counter == 10){
            std::cout << "remove window..." << std::endl;
            window->removeViewport({3});
        }
        // window->emit_windowEvent_Paint();
        // std::cout << "heartbeat" << std::endl;
    }
    t.join();
    std::cout << "joined" << std::endl;
}