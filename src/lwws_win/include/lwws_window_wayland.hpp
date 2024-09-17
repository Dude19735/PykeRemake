#pragma once
#include <stdexcept>
#include "lwws_window.hpp"

namespace LWWS {
    class LWWS_Window_Wayland: public LWWS_Window{
        LWWS_Window_Wayland() : LWWS_Window(0, 0, true, 0, true) {
            throw std::runtime_error("Not supported yet because NVidia driver support for Wayland apparently sucks (-.-)");
        }
    };
}