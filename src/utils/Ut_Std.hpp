#pragma once

#include <vector>
#include <type_traits>

namespace UT {
    class Ut_Std {
        template <class T, class... Ts>
		struct is_any : std::disjunction<std::is_same<T, Ts>...> {};

    public:
		template<class T>
		static std::vector<T> vec_range(T from, T to){
			static_assert(is_any<T, int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t>::value == true);
			assert(to > from);
			T range = to - from;
			std::vector<T> res;
			res.reserve(range);
			for(T i=from; i<to; ++i) res.push_back(i);
			return res;
		}
    };
}