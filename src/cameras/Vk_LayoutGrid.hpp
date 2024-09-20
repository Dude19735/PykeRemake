#pragma once

#include <vector>
#include <set>
#include <cinttypes>

#include "../utils/Ut_Colors.hpp"
#include "../utils/Ut_Logger.hpp"
#include "I_Layout.hpp"

namespace VK5 {

    class Vk_LayoutGrid: public I_Layout {
    private:
        struct elem {
            int gridPosH, gridPosW;
            Vk_LayoutElem layoutElem;

            bool const operator==(const elem &other) const {
                return gridPosW == other.gridPosW && gridPosH == other.gridPosH;
            }

            bool const operator<(const elem &other) const {
                return gridPosW < other.gridPosW || (gridPosW == other.gridPosW && gridPosH < other.gridPosH);
            }
        };

        int _hCount;
        int _wCount;
        int _wSpacing;
        int _hSpacing;
        std::set<elem> _viewports;

    public:
        Vk_LayoutGrid(int wCount, int hCount, int wSpacing, int hSpacing) 
        : 
        _hCount(hCount), _wCount(wCount), _wSpacing(wSpacing), _hSpacing(hSpacing) 
        {
            if(hCount < 1 || wCount < 1){
                UT::Ut_Logger::RuntimeError(typeid(this), "Both hCount and wCount must be greater than 0 but hCount={0} and wCount={1}!", hCount, wCount);
            }

            if(hSpacing < 0 || wSpacing < 0){
                UT::Ut_Logger::RuntimeError(typeid(this), "Spacing must be greater or equal!");
            }
        }

        std::unordered_map<LWWS::TViewportId, Vk_Camera> layout(int width, int height) const {
            int viewportWidth = static_cast<int>(std::floor(static_cast<float>(width - 2*_wCount*_wSpacing) / static_cast<float>(_wCount)));
            int viewportHeight = static_cast<int>(std::floor(static_cast<float>(height - 2*_hCount*_hSpacing) / static_cast<float>(_hCount)));

            std::unordered_map<LWWS::TViewportId, Vk_Camera> res;
            LWWS::TViewportId viewportId = 0;
            for(const auto& v : _viewports){
                res.insert({
                    viewportId,
                    Vk_Camera(Vk_CameraInit{
                        .Misc=v.layoutElem.cameraMisc,
                        .Viewport=LWWS::LWWS_Viewport(
                            viewportId,
                            static_cast<int>((v.gridPosW*(viewportWidth + 2*_wSpacing) + _wSpacing)), 
                            static_cast<int>((v.gridPosH*(viewportHeight + 2*_hSpacing) + _hSpacing)),
                            viewportWidth, viewportHeight,
                            v.layoutElem.viewportMisc
                        ),
                        .Pinhole=v.layoutElem.cameraPinhole
                    })
                });
                viewportId++;
            }

            return res;
        }

        const int count() const { return static_cast<int>(_viewports.size()); }

        /**
         * Add camera specs at location (v,h) where v is the vertical and h the horizontal coordinate
         * starting at zero. If override=true, a camera at a specific location
         * will be overridden. Indexes start at 0. x coord runs over the width, y coord runs over
         * the height.
         * 
         * @returns the number of inserted elements. That is 1 if the operation could be completed. 
         *          If override=false and (x,y) is already taken, the return value will be 0. If override=true and (x,y) 
         *          is already taken and (x,y) fit into the grid, then the return value will be 1.
        */
        Vk_LayoutGrid& add(
            int gridPosW, int gridPosH, 
            const Vk_CameraPinhole& cameraPinhole,
            const Vk_CameraMisc& cameraMisc,
            const LWWS::LWWS_ViewportMisc& viewportMisc=LWWS::LWWS_ViewportMiscStandard()
        ){
            if(gridPosH < 0 || gridPosH >= _hCount) {
                UT::Ut_Logger::RuntimeError(typeid(this), "ViewportId with position gridPosW={0}, gridPosH={1} doesn't exist. Possible are gridPosW=0 to gridPosW={2} and gridPosH=0 to gridPosH={3}", gridPosW, gridPosH, _wCount, _hCount);
            }
            if(gridPosW < 0 || gridPosW >= _wCount) {
                UT::Ut_Logger::RuntimeError(typeid(this), "ViewportId with position gridPosW={0}, gridPosH={1} doesn't exist. Possible are gridPosW=0 to gridPosW={2} and gridPosH=0 to gridPosH={3}", gridPosW, gridPosH, _wCount, _hCount);
            }

            elem e {
                .gridPosH=gridPosH,
                .gridPosW=gridPosW,
                .layoutElem=Vk_LayoutElem{
                    .viewportMisc=viewportMisc,
                    .cameraMisc=cameraMisc,
                    .cameraPinhole=cameraPinhole
                }
            };

            auto loc = _viewports.find(e);
            if(loc != _viewports.end()){
                UT::Ut_Logger::RuntimeError(typeid(this), "ViewportId with position gridPosW={0}, gridPosH={1} already exists.", e.gridPosW, e.gridPosH);
            }

            _viewports.insert(e);
            return *this;
        }
    };
}