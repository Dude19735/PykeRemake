#pragma once

#include <tuple>
#include <functional>

namespace VK5 {
    template<typename ObjType>
	using t_func = void (ObjType::*)(std::function<void()>);
    
    class Vk_Action {
    private:
        int _size;
    public:
        Vk_Action(int size) : _size(size) {}
        ~Vk_Action(){}

        virtual void operator()(std::function<void()> repeat){}

        std::shared_ptr<Vk_Action> get() {
            std::shared_ptr<void> x( new char[_size]);
            memcpy(x.get(), this, _size);

            return std::static_pointer_cast<Vk_Action>(x);
        }
    };

    template<class ObjType, class ... _Types>
    class Vk_TFunc : public Vk_Action {
    private:
        // function pointer type
        typedef void(ObjType::* mf_type)(std::function<void()>, _Types...);

        // recursion to unpack the arguments
        template <std::size_t... _Indices>
		struct _indices {};

		template <std::size_t _N, std::size_t... _Is>
		struct _build_indices : _build_indices<_N - 1, _N - 1, _Is...> {};

		template <std::size_t... _Is>
		struct _build_indices<0, _Is...> : _indices<_Is...> {};

        mf_type _func;
		ObjType* _obj;
		std::tuple<_Types...> _args;

    public:
        Vk_TFunc(ObjType* obj, mf_type func, std::tuple<_Types...>&& args)
        :
        _obj(obj),
        _func(func),
        _args(args),
        Vk_Action(sizeof(Vk_TFunc) + sizeof(Vk_Action))
        {}

        void operator()(std::function<void()> repeat) override {
            const _build_indices<std::tuple_size<std::tuple<_Types...>>::value> ind;
			_call_F(repeat, std::move(_args), ind);
        }

    private:
        template <typename _Args, std::size_t... _Inds>
		auto _call_F(std::function<void()> repeat, _Args&& args, const _indices<_Inds...>&) -> void
		{
			(_obj->*_func)(repeat, std::get<_Inds>(args)...);
		}
    };
}