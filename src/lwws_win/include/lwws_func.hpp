#pragma once

#include <functional>
#include <memory>
#include <set>
#include "lwws_key.hpp"

namespace LWWS {

    class LWWS_Func {
    public:
        LWWS_Func() {}
        virtual void operator()(void* aptr){}
        virtual void operator()(int key, ButtonOp op, const std::set<int>& otherPressedKeys, void* aptr){}
        virtual void operator()(int x, int y, int px, int py, const std::set<int>& pressedKeys, WindowAction state, void* aptr){}
        virtual void operator()(int px, int py, int dx, int dy, float dz, const std::set<int>& pressedKeys, MouseButton button, ButtonOp bop, MouseAction action, void* aptr){}
    };

    // on_Destructor_Callback, on_WindowClose_Callback
    // ..(void) ===============================
    template<class ObjType>
    using t_void_func = void(ObjType::*)(void* aptr);
    using t_void_func_null = void(*)(void* aptr);
    template<class ObjType>
    class LWWS_VoidFunc: public LWWS_Func {
    public:
        LWWS_VoidFunc(ObjType* obj, t_void_func<ObjType> f): _obj(obj), _func(f) {}

        void operator()(void* aptr) override{
            (_obj->*_func)(aptr);
        }

    private:
        ObjType* _obj;
        t_void_func<ObjType> _func;
    };

    class LWWS_VoidFuncNull: public LWWS_Func {
    public:
        LWWS_VoidFuncNull(t_void_func_null f): _func(f) {}

        void operator()(void* aptr) override{
            (*_func)(aptr);
        }

    private:
        t_void_func_null _func;
    };

    // on_IntKey_Callback
    // ..(IntKey, Up/Down) ================================
    template<class ObjType>
    using t_int_func = void(ObjType::*)(int key, ButtonOp op, const std::set<int>& otherPressedKeys, void* aptr);
    using t_int_func_null = void(*)(int key, ButtonOp op, const std::set<int>& otherPressedKeys, void* aptr);
    template<class ObjType>
    class LWWS_IntFunc: public LWWS_Func {
    public:
        LWWS_IntFunc(ObjType* obj, t_int_func<ObjType> f): _obj(obj), _func(f) {}

        void operator()(int key, ButtonOp op, const std::set<int>& otherPressedKeys, void* aptr) override{
            (_obj->*_func)(key, op, otherPressedKeys, aptr);
        }

    private:
        ObjType* _obj;
        t_int_func<ObjType> _func;
    };

    class LWWS_IntFuncNull: public LWWS_Func {
    public:
        LWWS_IntFuncNull(t_int_func_null f): _func(f) {}

        void operator()(int key, ButtonOp op, const std::set<int>& otherPressedKeys, void* aptr) override{
            (*_func)(key, op, otherPressedKeys, aptr);
        }

    private:
        t_int_func_null _func;
    };

    // on_WindowState_Callback
    // ..(int, int, Activate/Deactivate/Resize/Max/Min) ===============================
    template<class ObjType>
    using t_windowstate_func = void(ObjType::*)(int w, int h, int px, int py, const std::set<int>& pressedKeys, WindowAction state, void* aptr);
    using t_windowstate_func_null = void(*)(int w, int h, int px, int py, const std::set<int>& pressedKeys, WindowAction state, void* aptr);
    template<class ObjType>
    class LWWS_WindowStateFunc: public LWWS_Func {
    public:
        LWWS_WindowStateFunc(ObjType* obj, t_windowstate_func<ObjType> f): _obj(obj), _func(f) {}

        void operator()(int w, int h, int px, int py, const std::set<int>& pressedKeys, WindowAction state, void* aptr) override{
            (_obj->*_func)(w, h, px, py, pressedKeys, state, aptr);
        }

    private:
        ObjType* _obj;
        t_windowstate_func<ObjType> _func;
    };

    class LWWS_WindowStateFuncNull: public LWWS_Func {
    public:
        LWWS_WindowStateFuncNull(t_windowstate_func_null f): _func(f) {}

        void operator()(int w, int h, int px, int py, const std::set<int>& pressedKeys, WindowAction state, void* aptr) override{
            (*_func)(w, h, px, py, pressedKeys, state, aptr);
        }

    private:
        t_windowstate_func_null _func;
    };

    // on_MouseAction_Callback
    // ..(int, int, int, MouseButton, ButtonOp, MouseAction) ===========================
    template<class ObjType>
    using t_mouseaction_func = void(ObjType::*)(int px, int py, int dx, int dy, float dz, const std::set<int>& pressedKeys, MouseButton button, ButtonOp bop, MouseAction action, void* aptr);
    using t_mouseaction_func_null = void(*)(int px, int py, int dx, int dy, float dz, const std::set<int>& pressedKeys, MouseButton button, ButtonOp bop, MouseAction action, void* aptr);
    template<class ObjType>
    class LWWS_MouseActionFunc: public LWWS_Func {
    public:
        LWWS_MouseActionFunc(ObjType* obj, t_mouseaction_func<ObjType> f): _obj(obj), _func(f) {}

        void operator()(int px, int py, int dx, int dy, float dz, const std::set<int>& pressedKeys, MouseButton button, ButtonOp bop, MouseAction action, void* aptr) override{
            (_obj->*_func)(px, py, dx, dy, dz, pressedKeys, button, bop, action, aptr);
        }

    private:
        ObjType* _obj;
        t_mouseaction_func<ObjType> _func;
    };

    class LWWS_MouseActionFuncNull: public LWWS_Func {
    public:
        LWWS_MouseActionFuncNull(t_mouseaction_func_null f): _func(f) {}

        void operator()(int px, int py, int dx, int dy, float dz, const std::set<int>& pressedKeys, MouseButton button, ButtonOp bop, MouseAction action, void* aptr) override{
            (*_func)(px, py, dx, dy, dz, pressedKeys, button, bop, action, aptr);
        }

    private:
        t_mouseaction_func_null _func;
    };
}
