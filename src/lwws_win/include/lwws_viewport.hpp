#pragma once

#define NOMINMAX
#include <cmath>
#include <format>
#include "../../utils/Ut_Colors.hpp"

namespace LWWS {
    typedef int TViewportId;

    struct LWWS_MouseState {
        bool leftWindow = false;
        int transitionedAtX = -1; // last position where the mouse transitioned between the window and outside or the other way around
        int transitionedAtY = -1; //  '' same for Y ''
        int posX = -1;
        int posY = -1;
        int dx = 0;
        int dy = 0;
        int hoverTimeoutMS = 100;
        bool disableMousePointerOnHover = true;
        bool isHovering = false;
    };

    struct LWWS_WindowState {
        bool active = true; // true if window is windowActive, false if not
        int initWidth = 0;
        int initHeight = 0;
        int width = 0;
        int height = 0;
        int posX = -1;
        int posY = -1;
        bool minimized = false;
        bool initialized = false;
        bool shouldClose = false;
    };

    struct LWWS_ViewportMisc {
        int BorderWidth; 
        UT::Ut_RGBColor BorderColor;
        UT::Ut_RGBColor BgColor;

        LWWS_ViewportMisc(int borderWidth, const UT::Ut_RGBColor& borderColor, const UT::Ut_RGBColor& bgColor)
        : BorderWidth(borderWidth), BorderColor(borderColor), BgColor(bgColor){}
    };

    struct LWWS_ViewportMiscStandard : LWWS_ViewportMisc {
        LWWS_ViewportMiscStandard() : LWWS_ViewportMisc(1, UT::RGB::Black, UT::RGB::White) {}
    };

    class LWWS_Viewport {
        float _posW;
        float _posH;
        float _width;
        float _height;
        float _originalPosW;
        float _originalPosH;
        float _originalW;
        float _originalH;
        int _borderWidth;
        const UT::Ut_RGBColor& _borderColor;
        const UT::Ut_RGBColor& _bgColor;
        TViewportId _viewportId;
        LWWS_MouseState* _parentMouseState;
        LWWS_WindowState* _parentWindowState;
    public:
        LWWS_Viewport(
            TViewportId viewportId, 
            int posw, int posh, int width, int height, 
            const LWWS_ViewportMisc& misc=LWWS_ViewportMiscStandard()
        ): 
        _posW(static_cast<float>(posw)), 
        _posH(static_cast<float>(posh)), 
        _width(static_cast<float>(width-2*misc.BorderWidth)), 
        _height(static_cast<float>(height-2*misc.BorderWidth)), 
        _originalPosW(static_cast<float>(posw)), 
        _originalPosH(static_cast<float>(posh)),
        _originalW(static_cast<float>(width-2*misc.BorderWidth)), 
        _originalH(static_cast<float>(height-2*misc.BorderWidth)),
        _borderWidth(misc.BorderWidth),
        _borderColor(misc.BorderColor),
        _bgColor(misc.BgColor),
        _viewportId(viewportId),
        _parentMouseState(nullptr),
        _parentWindowState(nullptr)
        {}

        ~LWWS_Viewport(){}

        void resize(){
            if(_parentWindowState == nullptr){
                throw std::runtime_error("Set parent for viewport before resize!");
            }

            float nw = static_cast<float>(_parentWindowState->width);
            float nh = static_cast<float>(_parentWindowState->height);
            float piw = static_cast<float>(_parentWindowState->initWidth);
            float pih = static_cast<float>(_parentWindowState->initHeight);

            float posW = _originalPosW / piw * nw;
            float posH = _originalPosH / pih * nh;
            float width = (_originalW + 2*_borderWidth) / piw * nw - 2*_borderWidth;
            float height = (_originalH + 2*_borderWidth) / pih * nh - 2*_borderWidth;

            _posW = std::roundf(posW);
            _posH = std::roundf(posH);
            _width = std::roundf(width);
            _height = std::roundf(height);

            // std::cout << _width << " " << parentW << std::endl;
        }

        void setParentState(LWWS_MouseState* parentMouseState, LWWS_WindowState* parentWindowState){
            _parentMouseState = parentMouseState;
            _parentWindowState = parentWindowState;
        }

        bool contains(int posw, int posh){
            bool b1 = _posW <= posw && posw <= _posW + _width;
            bool b2 = _posH <= posh && posh <= _posH + _height;
            return b1 && b2;
        }

        int posW() const { return static_cast<int>(_posW); }
        int posH() const { return static_cast<int>(_posH); }
        int width() const { return static_cast<int>(_width); }
        int height() const { return static_cast<int>(_height); }
        LWWS_MouseState* parentMouseState() const { return _parentMouseState; }
        LWWS_WindowState* parentWindowState() const { return _parentWindowState; }
        int borderWidth() const { return _borderWidth; }
        const UT::Ut_RGBColor& borderColor() const { return _borderColor; }
        const UT::Ut_RGBColor& bgColor() const { return _bgColor; }
        const TViewportId viewportId() const { return _viewportId; }
    };
}