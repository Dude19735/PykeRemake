#pragma once

#include <vector>
#include <functional>
#include <sstream>
#include <type_traits>

namespace UT {
    class Ut_Std {
        template <class T, class... Ts>
		struct is_any : std::disjunction<std::is_same<T, Ts>...> {};

    public:
		static bool str_replace(std::string& str, const std::string& from, const std::string& to) {
			size_t start_pos = str.find(from);
			if (start_pos == std::string::npos)
				return false;
			str.replace(start_pos, from.length(), to);
			return true;
		}

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

		template<class T>
		static std::string set_to_str(const std::set<T>& set, std::string(*toStr)(const T&)){
			std::stringstream ss;
			std::for_each(set.begin(), set.end(), [&](const T& elem) { ss << toStr(elem); });
			return ss.str();
		}
    };
}