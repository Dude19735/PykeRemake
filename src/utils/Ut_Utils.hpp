#pragma once

#include <string>
#include <unordered_map>
#include <sstream>
#include <algorithm>

namespace UT {
    class Ut_Utils {
    public:
        template<class TKey, class TVal>
        static std::string toStr_unorderedMapKeys(const std::unordered_map<TKey, TVal>& unorderedMap){
            std::stringstream ss;
            ss << "[";
            size_t len = unorderedMap.size()-1;
            size_t c = 0;
            std::for_each(unorderedMap.begin(), unorderedMap.end(), [&](const std::pair<const TKey, TVal>& pp) { 
                ss << pp.first;
                if(c < len) ss << ",";
                c++;
            });
            ss << "]";
            return ss.str();
        }
    };
}