#pragma once

#include "../application/Vk_Device.h"

namespace VK5{
    class I_Renderer {
    private:
        Vk_Device* _device;
    public:
        I_Renderer(Vk_Device* device)
        :
        _device(device)
        {}
    };
}