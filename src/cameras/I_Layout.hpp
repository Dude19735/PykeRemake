#pragma once

#include <unordered_map>
#include "../Defines.h"
#include "Vk_Camera.hpp"

namespace VK5 {
    struct Vk_LayoutElem {
        LWWS::LWWS_ViewportMisc viewportMisc;
        Vk_CameraMisc cameraMisc;
        Vk_CameraPinhole cameraPinhole;
    };

    class I_Layout {
    public:
        virtual std::unordered_map<LWWS::TViewportId, Vk_Camera> layout(int width, int height) const = 0;
        virtual const int count() const = 0;
    };
}